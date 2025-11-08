/************************************
 * PathPlanner Communication Module
 * Handles TCP communication with RobotPathPlanner GUI
 * Author: Integration Module
 * Version: 1.0.0
 *************************************/

#ifndef PATHPLANNER_COMM_H
#define PATHPLANNER_COMM_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

namespace PathPlanner {

    // Waypoint structure matching the GUI format
    struct Waypoint {
        double x;           // meters
        double y;           // meters
        double heading;     // radians
        double velocity;    // m/s

        Waypoint(double x_ = 0, double y_ = 0, double heading_ = 0, double velocity_ = 0)
            : x(x_), y(y_), heading(heading_), velocity(velocity_) {}
    };

    // Path structure
    struct Path {
        std::string name;
        std::vector<Waypoint> waypoints;

        Path(const std::string& name_ = "") : name(name_) {}
    };

    // Robot pose structure
    struct RobotPose {
        double x;           // meters
        double y;           // meters
        double heading;     // radians

        RobotPose(double x_ = 0, double y_ = 0, double heading_ = 0)
            : x(x_), y(y_), heading(heading_) {}
    };

    // Communication class
    class PathPlannerComm {
    public:
        PathPlannerComm(int port = 5800);
        ~PathPlannerComm();

        // Start/Stop the TCP server
        void Start();
        void Stop();

        // Check if connected to GUI
        bool IsConnected() const;

        // Update robot pose (call this periodically, e.g., in RobotPeriodic)
        void UpdateRobotPose(double x, double y, double heading);
        void UpdateRobotPose(const RobotPose& pose);

        // Set callback for when a new path is received
        void SetPathReceivedCallback(std::function<void(const Path&)> callback);

        // Get the latest received path
        bool GetLatestPath(Path& path);

        // Get all stored paths
        std::vector<Path> GetAllPaths();

        // Get a specific path by name
        bool GetPathByName(const std::string& name, Path& path);

        // Get a specific path by index
        bool GetPathByIndex(int index, Path& path);

        // Get number of stored paths
        int GetPathCount() const;

        // Clear all stored paths
        void ClearPaths();

        // Send status message to GUI
        void SendStatus(const std::string& status, bool isMoving = false);

        // Notify GUI of path execution events
        void NotifyPathExecutionStarted();
        void NotifyPathExecutionFinished(bool success);

    private:
        int m_port;
        int m_serverSocket;
        int m_clientSocket;
        std::atomic<bool> m_running;
        std::atomic<bool> m_connected;

        std::thread m_serverThread;
        std::thread m_sendThread;

        RobotPose m_currentPose;
        mutable std::mutex m_poseMutex;

        Path m_latestPath;
        bool m_hasNewPath;
        std::vector<Path> m_allPaths;  // Store all received paths
        mutable std::mutex m_pathMutex;

        std::function<void(const Path&)> m_pathCallback;

        // Thread functions
        void ServerThread();
        void SendThread();

        // Message handling
        void HandleMessage(const std::string& message);
        void SendMessage(const std::string& message);

        // JSON parsing helpers
        Path ParsePathFromJson(const std::string& json);
        std::string CreatePoseJson(const RobotPose& pose);
        std::string CreateStatusJson(const std::string& status, bool isMoving);
    };

} // namespace PathPlanner

#endif // PATHPLANNER_COMM_H
