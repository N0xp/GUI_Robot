/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#include "main.h"

#include "Hardware.h"
#include "Movement.h"
#include "Sensors.h"
#include "Camera.h"
#include "lidar.h"
#include "Oms.h"
#include "ManualDrive.h"
#include "PathPlannerComm.h"

#include <dfs.h>
#include <limits>
#include <cmath>

class Robot : public frc::TimedRobot {
 public:
  studica::MockDS ds;

  private:

};

inline Hardware hard;
inline Sensor sensor( &hard );
inline Movement movement( &hard, &sensor );
inline Lidar lidar( &movement, &sensor );
inline Oms oms( &hard );
inline Camera cam( &movement, &oms, &hard );
inline OI oi;
inline Drive drive( &hard, &movement, &oi );
inline PathPlanner::PathPlannerComm pathPlanner(5800);

static double SL(){
  return lidar.GetLidarLeft() * 100 + offset_side;
}
static double SR(){
  return lidar.GetLidarRight() * 100 + offset_side;
}
static double SF(){
  return lidar.GetLidarFront() * 100 + offset_front;
}
static double SBR(){
  return sensor.GetRightUS() + offset_back;
}
static double SBL(){
  return sensor.GetLeftUS() + offset_back ;
}

static double setAngle(){
  float ang = movement.get_th();
  return sensor.setAngle(ang);
}

static void set_gripper( int ang ){
  oms.set_gripper( ang );
}
static void set_base( float ang ){
  oms.set_base( ang );
}
static void set_arm( float ang ){
  oms.set_arm( ang );
}
static void reset_height( int direction ){
  oms.reset(direction);
}
static void oms_driver( float height ){
  oms.oms_driver( height, 0 );
}



static bool get_stop_button(){
  return hard.GetStopButton();
}
static bool get_start_button(){
  return hard.GetStartButton();
}
static void led_red( bool state ){
  hard.SetStoppedLED( state );
}
static void led_green( bool state ){
  hard.SetRunningLED( state );
}
static void start_button(){
  while( get_start_button() ){ delay(50);}
}

static void position_driver(float x, float y, float th){
  movement.PositionDriver( x, y, th );
}
static void set_position(float x, float y, float th){
  movement.SetPosition( x, y, th );
}
static void linear_align( double dist, std::string direction ){
  lidar.linear_align( dist, direction );
}
static void angular_align(  ){
  movement.angular_align();
}
static void linear_increment( float dist, std::string direction ){
  movement.linear_increment( dist, direction );
}
static void line_align( std::string direction ){
  movement.line_align( direction );
}

static double get_x(){
  return movement.get_x();
}
static double get_y(){
  return movement.get_y();
}
static double get_th(){
  return movement.get_th();
}

static void DetectFruitArea( std::vector<std::string> fruits ){
  bool debug = false;
  bool use_area = true;
  double ang = straight_ang( movement.get_th() );
  cam.DetectFruit( fruits, ang, debug, use_area );
}
static void DetectFruitSharp( std::vector<std::string> fruits ){
  bool debug = false;
  bool use_area = false;
  double ang = straight_ang( movement.get_th() );
  cam.DetectFruit( fruits, ang, debug, use_area );
}

static Coord prev_Coord;

static std::vector<std::vector<double>> best_way( Coord &d, Coord &c ){
  std::vector<Coord> path;
  std::vector<std::vector<double>> d_path;
  double dist = 0;
  std::vector<std::string> checked;
 
  dfs( &d, &c, path, 0, d_path, dist, checked );
 
  prev_Coord = d;
 
  return d_path;
}
static void path_driver( std::vector<std::vector<double>> path ){
  for( std::vector<double> p : path ){
    // movement.PositionDriver( p[0], p[1], p[2] );
    // std::cout << "x: " << p[0] << " y: " << p[1] << " th: " << p[2] << std::endl;
  }
}

// PathPlanner functions
static void pathplanner_init(){
  std::cout << "[FRC] ===== STARTING PATHPLANNER COMMUNICATION =====" << std::endl;
  pathPlanner.Start();
  std::cout << "[FRC] PathPlanner communication started on port 5800" << std::endl;
  std::cout << "[FRC] =============================================" << std::endl;
}

static void pathplanner_update_odometry(bool verbose = false){
  // Get current robot position (convert cm to meters)
  double x_meters = movement.get_x() / 100.0;
  double y_meters = movement.get_y() / 100.0;
  double heading_radians = (movement.get_th() * M_PI) / 180.0;

  // Update PathPlanner with current pose
  pathPlanner.UpdateRobotPose(x_meters, y_meters, heading_radians);

  // Print odometry for debugging (only if verbose or every 20 calls)
  static int update_count = 0;
  update_count++;

  if (verbose || update_count % 20 == 0) {
    std::cout << "[FRC] ODOM: x=" << x_meters << "m, y=" << y_meters
              << "m, θ=" << movement.get_th() << "° | Connected: "
              << (pathPlanner.IsConnected() ? "YES" : "NO") << std::endl;
  }
}

// Calculate distance between two points
static double calculate_distance(double x1, double y1, double x2, double y2) {
  double dx = x2 - x1;
  double dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
}

// List all available paths
static void pathplanner_list_paths() {
  int count = pathPlanner.GetPathCount();
  std::cout << "[FRC] ===== AVAILABLE PATHS =====" << std::endl;
  std::cout << "[FRC] Total paths stored: " << count << std::endl;

  std::vector<PathPlanner::Path> allPaths = pathPlanner.GetAllPaths();
  for (size_t i = 0; i < allPaths.size(); i++) {
    const auto& p = allPaths[i];
    std::cout << "[FRC] [" << i << "] \"" << p.name << "\" - "
              << p.waypoints.size() << " waypoints" << std::endl;
  }
  std::cout << "[FRC] ==============================" << std::endl;
}

// Smart path execution: finds nearest waypoint and executes intelligently
static bool pathplanner_execute_path(const std::string& pathName, bool executeFromNearest = true) {
  PathPlanner::Path path;

  // Get the path by name
  if (!pathPlanner.GetPathByName(pathName, path)) {
    std::cout << "[FRC] ERROR: Path \"" << pathName << "\" not found!" << std::endl;
    pathplanner_list_paths();
    return false;
  }

  if (path.waypoints.empty()) {
    std::cout << "[FRC] ERROR: Path \"" << pathName << "\" has no waypoints!" << std::endl;
    return false;
  }

  // Get current robot position (in meters)
  double robot_x_m = movement.get_x() / 100.0;
  double robot_y_m = movement.get_y() / 100.0;

  std::cout << "[FRC] ===== SMART PATH EXECUTION =====" << std::endl;
  std::cout << "[FRC] Path: \"" << path.name << "\"" << std::endl;
  std::cout << "[FRC] Robot position: x=" << robot_x_m << "m, y=" << robot_y_m << "m" << std::endl;
  std::cout << "[FRC] Total waypoints: " << path.waypoints.size() << std::endl;

  int startIndex = 0;
  bool executeForward = true;

  if (executeFromNearest && path.waypoints.size() > 1) {
    // Find nearest waypoint
    double minDist = std::numeric_limits<double>::max();
    int nearestIndex = 0;

    for (size_t i = 0; i < path.waypoints.size(); i++) {
      const auto& wp = path.waypoints[i];
      double dist = calculate_distance(robot_x_m, robot_y_m, wp.x, wp.y);

      std::cout << "[FRC]   WP[" << i << "] distance: " << dist << "m" << std::endl;

      if (dist < minDist) {
        minDist = dist;
        nearestIndex = i;
      }
    }

    startIndex = nearestIndex;
    std::cout << "[FRC] Nearest waypoint: [" << nearestIndex << "] at distance " << minDist << "m" << std::endl;

    // Determine direction based on position in path
    if (nearestIndex == 0) {
      executeForward = true;
      std::cout << "[FRC] Direction: FORWARD (from beginning)" << std::endl;
    } else if (nearestIndex == static_cast<int>(path.waypoints.size()) - 1) {
      executeForward = false;
      std::cout << "[FRC] Direction: BACKWARD (from end to start)" << std::endl;
    } else {
      // In middle - check which neighbor is closer
      double distToNext = calculate_distance(robot_x_m, robot_y_m,
                                             path.waypoints[nearestIndex + 1].x,
                                             path.waypoints[nearestIndex + 1].y);
      double distToPrev = calculate_distance(robot_x_m, robot_y_m,
                                             path.waypoints[nearestIndex - 1].x,
                                             path.waypoints[nearestIndex - 1].y);

      if (distToPrev < distToNext) {
        executeForward = false;
        std::cout << "[FRC] Direction: BACKWARD then FORWARD" << std::endl;
      } else {
        executeForward = true;
        std::cout << "[FRC] Direction: FORWARD to end" << std::endl;
      }
    }
  } else {
    std::cout << "[FRC] Executing from start to end" << std::endl;
  }

  std::cout << "[FRC] ======================================" << std::endl;

  // Notify GUI
  pathPlanner.NotifyPathExecutionStarted();
  bool success = true;

  try {
    if (executeForward) {
      // Execute from startIndex to end
      for (size_t i = startIndex; i < path.waypoints.size(); i++) {
        const auto& wp = path.waypoints[i];
        double x_cm = wp.x * 100.0;
        double y_cm = wp.y * 100.0;
        double heading_deg = (wp.heading * 180.0) / M_PI;

        std::cout << "[FRC] → WP[" << i << "]: (" << x_cm << "cm, "
                  << y_cm << "cm, " << heading_deg << "°)" << std::endl;

        movement.PositionDriver(x_cm, y_cm, heading_deg);

        if (hard.GetStopButton()) {
          std::cout << "[FRC] STOPPED by user!" << std::endl;
          success = false;
          break;
        }
      }
    } else {
      // Execute backward then forward for complete coverage
      for (int i = startIndex; i >= 0; i--) {
        const auto& wp = path.waypoints[i];
        double x_cm = wp.x * 100.0;
        double y_cm = wp.y * 100.0;
        double heading_deg = (wp.heading * 180.0) / M_PI;

        std::cout << "[FRC] ← WP[" << i << "]: (" << x_cm << "cm, "
                  << y_cm << "cm, " << heading_deg << "°)" << std::endl;

        movement.PositionDriver(x_cm, y_cm, heading_deg);

        if (hard.GetStopButton()) {
          success = false;
          break;
        }
      }

      if (success && startIndex < static_cast<int>(path.waypoints.size()) - 1) {
        for (size_t i = 1; i < path.waypoints.size(); i++) {
          const auto& wp = path.waypoints[i];
          double x_cm = wp.x * 100.0;
          double y_cm = wp.y * 100.0;
          double heading_deg = (wp.heading * 180.0) / M_PI;

          std::cout << "[FRC] → WP[" << i << "]: (" << x_cm << "cm, "
                    << y_cm << "cm, " << heading_deg << "°)" << std::endl;

          movement.PositionDriver(x_cm, y_cm, heading_deg);

          if (hard.GetStopButton()) {
            success = false;
            break;
          }
        }
      }
    }
  } catch (...) {
    success = false;
  }

  pathPlanner.NotifyPathExecutionFinished(success);
  std::cout << (success ? "[FRC] ✓ Path completed!" : "[FRC] ✗ Path failed!") << std::endl;

  return success;
}

// Execute path by index
static bool pathplanner_execute_path_by_index(int index, bool executeFromNearest = true) {
  PathPlanner::Path path;
  if (!pathPlanner.GetPathByIndex(index, path)) {
    std::cout << "[FRC] ERROR: Invalid path index " << index << std::endl;
    pathplanner_list_paths();
    return false;
  }
  return pathplanner_execute_path(path.name, executeFromNearest);
}

// Auto-execute new paths as they arrive
static void pathplanner_check_new_path(){
  PathPlanner::Path path;
  if (pathPlanner.GetLatestPath(path)) {
    std::cout << "[FRC] ===== NEW PATH RECEIVED =====" << std::endl;
    std::cout << "[FRC] Path: \"" << path.name << "\" (" << path.waypoints.size() << " waypoints)" << std::endl;

    // Execute using smart function with nearest waypoint detection
    pathplanner_execute_path(path.name, true);
  }
}