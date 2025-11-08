# PathPlanner Smart Execution Guide

**Advanced path execution with automatic nearest waypoint detection and intelligent routing.**

---

## 🚀 New Features

### 1. **Multiple Path Storage**
The robot now stores ALL paths sent from the GUI, not just the latest one.

### 2. **Smart Path Selection**
Execute any stored path by name or index.

### 3. **Nearest Waypoint Detection**
Automatically finds the closest waypoint to the robot's current position.

### 4. **Intelligent Direction Selection**
Decides whether to go forward or backward based on robot position and neighboring waypoints.

---

## 📚 API Reference

### Core Functions

#### `pathplanner_list_paths()`
Lists all stored paths with their names and waypoint counts.

```cpp
pathplanner_list_paths();
```

**Output:**
```
[FRC] ===== AVAILABLE PATHS =====
[FRC] Total paths stored: 3
[FRC] [0] "Path to Goal A" - 5 waypoints
[FRC] [1] "Return Path" - 4 waypoints
[FRC] [2] "Alternate Route" - 6 waypoints
[FRC] ==============================
```

---

#### `pathplanner_execute_path(name, executeFromNearest)`
Smart path execution with nearest waypoint detection.

**Parameters:**
- `name` (string) - Name of the path to execute
- `executeFromNearest` (bool) - If true, starts from nearest waypoint (default: true)

**Returns:** `bool` - true if successful, false if failed or stopped

```cpp
// Execute with smart routing
bool success = pathplanner_execute_path("Path to Goal A", true);

// Execute from start to end (ignore nearest)
bool success = pathplanner_execute_path("Path to Goal A", false);
```

**What it does:**
1. Finds the path by name
2. Calculates distance to all waypoints
3. Identifies the nearest waypoint
4. Determines optimal direction (forward/backward)
5. Executes the path intelligently
6. Handles stop button interruption

---

#### `pathplanner_execute_path_by_index(index, executeFromNearest)`
Execute path by its index in the stored paths list.

**Parameters:**
- `index` (int) - Index of the path (0-based)
- `executeFromNearest` (bool) - Smart routing enabled (default: true)

**Returns:** `bool` - true if successful

```cpp
// Execute first path with smart routing
pathplanner_execute_path_by_index(0, true);

// Execute second path from start
pathplanner_execute_path_by_index(1, false);
```

---

#### `pathplanner_check_new_path()`
Auto-executes newly received paths from GUI.

```cpp
// In your main loop
pathplanner_check_new_path();
```

**Behavior:**
- Checks if a new path arrived from GUI
- Automatically executes it with smart routing
- No manual intervention needed

---

## 🧠 Smart Routing Logic

### How It Works

```
Robot at position (2.5m, 3.0m)

Path "Route A" waypoints:
  [0] (1.0, 1.0) - distance: 2.5m
  [1] (2.0, 2.0) - distance: 1.4m  ← NEAREST
  [2] (3.0, 3.0) - distance: 0.7m
  [3] (4.0, 4.0) - distance: 2.1m

Decision:
1. Nearest waypoint = [1] at 1.4m
2. Check neighbors:
   - Distance to [0] = 2.5m
   - Distance to [2] = 0.7m
3. [2] is closer → Execute FORWARD from [1] to [3]
```

### Direction Logic

| Robot Position | Nearest Waypoint | Action |
|----------------|------------------|--------|
| Before path start | [0] | Execute forward [0]→[end] |
| After path end | [last] | Execute backward [last]→[0], then forward [1]→[end] |
| Middle of path | [n] | Check neighbors, go toward closer side |

---

## 💡 Usage Examples

### Example 1: Autonomous Mode with Multiple Paths

```cpp
int MainTask() {
    pathplanner_init();
    set_position(0, 0, 0);

    // Wait for GUI to send paths
    while (pathPlanner.GetPathCount() < 3) {
        pathplanner_update_odometry(false);
        delay(100);
    }

    // List all available paths
    pathplanner_list_paths();

    // Execute specific paths in sequence
    pathplanner_execute_path("Pickup Cargo", true);
    delay(500);

    pathplanner_execute_path("Deliver Cargo", true);
    delay(500);

    pathplanner_execute_path("Return Home", true);

    return 0;
}
```

---

### Example 2: Selective Path Execution

```cpp
void ExecutePathBasedOnSensor() {
    if (sensor->DetectTarget()) {
        // Target found - execute direct path
        pathplanner_execute_path("Direct Route", true);
    } else {
        // No target - execute search pattern
        pathplanner_execute_path("Search Pattern", false);
    }
}
```

---

### Example 3: Manual Path Selection

```cpp
void ManualPathSelection() {
    pathplanner_list_paths();

    // User selects path via dashboard or button
    int selectedPath = frc::SmartDashboard::GetNumber("Path Selection", 0);

    std::cout << "[FRC] Executing selected path: " << selectedPath << std::endl;
    pathplanner_execute_path_by_index(selectedPath, true);
}
```

---

### Example 4: Interrupted Path Recovery

```cpp
void PathExecutionWithRecovery() {
    bool success = pathplanner_execute_path("Main Path", true);

    if (!success) {
        std::cout << "[FRC] Path interrupted! Trying recovery path..." << std::endl;

        // Execute backup path from current position
        pathplanner_execute_path("Recovery Path", true);
    }
}
```

---

## 📊 Console Output Examples

### Listing Paths

```
[FRC] ===== AVAILABLE PATHS =====
[FRC] Total paths stored: 2
[FRC] [0] "Test Path 1" - 4 waypoints
[FRC] [1] "Test Path 2" - 3 waypoints
[FRC] ==============================
```

---

### Smart Execution Output

```
[FRC] ===== SMART PATH EXECUTION =====
[FRC] Path: "Test Path 1"
[FRC] Robot position: x=1.5m, y=2.0m
[FRC] Total waypoints: 4
[FRC]   WP[0] distance: 2.5m
[FRC]   WP[1] distance: 0.7m
[FRC]   WP[2] distance: 1.2m
[FRC]   WP[3] distance: 2.8m
[FRC] Nearest waypoint: [1] at distance 0.7m
[FRC] Direction: FORWARD to end
[FRC] ======================================
[FRC] → WP[1]: (200.0cm, 250.0cm, 45.0°)
[FRC] → WP[2]: (300.0cm, 300.0cm, 90.0°)
[FRC] → WP[3]: (400.0cm, 350.0cm, 135.0°)
[FRC] ✓ Path completed!
```

---

### Path Storage Messages

```
[PathPlanner] Added new path: Test Path 1 (Total paths: 1)
[PathPlanner] Added new path: Test Path 2 (Total paths: 2)
[PathPlanner] Updated existing path: Test Path 1
```

---

## 🎮 Workflow Example

### Competition Autonomous Routine

1. **Pre-Match Setup (GUI)**
   ```
   - Create "Start to Pickup" path
   - Create "Pickup to Goal" path
   - Create "Goal to Defense" path
   - Send all paths to robot
   ```

2. **Autonomous Init (Robot)**
   ```cpp
   void AutonomousInit() {
       pathplanner_init();
       set_position(0, 0, 0);  // Set starting position
       pathplanner_list_paths();  // Verify paths loaded
   }
   ```

3. **Autonomous Periodic (Robot)**
   ```cpp
   void AutonomousPeriodic() {
       // Phase 1: Go to pickup
       if (autonomousPhase == 1) {
           if (pathplanner_execute_path("Start to Pickup", true)) {
               autonomousPhase = 2;
           }
       }

       // Phase 2: Pickup action
       if (autonomousPhase == 2) {
           // Do pickup action
           oms.pickup_cargo();
           autonomousPhase = 3;
       }

       // Phase 3: Go to goal
       if (autonomousPhase == 3) {
           if (pathplanner_execute_path("Pickup to Goal", true)) {
               autonomousPhase = 4;
           }
       }

       // Phase 4: Score
       if (autonomousPhase == 4) {
           oms.release_cargo();
           autonomousPhase = 5;
       }

       // Phase 5: Defensive position
       if (autonomousPhase == 5) {
           pathplanner_execute_path("Goal to Defense", true);
           autonomousPhase = 0;  // Done
       }
   }
   ```

---

## ⚙️ Advanced Features

### Path Reversal
If the robot is near the end of a path, it will execute backward:

```
Robot near end of path:
1. Goes backward from [end] to [0]
2. Then forward from [1] to [end] (complete coverage)
```

### Error Handling
- Path not found → Shows available paths
- Empty path → Error message
- Stop button pressed → Graceful stop, returns false
- Exception during execution → Caught, returns false

---

## 🔧 Low-Level API

### PathPlannerComm Methods

```cpp
// Get all paths
std::vector<PathPlanner::Path> allPaths = pathPlanner.GetAllPaths();

// Get path by name
PathPlanner::Path path;
if (pathPlanner.GetPathByName("MyPath", path)) {
    // Use path
}

// Get path by index
PathPlanner::Path path;
if (pathPlanner.GetPathByIndex(0, path)) {
    // Use path
}

// Get count
int count = pathPlanner.GetPathCount();

// Clear all paths
pathPlanner.ClearPaths();
```

---

## 🧪 Testing Recommendations

### Test 1: Multi-Path Storage
1. Send 3 different paths from GUI
2. Call `pathplanner_list_paths()`
3. Verify all 3 are stored

### Test 2: Path Execution by Name
1. Send path named "TestPath"
2. Call `pathplanner_execute_path("TestPath", true)`
3. Verify robot executes correctly

### Test 3: Nearest Waypoint Detection
1. Position robot at (2.0, 2.0)
2. Create path with waypoints at (0,0), (1,1), (3,3), (4,4)
3. Execute with `executeFromNearest=true`
4. Verify it starts from nearest (1,1) or (3,3)

### Test 4: Direction Intelligence
1. Robot near end of path
2. Verify it executes backward first
3. Then forward to complete full path

### Test 5: Stop Button
1. Execute long path
2. Press stop button mid-execution
3. Verify graceful stop and `false` return

---

## 📈 Performance

| Metric | Value |
|--------|-------|
| Path storage | Unlimited (memory limited) |
| Lookup by name | O(n) linear search |
| Lookup by index | O(1) constant time |
| Distance calculation | O(n) for n waypoints |
| Thread safety | Fully thread-safe with mutexes |

---

## 🐛 Troubleshooting

### "Path not found" Error
```
[FRC] ERROR: Path "MyPath" not found!
```
**Solution:**
- Check path name spelling (case-sensitive)
- Call `pathplanner_list_paths()` to see available paths
- Ensure GUI sent the path

---

### Robot Goes Wrong Direction
**Solution:**
- Use `executeFromNearest=false` to force start-to-end
- Check waypoint order in GUI
- Verify robot position is correct

---

### Path Doesn't Execute
**Solution:**
- Check waypoints are not empty
- Verify robot code has `Movement.cpp` working
- Look for exception messages in console

---

## 🎯 Best Practices

1. **Always list paths** before executing to verify they're loaded
2. **Use descriptive names** for paths ("Pickup Blue Cargo" not "Path 1")
3. **Test with `executeFromNearest=false`** first to verify path is correct
4. **Check return values** to handle failures
5. **Use stop button** for safety during testing

---

## 🔄 Migration from Old API

### Old Code
```cpp
pathplanner_check_new_path();  // Auto-executes immediately
```

### New Code (More Control)
```cpp
// Option 1: Auto-execute (same as before)
pathplanner_check_new_path();

// Option 2: Manual execution with control
PathPlanner::Path path;
if (pathPlanner.GetLatestPath(path)) {
    // Do something before executing
    std::cout << "Got path: " << path.name << std::endl;

    // Execute with smart routing
    pathplanner_execute_path(path.name, true);
}
```

---

## 📝 Summary

**New capabilities:**
- ✅ Store multiple paths simultaneously
- ✅ Execute any path by name or index
- ✅ Automatic nearest waypoint detection
- ✅ Intelligent forward/backward routing
- ✅ Complete path coverage
- ✅ Stop button support
- ✅ Error handling and recovery

**Use cases:**
- Complex autonomous routines with multiple paths
- Dynamic path selection based on sensors
- Recovery from interruptions
- Competition mode with pre-loaded paths

---

**Ready to use smart path execution! 🤖🧠**

---

**Version:** 2.0
**Last Updated:** 2025-11-08
**Compatibility:** Works with existing PathPlannerTest code
