# Robot Integration Examples

This directory contains example code showing how to integrate the Path Planner GUI with your FRC robot.

## Files

### RobotIntegration.h / RobotIntegration.cpp
Reusable class that handles communication with the GUI. This provides:
- TCP server setup (you need to add socket implementation)
- Message parsing and sending
- Pose updates to GUI
- Path reception from GUI
- Status reporting

### ExampleRobotUsage.cpp
Complete example of a robot program that uses the Path Planner GUI. Shows:
- How to initialize the PathPlannerInterface
- Sending robot position to GUI in periodic loop
- Receiving paths from GUI
- Following waypoints
- Reporting execution status

## Quick Start

### 1. Add to Your Robot Project

Copy `RobotIntegration.h` and `RobotIntegration.cpp` to your robot project's source directory.

### 2. Add TCP Socket Implementation

You'll need to implement TCP communication. Options:

**Option A: WPILib TCPAcceptor/TCPConnector**
```cpp
#include <wpi/TCPAcceptor.h>
#include <wpi/TCPConnector.h>
```

**Option B: NetworkTables (Alternative)**
Use NetworkTables instead of raw TCP for easier setup:
```cpp
#include <networktables/NetworkTable.h>
```

**Option C: External library**
Use asio, boost::asio, or similar

### 3. Initialize in Robot Code

```cpp
class Robot : public frc::TimedRobot {
public:
    Robot() : m_pathPlanner(5800) {
        m_pathPlanner.onPathReceived([this](const RobotPath& path) {
            handleNewPath(path);
        });
    }

    void RobotPeriodic() override {
        // Update position
        m_pathPlanner.setRobotPose(getX(), getY(), getHeading());

        // Handle communication
        m_pathPlanner.periodic();
    }

private:
    PathPlannerInterface m_pathPlanner;

    void handleNewPath(const RobotPath& path) {
        // Execute the path
    }
};
```

### 4. Run Your Robot

1. Deploy robot code
2. Connect robot to network
3. Launch Path Planner GUI
4. Enter robot IP (e.g., 10.0.0.2)
5. Click "Connect"
6. Create and send paths!

## Integration Checklist

- [ ] Add RobotIntegration files to project
- [ ] Implement TCP socket communication
- [ ] Initialize PathPlannerInterface in robot constructor
- [ ] Call `setRobotPose()` with current position (from odometry)
- [ ] Call `periodic()` in RobotPeriodic
- [ ] Register path callback with `onPathReceived()`
- [ ] Implement path following logic
- [ ] Test connection from GUI
- [ ] Verify pose updates appear in GUI
- [ ] Test sending and executing paths

## Network Configuration

### Default Settings
- **Port**: 5800
- **Protocol**: TCP
- **Format**: Newline-delimited JSON

### FRC Network Setup
Typical roboRIO IP addresses:
- USB: `172.22.11.2`
- WiFi (competition): `10.TE.AM.2` (replace TE.AM with team number)
- WiFi (practice): Depends on router configuration

### Firewall
Ensure port 5800 is open on both robot and driver station computers.

## Message Protocol

### GUI → Robot

**Send Path:**
```json
{
  "type": "sendPath",
  "path": {
    "name": "Auto Path 1",
    "waypoints": [
      {"x": 1.0, "y": 2.0, "heading": 0.0, "velocity": 1.5}
    ]
  }
}
```

**Get State:**
```json
{
  "type": "getState"
}
```

### Robot → GUI

**Pose Update:**
```json
{
  "type": "robotPose",
  "x": 1.23,
  "y": 2.45,
  "heading": 1.57
}
```

**Status:**
```json
{
  "type": "status",
  "status": "Following path",
  "moving": true
}
```

## Path Following

### Simple Approach (Demo)
```cpp
void followPath(const RobotPath& path) {
    for (const auto& waypoint : path.waypoints) {
        // Drive to waypoint.x, waypoint.y
        // Set heading to waypoint.heading
        // Use waypoint.velocity as target speed
    }
}
```

### Production Approach (Recommended)
Use FRC trajectory following:
```cpp
#include <frc/trajectory/TrajectoryGenerator.h>
#include <frc/controller/RamseteController.h>

// Convert waypoints to trajectory
std::vector<frc::Pose2d> poses;
for (const auto& wp : path.waypoints) {
    poses.emplace_back(
        units::meter_t(wp.x),
        units::meter_t(wp.y),
        frc::Rotation2d(units::radian_t(wp.heading))
    );
}

auto trajectory = frc::TrajectoryGenerator::GenerateTrajectory(
    poses, trajectoryConfig
);

// Follow with RamseteController
```

## Coordinate Systems

### Important!
Make sure your robot's coordinate system matches the GUI:
- **Origin**: Set in GUI Settings panel
- **X-axis**: Typically forward/right
- **Y-axis**: Typically left/up
- **Heading**: Radians, 0 = pointing right (positive X)
- **Units**: Meters

### Converting from Different Systems
If your robot uses different conventions:
```cpp
// Example: If robot uses degrees instead of radians
double headingRadians = headingDegrees * M_PI / 180.0;

// Example: If Y-axis is flipped
double guiY = -robotY;
```

## Debugging

### Enable Debug Output
```cpp
m_pathPlanner.setDebugEnabled(true); // Add this method
```

### Check Connection
```cpp
if (!m_pathPlanner.isConnected()) {
    frc::SmartDashboard::PutString("GUI", "Disconnected");
} else {
    frc::SmartDashboard::PutString("GUI", "Connected");
}
```

### Monitor Messages
Add logging to `handleIncomingMessage()`:
```cpp
void handleIncomingMessage(const std::string& message) {
    std::cout << "Received: " << message << std::endl;
    // ...
}
```

## Tips

1. **Test Locally First**: Run GUI and robot code on same computer using `localhost`
2. **Use SmartDashboard**: Display connection status and path info
3. **Start Simple**: Test just pose updates before path following
4. **Validate Coordinates**: Print first waypoint, drive there manually to verify
5. **Error Handling**: Add try-catch around JSON parsing
6. **Rate Limiting**: Send poses at 20-50 Hz, not faster

## Alternative: NetworkTables Integration

If you prefer NetworkTables over TCP:

```cpp
auto table = nt::NetworkTableInstance::GetDefault().GetTable("pathplanner");

// Send pose
table->GetEntry("x").SetDouble(m_currentX);
table->GetEntry("y").SetDouble(m_currentY);
table->GetEntry("heading").SetDouble(m_currentHeading);

// Receive path (as JSON string)
auto pathJson = table->GetEntry("currentPath").GetString("");
```

Then modify the GUI to use NetworkTables client instead of TCP.

## Support

For questions specific to FRC robot programming, consult:
- WPILib documentation: https://docs.wpilib.org
- Chief Delphi forums: https://www.chiefdelphi.com
- Your team's mentors

For GUI-specific issues, refer to the main README.md.
