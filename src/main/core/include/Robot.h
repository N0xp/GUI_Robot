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

#include <dfs.h>

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

static double SL(){
  return lidar.GetLidarLeft() * 100 + offset_side;
}
static double SR(){
  return lidar.GetLidarRight() * 100 + offset_side;
}
static double SFL(){
  return sensor.GetLeftSharp() + offset_front;
}
static double SFR(){
  return sensor.GetRightSharp() + offset_front;
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

static void DetectFruit( std::vector<std::string> fruits ){
  bool debug = false;
  double ang = straight_ang( movement.get_th() );
  cam.DetectFruit( fruits, ang, debug );
}

static std::vector<std::vector<double>> best_way( Coord &d, Coord &c ){
  std::vector<Coord> path;
  std::vector<std::vector<double>> d_path;
  double dist = 0;
  std::vector<std::string> checked;
 
  dfs( &d, &c, path, 0, d_path, dist, checked );
  return d_path;
}
static void path_driver( std::vector<std::vector<double>> path ){
  for( std::vector<double> p : path ){
    movement.PositionDriver( p[0], p[1], p[2] );
    std::cout << "x: " << p[0] << " y: " << p[1] << " th: " << p[2] << std::endl;
  }
}