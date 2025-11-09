/************************************
 * PathPlanner Communication Module Implementation
 * Handles TCP communication with RobotPathPlanner GUI
 *************************************/

#include "PathPlannerComm.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cmath>
#include <chrono>
#include <thread>

// Platform-specific socket includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

namespace PathPlanner {

    // Simple JSON parser (minimal implementation for our needs)
    class SimpleJson {
    public:
        static std::string GetString(const std::string& json, const std::string& key) {
            std::string searchKey = "\"" + key + "\":";
            size_t pos = json.find(searchKey);
            if (pos == std::string::npos) return "";

            pos += searchKey.length();
            while (pos < json.length() && (json[pos] == ' ' || json[pos] == '"')) pos++;

            size_t endPos = json.find_first_of("\",}", pos);
            if (endPos == std::string::npos) return "";

            return json.substr(pos, endPos - pos);
        }

        static double GetNumber(const std::string& json, const std::string& key) {
            std::string value = GetString(json, key);
            if (value.empty()) return 0.0;
            return std::stod(value);
        }

        static bool GetBool(const std::string& json, const std::string& key) {
            std::string value = GetString(json, key);
            return value == "true";
        }
    };

    PathPlannerComm::PathPlannerComm(int port)
        : m_port(port),
          m_serverSocket(INVALID_SOCKET),
          m_clientSocket(INVALID_SOCKET),
          m_running(false),
          m_connected(false),
          m_currentPose(0, 0, 0),
          m_hasNewPath(false) {

#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

        std::cout << "[PathPlanner] PathPlannerComm initialized on port " << m_port << std::endl;
    }

    PathPlannerComm::~PathPlannerComm() {
        Stop();

#ifdef _WIN32
        WSACleanup();
#endif
    }

    void PathPlannerComm::Start() {
        if (m_running) {
            std::cout << "[PathPlanner] Already running!" << std::endl;
            return;
        }

        m_running = true;
        m_serverThread = std::thread(&PathPlannerComm::ServerThread, this);
        m_sendThread = std::thread(&PathPlannerComm::SendThread, this);

        std::cout << "[PathPlanner] Communication started" << std::endl;
    }

    void PathPlannerComm::Stop() {
        if (!m_running) return;

        m_running = false;
        m_connected = false;

        if (m_clientSocket != INVALID_SOCKET) {
#ifdef _WIN32
            closesocket(m_clientSocket);
#else
            close(m_clientSocket);
#endif
            m_clientSocket = INVALID_SOCKET;
        }

        if (m_serverSocket != INVALID_SOCKET) {
#ifdef _WIN32
            closesocket(m_serverSocket);
#else
            close(m_serverSocket);
#endif
            m_serverSocket = INVALID_SOCKET;
        }

        if (m_serverThread.joinable()) m_serverThread.join();
        if (m_sendThread.joinable()) m_sendThread.join();

        std::cout << "[PathPlanner] Communication stopped" << std::endl;
    }

    bool PathPlannerComm::IsConnected() const {
        return m_connected;
    }

    void PathPlannerComm::UpdateRobotPose(double x, double y, double heading) {
        std::lock_guard<std::mutex> lock(m_poseMutex);
        m_currentPose.x = x;
        m_currentPose.y = y;
        m_currentPose.heading = heading;
    }

    void PathPlannerComm::UpdateRobotPose(const RobotPose& pose) {
        UpdateRobotPose(pose.x, pose.y, pose.heading);
    }

    void PathPlannerComm::SetPathReceivedCallback(std::function<void(const Path&)> callback) {
        m_pathCallback = callback;
    }

    bool PathPlannerComm::GetLatestPath(Path& path) {
        std::lock_guard<std::mutex> lock(m_pathMutex);
        if (!m_hasNewPath) return false;

        path = m_latestPath;
        m_hasNewPath = false;
        return true;
    }

    std::vector<Path> PathPlannerComm::GetAllPaths() {
        std::lock_guard<std::mutex> lock(m_pathMutex);
        return m_allPaths;
    }

    bool PathPlannerComm::GetPathByName(const std::string& name, Path& path) {
        std::lock_guard<std::mutex> lock(m_pathMutex);

        for (const auto& p : m_allPaths) {
            if (p.name == name) {
                path = p;
                return true;
            }
        }
        return false;
    }

    bool PathPlannerComm::GetPathByIndex(int index, Path& path) {
        std::lock_guard<std::mutex> lock(m_pathMutex);

        if (index < 0 || index >= static_cast<int>(m_allPaths.size())) {
            return false;
        }

        path = m_allPaths[index];
        return true;
    }

    int PathPlannerComm::GetPathCount() const {
        std::lock_guard<std::mutex> lock(m_pathMutex);
        return static_cast<int>(m_allPaths.size());
    }

    void PathPlannerComm::ClearPaths() {
        std::lock_guard<std::mutex> lock(m_pathMutex);
        m_allPaths.clear();
        m_hasNewPath = false;
    }

    void PathPlannerComm::SendStatus(const std::string& status, bool isMoving) {
        if (!m_connected) return;
        std::string json = CreateStatusJson(status, isMoving);
        SendMessage(json);

        std::cout << "[PathPlanner] Sent status: " << status << " (moving: " << isMoving << ")" << std::endl;
    }

    void PathPlannerComm::NotifyPathExecutionStarted() {
        if (!m_connected) return;
        SendMessage("{\"type\":\"pathExecutionStarted\"}");

        std::cout << "[PathPlanner] Sent path execution started notification" << std::endl;
    }

    void PathPlannerComm::NotifyPathExecutionFinished(bool success) {
        if (!m_connected) return;
        std::string json = "{\"type\":\"pathExecutionFinished\",\"success\":";
        json += success ? "true}" : "false}";
        SendMessage(json);

        std::cout << "[PathPlanner] Sent path execution finished: " << (success ? "success" : "failed") << std::endl;
    }

    void PathPlannerComm::ServerThread() {
        // Create server socket
        m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_serverSocket == INVALID_SOCKET) {
            std::cerr << "[PathPlanner] Failed to create server socket!" << std::endl;
            return;
        }

        // Set socket options
        int opt = 1;
        setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

        // Bind socket
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(m_port);

        if (bind(m_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "[PathPlanner] Failed to bind socket to port " << m_port << std::endl;
            return;
        }

        // Listen for connections
        if (listen(m_serverSocket, 1) == SOCKET_ERROR) {
            std::cerr << "[PathPlanner] Failed to listen on socket!" << std::endl;
            return;
        }

        std::cout << "[PathPlanner] Server listening on port " << m_port << std::endl;

        while (m_running) {
            // Accept connection
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);

            std::cout << "[PathPlanner] Waiting for GUI connection..." << std::endl;
            m_clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

            if (m_clientSocket == INVALID_SOCKET) {
                if (m_running) {
                    std::cerr << "[PathPlanner] Accept failed!" << std::endl;
                }
                continue;
            }

            m_connected = true;
            std::cout << "[PathPlanner] GUI connected!" << std::endl;

            // Receive messages
            char buffer[4096];
            std::string receivedData;

            while (m_running && m_connected) {
                memset(buffer, 0, sizeof(buffer));
                int bytesReceived = recv(m_clientSocket, buffer, sizeof(buffer) - 1, 0);

                if (bytesReceived <= 0) {
                    m_connected = false;
                    std::cout << "[PathPlanner] GUI disconnected" << std::endl;
                    break;
                }

                receivedData += std::string(buffer, bytesReceived);

                // Process complete messages (newline-delimited)
                size_t pos;
                while ((pos = receivedData.find('\n')) != std::string::npos) {
                    std::string message = receivedData.substr(0, pos);
                    receivedData = receivedData.substr(pos + 1);

                    if (!message.empty()) {
                        std::cout << "[PathPlanner] Received message: " << message << std::endl;
                        HandleMessage(message);
                    }
                }
            }

#ifdef _WIN32
            closesocket(m_clientSocket);
#else
            close(m_clientSocket);
#endif
            m_clientSocket = INVALID_SOCKET;
        }
    }

    void PathPlannerComm::SendThread() {
        while (m_running) {
            if (m_connected) {
                // Send robot pose update (20Hz)
                RobotPose pose;
                {
                    std::lock_guard<std::mutex> lock(m_poseMutex);
                    pose = m_currentPose;
                }

                std::string json = CreatePoseJson(pose);
                SendMessage(json);
            }

            // Sleep for 50ms (20Hz)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    void PathPlannerComm::HandleMessage(const std::string& message) {
        std::string type = SimpleJson::GetString(message, "type");

        // Only print detailed info for sendPath messages
        if (type == "sendPath") {
            std::cout << "[PathPlanner] ===== PATH MESSAGE RECEIVED =====" << std::endl;
            std::cout << "[PathPlanner] Full message: " << message << std::endl;
        }

        if (type == "sendPath") {
            // Parse path from JSON
            Path path = ParsePathFromJson(message);

            std::cout << "[PathPlanner] ===== PATH DATA =====" << std::endl;
            std::cout << "[PathPlanner] Path name: " << path.name << std::endl;
            std::cout << "[PathPlanner] Number of waypoints: " << path.waypoints.size() << std::endl;

            for (size_t i = 0; i < path.waypoints.size(); i++) {
                const auto& wp = path.waypoints[i];
                std::cout << "[PathPlanner]   Waypoint " << i << ": "
                         << "x=" << wp.x << "m, "
                         << "y=" << wp.y << "m, "
                         << "heading=" << wp.heading << "rad, "
                         << "velocity=" << wp.velocity << "m/s" << std::endl;
            }
            std::cout << "[PathPlanner] ================================" << std::endl;

            {
                std::lock_guard<std::mutex> lock(m_pathMutex);
                m_latestPath = path;
                m_hasNewPath = true;

                // Store in all paths - replace if name already exists
                bool found = false;
                for (auto& p : m_allPaths) {
                    if (p.name == path.name) {
                        p = path;  // Replace existing path
                        found = true;
                        std::cout << "[PathPlanner] Updated existing path: " << path.name << std::endl;
                        break;
                    }
                }

                if (!found) {
                    m_allPaths.push_back(path);
                    std::cout << "[PathPlanner] Added new path: " << path.name
                             << " (Total paths: " << m_allPaths.size() << ")" << std::endl;
                }
            }

            // Call callback if set
            if (m_pathCallback) {
                m_pathCallback(path);
            }
        }
        else if (type == "getState") {
            // Don't spam console with state requests
            SendStatus("idle", false);

            // Also send current robot pose
            {
                std::lock_guard<std::mutex> lock(m_poseMutex);
                std::string poseJson = CreatePoseJson(m_currentPose);
                SendMessage(poseJson);
            }
        }
    }

    void PathPlannerComm::SendMessage(const std::string& message) {
        if (!m_connected || m_clientSocket == INVALID_SOCKET) return;

        std::string toSend = message + "\n";
        int result = send(m_clientSocket, toSend.c_str(), toSend.length(), 0);

        // If send fails, mark as disconnected
        if (result <= 0) {
            std::cerr << "[PathPlanner] Send failed, marking disconnected" << std::endl;
            m_connected = false;
        }
    }

    Path PathPlannerComm::ParsePathFromJson(const std::string& json) {
        Path path;

        // Extract path name
        path.name = SimpleJson::GetString(json, "name");

        // Find waypoints array
        size_t waypointsPos = json.find("\"waypoints\":");
        if (waypointsPos == std::string::npos) return path;

        size_t arrayStart = json.find('[', waypointsPos);
        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayStart == std::string::npos || arrayEnd == std::string::npos) return path;

        std::string waypointsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

        // Parse individual waypoints
        size_t pos = 0;
        while (pos < waypointsArray.length()) {
            size_t objStart = waypointsArray.find('{', pos);
            if (objStart == std::string::npos) break;

            size_t objEnd = waypointsArray.find('}', objStart);
            if (objEnd == std::string::npos) break;

            std::string waypointJson = waypointsArray.substr(objStart, objEnd - objStart + 1);

            Waypoint wp;
            wp.x = SimpleJson::GetNumber(waypointJson, "x");
            wp.y = SimpleJson::GetNumber(waypointJson, "y");
            wp.heading = SimpleJson::GetNumber(waypointJson, "heading");
            wp.velocity = SimpleJson::GetNumber(waypointJson, "velocity");

            path.waypoints.push_back(wp);
            pos = objEnd + 1;
        }

        return path;
    }

    std::string PathPlannerComm::CreatePoseJson(const RobotPose& pose) {
        std::ostringstream oss;
        oss << "{\"type\":\"robotPose\","
            << "\"x\":" << pose.x << ","
            << "\"y\":" << pose.y << ","
            << "\"heading\":" << pose.heading << "}";
        return oss.str();
    }

    std::string PathPlannerComm::CreateStatusJson(const std::string& status, bool isMoving) {
        std::ostringstream oss;
        oss << "{\"type\":\"status\","
            << "\"status\":\"" << status << "\","
            << "\"moving\":" << (isMoving ? "true" : "false") << "}";
        return oss.str();
    }

} // namespace PathPlanner
