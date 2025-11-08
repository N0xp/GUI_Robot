# PathPlanner Quick Reference

**Fast lookup for PathPlanner functions and usage.**

---

## 📞 Function Calls

### Initialization
```cpp
pathplanner_init();  // Start TCP server on port 5800
```

### Odometry
```cpp
pathplanner_update_odometry();         // With reduced spam (every 20 calls)
pathplanner_update_odometry(true);     // Verbose (every call)
```

### List Paths
```cpp
pathplanner_list_paths();  // Show all stored paths
```

### Execute Paths

**By Name (Recommended):**
```cpp
pathplanner_execute_path("Path Name", true);   // Smart routing
pathplanner_execute_path("Path Name", false);  // Start to end only
```

**By Index:**
```cpp
pathplanner_execute_path_by_index(0, true);    // First path, smart routing
pathplanner_execute_path_by_index(1, false);   // Second path, sequential
```

**Auto-Execute New Paths:**
```cpp
pathplanner_check_new_path();  // Executes paths as they arrive from GUI
```

---

## 🎯 Common Patterns

### Pattern 1: Wait for Paths, Then Execute
```cpp
pathplanner_init();

// Wait for GUI to send paths
while (pathPlanner.GetPathCount() == 0) {
    pathplanner_update_odometry(false);
    delay(100);
}

// List available paths
pathplanner_list_paths();

// Execute specific path
pathplanner_execute_path("MyPath", true);
```

---

### Pattern 2: Sequential Path Execution
```cpp
pathplanner_execute_path("Path 1", true);
delay(500);
pathplanner_execute_path("Path 2", true);
delay(500);
pathplanner_execute_path("Path 3", true);
```

---

### Pattern 3: Conditional Path Selection
```cpp
if (sensor->DetectTarget()) {
    pathplanner_execute_path("Direct Route", true);
} else {
    pathplanner_execute_path("Search Pattern", true);
}
```

---

### Pattern 4: Auto-Execute in Loop
```cpp
while (running) {
    pathplanner_update_odometry(false);
    pathplanner_check_new_path();  // Auto-executes new paths
    delay(100);
}
```

---

## 🧠 Smart Routing Behavior

| `executeFromNearest` | Behavior |
|---------------------|----------|
| `true` | Finds nearest waypoint, goes optimal direction |
| `false` | Always starts at waypoint [0], goes to end |

---

## 📊 Return Values

All execute functions return `bool`:
- `true` = Path completed successfully
- `false` = Path failed, stopped by user, or not found

```cpp
bool success = pathplanner_execute_path("Path", true);
if (!success) {
    std::cout << "Path failed!" << std::endl;
}
```

---

## 🔍 Debugging

### Check Connection
```cpp
bool connected = pathPlanner.IsConnected();
std::cout << "Connected: " << (connected ? "YES" : "NO") << std::endl;
```

### Check Path Count
```cpp
int count = pathPlanner.GetPathCount();
std::cout << "Paths stored: " << count << std::endl;
```

### Clear All Paths
```cpp
pathPlanner.ClearPaths();
```

---

## ⚡ Quick Test Code

```cpp
int PathPlannerQuickTest() {
    // Initialize
    pathplanner_init();
    set_position(0, 0, 0);

    std::cout << "Waiting for paths..." << std::endl;

    // Wait for at least one path
    while (pathPlanner.GetPathCount() == 0) {
        pathplanner_update_odometry(false);
        delay(100);
    }

    // List all paths
    pathplanner_list_paths();

    // Execute first path
    std::cout << "Executing first path..." << std::endl;
    bool success = pathplanner_execute_path_by_index(0, true);

    std::cout << "Result: " << (success ? "SUCCESS" : "FAILED") << std::endl;

    return 0;
}
```

---

## 🚨 Common Errors

| Error Message | Fix |
|---------------|-----|
| `Path "X" not found` | Check spelling, call `pathplanner_list_paths()` |
| `Invalid path index` | Path count is lower, call `pathplanner_list_paths()` |
| `Path has no waypoints` | Recreate path in GUI with waypoints |
| Connection issues | Check robot IP, port 5800, firewall |

---

## 📚 Full Documentation

- **Installation:** [WINDOWS_INSTALL_GUIDE.md](RobotPathPlanner/WINDOWS_INSTALL_GUIDE.md)
- **Integration:** [PATHPLANNER_INTEGRATION.md](PATHPLANNER_INTEGRATION.md)
- **Smart Execution:** [PATHPLANNER_SMART_EXECUTION.md](PATHPLANNER_SMART_EXECUTION.md)
- **Fixes:** [PATHPLANNER_FIXES.md](PATHPLANNER_FIXES.md)
- **Complete Guide:** [PATHPLANNER_COMPLETE_GUIDE.md](PATHPLANNER_COMPLETE_GUIDE.md)

---

**Keep this handy for quick lookups! 📋**
