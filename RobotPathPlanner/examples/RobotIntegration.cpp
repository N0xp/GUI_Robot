// RobotIntegration.cpp
// Example implementation for robot-side GUI integration

#include "RobotIntegration.h"
#include <iostream>

PathPlannerInterface::PathPlannerInterface(int port)
    : m_port(port)
    , m_robotX(0.0)
    , m_robotY(0.0)
    , m_robotHeading(0.0)
{
    // TODO: Initialize TCP server on m_port
    // Start listening for connections from the GUI
}

void PathPlannerInterface::periodic() {
    // TODO: Check for incoming messages from GUI
    // This should be called in your robot's periodic loop

    // Example pseudo-code:
    // if (socket.hasData()) {
    //     std::string message = socket.readLine();
    //     handleIncomingMessage(message);
    // }

    // Send robot pose update (at ~20Hz)
    wpi::json poseMsg;
    poseMsg["type"] = "robotPose";
    poseMsg["x"] = m_robotX;
    poseMsg["y"] = m_robotY;
    poseMsg["heading"] = m_robotHeading;

    sendMessage(poseMsg);
}

void PathPlannerInterface::setRobotPose(double x, double y, double heading) {
    m_robotX = x;
    m_robotY = y;
    m_robotHeading = heading;
}

void PathPlannerInterface::onPathReceived(std::function<void(const RobotPath&)> callback) {
    m_pathCallback = callback;
}

void PathPlannerInterface::sendStatus(const std::string& status, bool moving) {
    wpi::json statusMsg;
    statusMsg["type"] = "status";
    statusMsg["status"] = status;
    statusMsg["moving"] = moving;

    sendMessage(statusMsg);
}

void PathPlannerInterface::notifyPathExecutionStarted() {
    wpi::json msg;
    msg["type"] = "pathExecutionStarted";
    sendMessage(msg);
}

void PathPlannerInterface::notifyPathExecutionFinished(bool success) {
    wpi::json msg;
    msg["type"] = "pathExecutionFinished";
    msg["success"] = success;
    sendMessage(msg);
}

void PathPlannerInterface::handleIncomingMessage(const std::string& message) {
    try {
        wpi::json json = wpi::json::parse(message);

        std::string type = json["type"].get<std::string>();

        if (type == "sendPath") {
            // Parse path and invoke callback
            RobotPath path;
            path.name = json["path"]["name"].get<std::string>();

            for (const auto& wpJson : json["path"]["waypoints"]) {
                Waypoint wp;
                wp.x = wpJson["x"].get<double>();
                wp.y = wpJson["y"].get<double>();
                wp.heading = wpJson["heading"].get<double>();
                wp.velocity = wpJson["velocity"].get<double>();
                path.waypoints.push_back(wp);
            }

            if (m_pathCallback) {
                m_pathCallback(path);
            }
        }
        else if (type == "getState") {
            // GUI is requesting current state - response sent in periodic()
        }
        else if (type == "setRobotShape") {
            // Optional: Handle robot shape changes
            std::string shape = json["shape"].get<std::string>();
            // Update robot configuration if needed
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
    }
}

void PathPlannerInterface::sendMessage(const wpi::json& json) {
    // TODO: Send JSON message over TCP socket
    // std::string message = json.dump() + "\n";
    // socket.send(message);
}
