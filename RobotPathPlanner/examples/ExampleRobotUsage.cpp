// ExampleRobotUsage.cpp
// Complete example showing how to integrate Path Planner GUI with your FRC robot

#include "RobotIntegration.h"
#include <frc/TimedRobot.h>
#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/drive/DifferentialDrive.h>
#include <frc/motorcontrol/PWMSparkMax.h>
#include <frc/controller/RamseteController.h>
#include <frc/trajectory/TrajectoryGenerator.h>
#include <frc/kinematics/DifferentialDriveOdometry.h>
#include <units/length.h>
#include <units/velocity.h>

class Robot : public frc::TimedRobot {
public:
    Robot()
        : m_pathPlanner(5800)
    {
        // Register callback for when paths are received from GUI
        m_pathPlanner.onPathReceived([this](const RobotPath& path) {
            onNewPathReceived(path);
        });
    }

    void RobotInit() override {
        // Initialize your robot hardware
    }

    void RobotPeriodic() override {
        // Update odometry from your sensors
        updateOdometry();

        // Send current position to GUI (called at ~50Hz by default)
        m_pathPlanner.setRobotPose(
            m_currentX,
            m_currentY,
            m_currentHeading
        );

        // Handle GUI communication
        m_pathPlanner.periodic();
    }

    void AutonomousInit() override {
        // Autonomous initialization
    }

    void AutonomousPeriodic() override {
        // If we have a path from the GUI, follow it
        if (m_hasPath) {
            followCurrentPath();
        }
    }

    void TeleopInit() override {
        // Teleop initialization
    }

    void TeleopPeriodic() override {
        // Regular teleop control
        // Position is still sent to GUI in RobotPeriodic()
    }

private:
    // Path Planner GUI interface
    PathPlannerInterface m_pathPlanner;

    // Robot state
    double m_currentX = 0.0;
    double m_currentY = 0.0;
    double m_currentHeading = 0.0;

    // Current path from GUI
    RobotPath m_currentPath;
    bool m_hasPath = false;
    size_t m_currentWaypointIndex = 0;

    void updateOdometry() {
        // TODO: Update m_currentX, m_currentY, m_currentHeading from your encoders/gyro
        // Example:
        // auto pose = m_odometry.Update(m_gyro.GetRotation2d(),
        //                               m_leftEncoder.GetDistance(),
        //                               m_rightEncoder.GetDistance());
        // m_currentX = pose.X().value();
        // m_currentY = pose.Y().value();
        // m_currentHeading = pose.Rotation().Radians().value();
    }

    void onNewPathReceived(const RobotPath& path) {
        std::cout << "Received new path: " << path.name << std::endl;
        std::cout << "Waypoints: " << path.waypoints.size() << std::endl;

        m_currentPath = path;
        m_hasPath = true;
        m_currentWaypointIndex = 0;

        // Notify GUI that we're starting path execution
        m_pathPlanner.notifyPathExecutionStarted();
        m_pathPlanner.sendStatus("Executing path", true);

        // You can convert waypoints to FRC trajectory here
        // convertToTrajectory(path);
    }

    void followCurrentPath() {
        if (m_currentWaypointIndex >= m_currentPath.waypoints.size()) {
            // Path complete
            m_hasPath = false;
            m_pathPlanner.notifyPathExecutionFinished(true);
            m_pathPlanner.sendStatus("Path complete", false);
            return;
        }

        // Get target waypoint
        const Waypoint& target = m_currentPath.waypoints[m_currentWaypointIndex];

        // Calculate distance to waypoint
        double dx = target.x - m_currentX;
        double dy = target.y - m_currentY;
        double distance = std::sqrt(dx * dx + dy * dy);

        // If close enough, move to next waypoint
        if (distance < 0.1) { // 10cm threshold
            m_currentWaypointIndex++;
            std::cout << "Reached waypoint " << m_currentWaypointIndex
                     << " of " << m_currentPath.waypoints.size() << std::endl;
            return;
        }

        // Simple path following (you'd want something more sophisticated)
        // This is just an example - use proper trajectory following in production
        double targetHeading = std::atan2(dy, dx);
        double headingError = targetHeading - m_currentHeading;

        // Normalize heading error to [-pi, pi]
        while (headingError > M_PI) headingError -= 2 * M_PI;
        while (headingError < -M_PI) headingError += 2 * M_PI;

        // Simple proportional control (replace with PID or Ramsete in production)
        double forwardSpeed = std::min(target.velocity, distance * 0.5);
        double turnRate = headingError * 2.0;

        // Send commands to drive
        // m_drive.ArcadeDrive(forwardSpeed, turnRate);
    }

    /*
    // More advanced: Convert GUI waypoints to FRC trajectory
    void convertToTrajectory(const RobotPath& path) {
        std::vector<frc::Pose2d> poses;

        for (const auto& wp : path.waypoints) {
            poses.emplace_back(
                units::meter_t(wp.x),
                units::meter_t(wp.y),
                frc::Rotation2d(units::radian_t(wp.heading))
            );
        }

        // Generate trajectory
        frc::TrajectoryConfig config(
            units::meters_per_second_t(2.0),
            units::meters_per_second_squared_t(1.0)
        );

        m_trajectory = frc::TrajectoryGenerator::GenerateTrajectory(
            poses, config
        );

        // Use RamseteController to follow trajectory
    }
    */
};

#ifndef RUNNING_FRC_TESTS
int main() {
    return frc::StartRobot<Robot>();
}
#endif
