// RobotIntegration.h
// Example code showing how to integrate the Path Planner GUI with your robot

#ifndef ROBOT_INTEGRATION_H
#define ROBOT_INTEGRATION_H

#include <string>
#include <vector>
#include <functional>
#include <wpi/json.h>

// Simple TCP server for communication
// Note: You'll need to implement your own TCP server or use WPILib's NetworkTables

struct Waypoint {
    double x;
    double y;
    double heading;
    double velocity;
};

struct RobotPath {
    std::string name;
    std::vector<Waypoint> waypoints;
};

class PathPlannerInterface {
public:
    PathPlannerInterface(int port = 5800);

    // Call this in your robot's periodic function
    void periodic();

    // Set current robot position (call frequently, ~20Hz)
    void setRobotPose(double x, double y, double heading);

    // Register callback for when a new path is received
    void onPathReceived(std::function<void(const RobotPath&)> callback);

    // Send status to GUI
    void sendStatus(const std::string& status, bool moving);

    // Notify path execution started/finished
    void notifyPathExecutionStarted();
    void notifyPathExecutionFinished(bool success);

private:
    void handleIncomingMessage(const std::string& message);
    void sendMessage(const wpi::json& json);

    int m_port;
    double m_robotX;
    double m_robotY;
    double m_robotHeading;

    std::function<void(const RobotPath&)> m_pathCallback;

    // Add your TCP socket implementation here
};

#endif // ROBOT_INTEGRATION_H
