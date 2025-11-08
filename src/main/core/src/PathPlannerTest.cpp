/************************************
 * PathPlanner Integration Test
 * Demonstrates PathPlanner GUI communication
 * with odometry updates and path execution
 *************************************/

#include "Robot.h"

int PathPlannerTest() {

    std::cout << "\n\n";
    std::cout << "************************************************" << std::endl;
    std::cout << "**      PATHPLANNER INTEGRATION TEST          **" << std::endl;
    std::cout << "************************************************" << std::endl;
    std::cout << "\n";

    // Start MockDS for robot simulation
    Robot r;
    r.ds.Enable();

    // Initialize PathPlanner communication
    pathplanner_init();

    // Initialize robot position
    set_position(0, 0, 0);  // Start at origin

    std::cout << "\n[FRC] Robot initialized at origin (0, 0, 0)" << std::endl;
    std::cout << "[FRC] Waiting for GUI connection on port 5800..." << std::endl;
    std::cout << "[FRC] Open RobotPathPlanner GUI and connect to robot IP" << std::endl;
    std::cout << "\n";

    // Main loop - continuously update odometry and check for new paths
    int iteration = 0;
    bool running = true;

    std::cout << "\n[FRC] To stop the test, use the Stop button or Ctrl+C" << std::endl;
    std::cout << "[FRC] Starting main loop..." << std::endl << std::endl;

    while (running) {

        // Update odometry every iteration (send to GUI)
        // Pass false to reduce console spam - only prints every 20 updates
        pathplanner_update_odometry(false);

        // Check for new paths from GUI
        pathplanner_check_new_path();

        // Check stop button
        if (hard.GetStopButton()) {
            std::cout << "\n[FRC] Stop button pressed, exiting..." << std::endl;
            running = false;
        }

        // Delay 100ms between updates (10Hz main loop, but odometry sent at 20Hz internally)
        delay(100);

        iteration++;
    }

    std::cout << "\n[FRC] PathPlanner test completed" << std::endl;

    pathPlanner.Stop();
    r.ds.Disable();

    return 0;
}
