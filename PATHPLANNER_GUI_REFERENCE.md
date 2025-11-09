# PathPlanner GUI Reference

This document reflects the current robot/GUI integration in `GUI_Robot` and
describes how to operate the RobotPathPlanner interface now that the Windows
and macOS builds share the same behavior.

---

## 1. Robot ↔ GUI Data Flow

1. **TCP Port** – `PathPlannerComm` listens on port `5800`
   (`src/main/core/include/Robot.h:44`). The GUI must connect to
   `robot-ip:5800`.
2. **Startup** – Call `pathplanner_init()` during robot init
   (`src/main/core/include/Robot.h:165`) to spawn the TCP server threads
   (`src/main/pathplanner/src/PathPlannerComm.cpp:85`).
3. **Pose Streaming** – `pathplanner_update_odometry(false)` publishes the
   robot pose converted to meters/radians (`Robot.h:172`). `Movement::PositionDriver`
   already calls this helper roughly every 200 ms while the drivetrain is
   moving (`src/main/base_controller/src/Movement.cpp:32-43`).
4. **GUI Polling** – When the GUI sends `getState` the robot answers with
   both status and the most recent pose (`src/main/pathplanner/src/PathPlannerComm.cpp:381-390`).
5. **Path Ingestion** – Incoming paths are parsed, stored, and optionally
   delivered to a callback (`PathPlannerComm.cpp:340-379`). Use
   `pathplanner_getPathByName/Index` helpers (`Robot.h:199-235`) to retrieve
   them from robot code.

---

## 2. Typical Robot Loop

```cpp
#include "Robot.h"

void AutonomousInit() {
    pathplanner_init();
    pathplanner_list_paths();
}

void AutonomousPeriodic() {
    pathplanner_update_odometry(false);
    pathplanner_check_new_path();  // optional helper
}
```

- Execute a stored path by name:
  `pathplanner_execute_path("Pickup Cargo", true);`
  (`Robot.h:215`).
- To wait for GUI uploads, poll `pathPlanner.GetPathCount()` (see
  `PATHPLANNER_SMART_EXECUTION.md` Example 1).

---

## 3. GUI Workflow (RobotPathPlanner)

1. **Launch** `RobotPathPlanner` from the `RobotPathPlanner` sub-folder.
2. **Connect**
   - Enter the robot IP (USB device mode is `192.168.55.1`).
   - Press **Connect** – the GUI shows pose/heading once `pathplanner_update_odometry`
     runs on the robot.
3. **Author Paths**
   - Use the Paths panel to add, duplicate, or delete routes.
   - Click **Send to Robot** (no keyboard shortcut yet) to transmit the selected path.
4. **Monitor Status**
   - The GUI sends periodic `getState` requests; the robot replies with its
     pose so the live icon tracks your drivetrain (`PathPlannerComm.cpp:381-390`).

---

## 4. Keyboard Shortcuts (from code)

The GUI defines the following shortcuts via `QKeySequence` inside
`RobotPathPlanner/src/MainWindow.cpp:217-235`. These automatically map to
`Ctrl+` on Windows/Linux and `Cmd+` on macOS.

| Shortcut | Action | Source |
|----------|--------|--------|
| `Ctrl+N` / `Cmd+N` | New map (`MainWindow::newMap`) | `MainWindow.cpp:216-218` |
| `Ctrl+O` / `Cmd+O` | Open map (`MainWindow::openMap`) | `MainWindow.cpp:219-221` |
| `Ctrl+S` / `Cmd+S` | Save map (`MainWindow::saveMap`) | `MainWindow.cpp:222-224` |
| `Ctrl+Shift+S` / `Cmd+Shift+S` | Save map as… (`MainWindow::saveMapAs`) | `MainWindow.cpp:225-226` |
| `Ctrl+Q` / `Cmd+Q` | Quit application | `MainWindow.cpp:234-235` |

> Planned combos such as `Ctrl+R` for “Send to Robot” or `Ctrl++` for zoom are
> mentioned in README/Quickstart but not implemented yet. Use the toolbar or
> menu buttons for those actions until matching shortcuts are added.

### Navigation Gestures

| Gesture | Action | Reference |
|---------|--------|-----------|
| Mouse wheel | Zoom in/out | `RobotPathPlanner/QUICKSTART.md:94-96` |
| Pan tool + drag | Move the camera view | `RobotPathPlanner/QUICKSTART.md:90-98` |
| “Fit to View” button/menu | Center and scale entire map | `MainWindow.cpp:240-244` |

---

## 5. Troubleshooting

| Symptom | Checks |
|---------|--------|
| GUI stuck on “Waiting for pose” | Ensure the robot calls `pathplanner_update_odometry(false)` regularly (see `Movement.cpp:32-43`) and that the GUI is pointed at the correct IP/port 5800. |
| Paths never appear on robot | Confirm `pathplanner_init()` ran and no errors were logged inside `PathPlannerComm::ServerThread`. Use `pathplanner_list_paths()` to print counts. |
| GUI disconnects during drive | Watch for “Send failed” errors from `PathPlannerComm::SendMessage` (`PathPlannerComm.cpp:394-404`). Check network stability and keep only one GUI client connected at a time. |
| Build fails with Groovy/Gradle errors | Run Gradle with `JAVA_HOME` set to Java 11 (`Temurin 11`), matching the legacy Gradle 6 wrapper bundled with the project. |

---

Keep this guide alongside `PATHPLANNER_SMART_EXECUTION.md` for robot-side logic
and the `RobotPathPlanner/README.md` for GUI tool specifics so both teams stay
in sync across macOS (diff-drive) and Windows (GUI_Robot) builds.
