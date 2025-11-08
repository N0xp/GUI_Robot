# Robot Path Planner

A comprehensive GUI application for creating maps, planning robot paths, and visualizing real-time robot movement for FRC robots.

## Features

### Map Editor
- **Draw Lines/Walls**: Create maps by drawing lines that represent walls and obstacles
- **Dimension Display**: Each line automatically shows its length in meters
- **Connected Lines**: Lines connect at endpoints to form complete maps
- **Grid System**: Configurable grid with adjustable size (meters)
- **Save/Load Maps**: Export and import maps in JSON format for use in robot code

### Path Planning
- **Multiple Paths**: Create and manage multiple robot paths
- **Waypoint Editor**: Click to add waypoints with position, heading, and velocity
- **Visual Editing**: See paths overlaid on the map
- **Path Export**: Save paths to JSON for robot execution
- **Send to Robot**: Directly send paths to the robot via network connection

### Robot Visualization
- **Real-time Position**: See robot position updated live on the map
- **Customizable Shape**: Switch between rectangle, square, and triangle
- **Size Configuration**: Adjust robot width and length
- **Heading Indicator**: Visual arrow showing robot orientation
- **Distance Measurement**: Display distance from robot to nearest wall

### Robot Communication
- **TCP/JSON Protocol**: Simple JSON-based protocol over TCP
- **Live Updates**: 20Hz robot pose updates
- **Bidirectional**: Send commands and receive telemetry
- **Path Execution**: Send paths and monitor execution status

### Additional Tools
- **Measurement Tool**: Click and drag to measure distances on the map
- **Zoom & Pan**: Navigate large maps easily
- **Origin Configuration**: Set map reference point
- **Grid Customization**: Adjust grid size for different field sizes

## Building the Application

### Prerequisites

**macOS:**
```bash
# Install Qt using Homebrew
brew install qt

# Or download Qt from https://www.qt.io/download
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install qt6-base-dev qt6-tools-dev cmake build-essential
# Or for Qt5:
sudo apt-get install qtbase5-dev qttools5-dev cmake build-essential
```

**Windows:**
- Download and install Qt from https://www.qt.io/download
- Install CMake from https://cmake.org/download/
- Install Visual Studio or MinGW

### Build Instructions

1. **Clone or navigate to the project:**
```bash
cd RobotPathPlanner
```

2. **Create build directory:**
```bash
mkdir build
cd build
```

3. **Configure with CMake:**
```bash
# Let CMake find Qt automatically
cmake ..

# Or specify Qt path manually if needed:
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 ..
# Example macOS with Homebrew Qt:
# cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt ..
```

4. **Build:**
```bash
cmake --build .
```

5. **Run:**
```bash
./RobotPathPlanner
```

## Usage Guide

### Creating a Map

1. Click the **"Draw Line"** tool in the toolbar
2. Click to start a line, drag, and release to finish
3. Each line's length is displayed automatically
4. Continue drawing lines to create walls and boundaries
5. Lines can connect by starting/ending at the same point

### Creating a Path

1. Create a new path: **Paths panel** → **"New"** button
2. Select the **"Draw Path"** tool
3. Click on the map to add waypoints
4. Waypoints are connected automatically in sequence
5. The path length is displayed in the Paths panel

### Configuring the Robot

1. Open the **Robot** panel
2. Select robot shape: Rectangle, Square, or Triangle
3. Adjust **Width** and **Length** (in meters)
4. Changes are reflected immediately in the visualization

### Connecting to the Robot

1. Enter robot IP address (default: 10.0.0.2 for FRC robots)
2. Click **"Connect"**
3. When connected, robot position updates automatically
4. Distance to nearest wall is displayed in the status bar

### Sending a Path to the Robot

1. Create and select a path
2. Connect to the robot
3. Click **"Send to Robot"** in the Paths panel
4. Robot will begin executing the path

### Measuring Distances

1. Select the **"Measure"** tool
2. Click and drag between two points
3. Distance is displayed on the line and in the status bar

### Saving and Loading

**Save Map:**
- File → Save Map (Ctrl+S)
- Saves all walls/lines to JSON

**Load Map:**
- File → Open Map (Ctrl+O)

**Save Paths:**
- Paths panel → "Save All Paths"
- Saves all paths with waypoints to JSON

**Load Paths:**
- Paths panel → "Load Paths"

### Keyboard Shortcuts

- **Ctrl+N** - New Map
- **Ctrl+O** - Open Map
- **Ctrl+S** - Save Map
- **Ctrl+R** - Send Path to Robot
- **Ctrl++** - Zoom In
- **Ctrl+-** - Zoom Out
- **F** - Fit to View

## File Formats

### Map File (JSON)
```json
{
  "name": "Competition Field",
  "gridSize": 1.0,
  "origin": { "x": 0.0, "y": 0.0 },
  "lines": [
    {
      "start": { "x": 0.0, "y": 0.0 },
      "end": { "x": 5.0, "y": 0.0 },
      "length": 5.0
    }
  ]
}
```

### Path File (JSON)
```json
{
  "activePathIndex": 0,
  "paths": [
    {
      "name": "Autonomous Path 1",
      "color": "#0000ff",
      "visible": true,
      "waypoints": [
        {
          "x": 1.0,
          "y": 1.0,
          "heading": 0.0,
          "velocity": 1.5
        }
      ]
    }
  ]
}
```

## Robot Communication Protocol

The application uses a simple TCP connection with newline-delimited JSON messages.

### Messages from GUI to Robot

**Send Path:**
```json
{
  "type": "sendPath",
  "path": {
    "name": "Path 1",
    "waypoints": [...]
  }
}
```

**Request State:**
```json
{
  "type": "getState"
}
```

**Set Robot Shape:**
```json
{
  "type": "setRobotShape",
  "shape": "rectangle"
}
```

### Messages from Robot to GUI

**Robot Pose Update:**
```json
{
  "type": "robotPose",
  "x": 2.5,
  "y": 1.3,
  "heading": 1.57
}
```

**Status Update:**
```json
{
  "type": "status",
  "status": "idle",
  "moving": false
}
```

**Path Execution:**
```json
{
  "type": "pathExecutionStarted"
}
{
  "type": "pathExecutionFinished",
  "success": true
}
```

## Robot-Side Integration

See the `examples/` directory for sample robot code that integrates with this GUI.

### Quick Integration

1. Create a TCP server on port 5800
2. Parse incoming JSON messages
3. Send robot pose at ~20Hz
4. Execute received paths

### Example C++ Robot Code Snippet

```cpp
// In your robot code, send pose updates:
void sendPoseUpdate() {
    json msg;
    msg["type"] = "robotPose";
    msg["x"] = getCurrentX();
    msg["y"] = getCurrentY();
    msg["heading"] = getCurrentHeading();

    std::string msgStr = msg.dump() + "\n";
    socket->send(msgStr);
}

// Call this periodically (20Hz recommended)
```

## Tips and Best Practices

1. **Set Grid Size**: Match your grid size to your field (1m for practice, 0.5m for detailed work)
2. **Origin Point**: Set the origin to match your robot's coordinate system
3. **Units**: All measurements are in meters, angles in radians
4. **Path Velocity**: Set appropriate velocities for each waypoint (m/s)
5. **Save Often**: Use Ctrl+S frequently to avoid losing work
6. **Test Paths**: Use the measurement tool to verify path distances

## Troubleshooting

**Can't connect to robot:**
- Verify robot is powered on and on the network
- Check IP address (roboRIO usually at 10.TE.AM.2)
- Ensure firewall allows TCP port 5800

**Map doesn't save:**
- Check file permissions
- Ensure directory exists

**Robot not moving on map:**
- Verify robot is sending pose updates
- Check console for connection errors

**Build errors:**
- Ensure Qt is properly installed
- Verify CMAKE_PREFIX_PATH points to Qt installation
- Check CMake output for missing dependencies

