# PathPlanner Integration Guide

## Overview

The differential robot project has been integrated with the RobotPathPlanner GUI from the UAE-Cpp project. This allows you to:

1. **Visualize** the robot's position in real-time on a 2D map
2. **Plan paths** using a graphical interface with waypoints
3. **Send paths** from the GUI to the robot via TCP/IP
4. **Execute paths** automatically on the robot
5. **Monitor** path execution status

---

## Architecture

```
┌─────────────────────────────────┐         TCP/IP (Port 5800)         ┌──────────────────────────┐
│  RobotPathPlanner GUI (Desktop) │◄──────────────────────────────────►│  Robot (VMX-Pi/RoboRIO)  │
│                                 │                                    │                          │
│  - Path planning                │         JSON Messages              │  - Path execution        │
│  - Robot visualization          │         - Robot pose (20Hz)        │  - Odometry updates      │
│  - Map editing                  │         - Path commands            │  - Movement control      │
└─────────────────────────────────┘         - Status updates           └──────────────────────────┘
```

---

## File Structure

```
differential robot/
├── RobotPathPlanner/                    # Qt GUI Application (copied from UAE-Cpp)
│   ├── src/                             # GUI source files
│   ├── include/                         # GUI header files
│   ├── CMakeLists.txt                   # Qt build configuration
│   └── README.md                        # GUI documentation
│
├── src/main/pathplanner/                # Robot communication module
│   ├── include/
│   │   └── PathPlannerComm.h           # Communication class header
│   └── src/
│       └── PathPlannerComm.cpp         # TCP/IP and JSON implementation
│
├── src/main/core/
│   ├── include/
│   │   ├── Robot.h                     # PathPlanner integration functions
│   │   └── main.h                      # Function declarations
│   └── src/
│       └── PathPlannerTest.cpp         # Example integration test
│
└── build.gradle                         # Updated with pathplanner module
```

---

## Setup Instructions

### 1. Build the Robot Code

The PathPlanner communication module is now integrated into the build system:

```bash
cd "differential robot"
./gradlew build
```

### 2. Build the GUI Application

The RobotPathPlanner GUI is a separate Qt application that runs on your laptop/desktop:

```bash
cd RobotPathPlanner
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt ..
cmake --build .
```

For detailed Qt installation instructions, see `RobotPathPlanner/README.md`.

---

## Usage

### Step 1: Deploy Robot Code

Deploy the robot code to your robot (VMX-Pi or RoboRIO):

```bash
./gradlew deploy
```

### Step 2: Run PathPlannerTest

On the robot, enable the `PathPlannerTest()` function in `main.cpp`:

```cpp
int main(){
    PathPlannerTest();  // Enable this line
    // Inspection();
    return 0;
}
```

Rebuild and deploy.

### Step 3: Start the GUI

On your laptop/desktop, run the RobotPathPlanner GUI:

```bash
cd RobotPathPlanner/build
./RobotPathPlanner
```

### Step 4: Connect to Robot

1. In the GUI, go to the **Robot** panel
2. Enter the robot's IP address (default: `10.0.0.2` or your robot's IP)
3. Port should be `5800` (default)
4. Click **Connect**

You should see:
- Robot position updating in real-time on the map
- Console output showing odometry updates

### Step 5: Create and Send a Path

1. In the **Paths** panel, click **New Path**
2. Select the "Draw Path" tool
3. Click on the map to add waypoints
4. Double-click waypoints to edit their properties:
   - Position (x, y in meters)
   - Heading (radians)
   - Velocity (m/s)
5. Click **Send Path** to transmit to robot
6. Watch the robot execute the path!

---

## Code Integration Examples

### Basic Integration in Your Code

```cpp
#include "Robot.h"

int main() {
    // Initialize PathPlanner
    pathplanner_init();

    // Set initial robot position
    set_position(0, 0, 0);

    while (true) {
        // Update odometry (sends position to GUI)
        pathplanner_update_odometry();

        // Check for new paths from GUI and execute them
        pathplanner_check_new_path();

        delay(100);  // 10Hz update rate
    }

    return 0;
}
```

### Manual Path Execution

```cpp
// Get path without auto-execution
PathPlanner::Path path;
if (pathPlanner.GetLatestPath(path)) {
    std::cout << "Received path: " << path.name << std::endl;

    for (const auto& wp : path.waypoints) {
        // Convert to your units (meters to cm, radians to degrees)
        double x_cm = wp.x * 100.0;
        double y_cm = wp.y * 100.0;
        double heading_deg = (wp.heading * 180.0) / M_PI;

        // Your custom movement function
        movement.PositionDriver(x_cm, y_cm, heading_deg);
    }
}
```

### Sending Status Updates

```cpp
// Notify GUI of robot status
pathPlanner.SendStatus("idle", false);
pathPlanner.SendStatus("moving", true);

// Notify path execution events
pathPlanner.NotifyPathExecutionStarted();
pathPlanner.NotifyPathExecutionFinished(true);  // true = success
```

---

## Communication Protocol

### Messages: Robot → GUI

**1. Robot Pose Update (20Hz)**
```json
{
  "type": "robotPose",
  "x": 1.5,
  "y": 2.3,
  "heading": 1.57
}
```

**2. Status Update**
```json
{
  "type": "status",
  "status": "idle",
  "moving": false
}
```

**3. Path Execution Events**
```json
{"type": "pathExecutionStarted"}
{"type": "pathExecutionFinished", "success": true}
```

### Messages: GUI → Robot

**1. Send Path**
```json
{
  "type": "sendPath",
  "path": {
    "name": "Path 1",
    "waypoints": [
      {"x": 1.0, "y": 2.0, "heading": 0.0, "velocity": 1.5},
      {"x": 3.0, "y": 4.0, "heading": 1.57, "velocity": 1.0}
    ]
  }
}
```

**2. Get State**
```json
{"type": "getState"}
```

---

## Print Statements for Debugging

The integration includes comprehensive print statements with `[FRC]` and `[PathPlanner]` tags:

### Odometry Updates
```
[FRC] ===== ODOMETRY UPDATE =====
[FRC] Sending to GUI: x=1.5m, y=2.3m, heading=1.57rad (90deg)
[FRC] Connected: YES
[FRC] ==============================
```

### Path Reception
```
[PathPlanner] ===== PATH MESSAGE RECEIVED =====
[PathPlanner] Message type: sendPath
[PathPlanner] Full message: {"type":"sendPath",...}
[PathPlanner] ===== PATH DATA =====
[PathPlanner] Path name: Path 1
[PathPlanner] Number of waypoints: 3
[PathPlanner]   Waypoint 0: x=1.0m, y=2.0m, heading=0.0rad, velocity=1.5m/s
[PathPlanner]   Waypoint 1: x=3.0m, y=4.0m, heading=1.57rad, velocity=1.0m/s
[PathPlanner] =================================
```

### Path Execution
```
[FRC] ===== NEW PATH RECEIVED FROM GUI =====
[FRC] Path name: Path 1
[FRC] Number of waypoints: 2
[FRC]   Waypoint 0: x=1.0m, y=2.0m, heading=0.0rad, velocity=1.5m/s
[FRC] Moving to waypoint: x=100.0cm, y=200.0cm, heading=0.0deg
[FRC] Path execution completed!
```

---

## API Reference

### Initialization
```cpp
void pathplanner_init()
```
Starts the TCP server on port 5800. Call this once during robot initialization.

### Odometry Update
```cpp
void pathplanner_update_odometry()
```
Sends current robot position to GUI. Call this periodically (10-20Hz recommended).
- Converts position from cm to meters
- Converts heading from degrees to radians
- Prints debug information

### Path Check
```cpp
void pathplanner_check_new_path()
```
Checks for new paths from GUI and executes them automatically.
- Prints received path details
- Converts waypoints to robot units
- Executes path using `movement.PositionDriver()`
- Sends execution status to GUI

### Low-Level API

```cpp
PathPlanner::PathPlannerComm pathPlanner(5800);

// Start/Stop
pathPlanner.Start();
pathPlanner.Stop();

// Check connection
bool connected = pathPlanner.IsConnected();

// Update pose
pathPlanner.UpdateRobotPose(x_meters, y_meters, heading_radians);

// Get latest path
PathPlanner::Path path;
if (pathPlanner.GetLatestPath(path)) {
    // Process path
}

// Send status
pathPlanner.SendStatus("idle", false);

// Path execution notifications
pathPlanner.NotifyPathExecutionStarted();
pathPlanner.NotifyPathExecutionFinished(true);
```

---

## Units and Coordinate Systems

| Parameter | Robot Code | GUI | Conversion |
|-----------|-----------|-----|------------|
| Position X/Y | centimeters | meters | `meters = cm / 100.0` |
| Heading | degrees | radians | `radians = deg * π / 180` |
| Velocity | cm/s | m/s | `m/s = cm/s / 100.0` |

**Coordinate System:**
- Origin: Configurable via GUI
- X-axis: Positive = forward/right
- Y-axis: Positive = left/up
- Heading: 0° = East, 90° = North (counter-clockwise)

---

## Troubleshooting

### GUI Won't Connect
1. Check robot IP address is correct
2. Verify port 5800 is not blocked by firewall
3. Ensure robot code is running with `pathplanner_init()` called
4. Check console output for `[PathPlanner] Server listening on port 5800`

### Robot Not Moving
1. Verify path waypoints are in valid range
2. Check `[FRC]` console output for path reception
3. Ensure `movement.PositionDriver()` is functioning
4. Verify robot hardware is enabled

### Odometry Not Updating
1. Check if `pathplanner_update_odometry()` is being called periodically
2. Verify `movement.get_x/y/th()` returns valid values
3. Look for `[FRC] Connected: YES` in console output

### Build Errors
1. Ensure `build.gradle` includes `pathplanner` module
2. Check all header files are in include path
3. For Qt GUI: Verify Qt installation and `CMAKE_PREFIX_PATH`

---

## Testing Checklist

- [ ] Robot code builds successfully
- [ ] GUI application builds and runs
- [ ] TCP connection established (check console: "GUI connected!")
- [ ] Odometry updates visible in console (`[FRC] ===== ODOMETRY UPDATE =====`)
- [ ] Robot position displays on GUI map
- [ ] Can create paths in GUI
- [ ] Path sent from GUI appears in robot console (`[PathPlanner] ===== PATH MESSAGE RECEIVED =====`)
- [ ] Robot executes received path
- [ ] Path execution status sent to GUI

---

## Advanced Features

### Custom Path Execution
Implement your own path following algorithm:

```cpp
static void custom_path_executor() {
    PathPlanner::Path path;
    if (pathPlanner.GetLatestPath(path)) {
        // Your custom trajectory follower
        // Pure pursuit, PID, etc.
    }
}
```

### Obstacle Avoidance
Integrate sensor data:

```cpp
pathplanner_update_odometry();

// Add obstacle detection
if (lidar.GetLidarFront() < 30) {
    pathPlanner.SendStatus("obstacle detected", false);
    // Stop or replan
}
```

### Path Recording
Record executed paths:

```cpp
std::vector<PathPlanner::Waypoint> recorded_path;

// During teleoperation
PathPlanner::Waypoint wp;
wp.x = movement.get_x() / 100.0;
wp.y = movement.get_y() / 100.0;
wp.heading = (movement.get_th() * M_PI) / 180.0;
wp.velocity = 1.0;
recorded_path.push_back(wp);
```

---

## Contributing

To extend the PathPlanner integration:

1. **Add new message types**: Edit `PathPlannerComm.cpp::HandleMessage()`
2. **Modify protocol**: Update JSON parsing in `PathPlannerComm.cpp`
3. **Enhance GUI**: See `RobotPathPlanner/README.md`

---

## Support

- **GUI Documentation**: `RobotPathPlanner/README.md`
- **Quick Start**: `RobotPathPlanner/QUICKSTART.md`
- **Example Code**: `src/main/core/src/PathPlannerTest.cpp`

---

## License

PathPlanner GUI: See `RobotPathPlanner/` directory
Robot Integration: Part of differential robot project

---

**Happy Path Planning! 🤖🗺️**
