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

static void position_driver(float x, float y, float th, bool forward){
  movement.PositionDriver( x, y, th, forward );
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

static void DetectFruit( std::vector<std::string> fruits ){
  bool debug = false;
  double ang = straight_ang( movement.get_th() );
  cam.DetectFruit( fruits, ang, debug );
}