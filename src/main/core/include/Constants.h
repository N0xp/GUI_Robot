/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

namespace constant
{
    //Motors
    static constexpr int TITAN_ID   = 42;
    static constexpr int LEFT_MOTOR = 1;    
    static constexpr int BACK_MOTOR = 2;    //BACK
    static constexpr int RIGHT_MOTOR = 0;
    static constexpr int ELEVATOR_MOTOR = 3;    //ELEVATOR

    //Encoder
    static constexpr double WHEEL_RADIUS    = 6.25; // Wheels Radius [cm]
    static constexpr double FRAME_RADIUS    = 15;  // Frame Radius [cm]
    static constexpr double PULSE_PER_REV   = 1464;
    static constexpr double GEAR_RATIO      = 1.0/1.0;
    static constexpr double ENCODER_PULSE_RATIO = PULSE_PER_REV * GEAR_RATIO;
    static constexpr double DIST_PER_TICK   =   (M_PI * 2 * WHEEL_RADIUS) / ENCODER_PULSE_RATIO;

    //Inputs
    static constexpr int START_BUTTON = 11;
    static constexpr int STOP_BUTTON  = 10;
    static constexpr int LIMIT_HIGH   =  8;
    static constexpr int LIMIT_LOW    =  9;

    static constexpr int SHARP_RIGHT  =  2;
    static constexpr int SHARP_LEFT   =  3;



    //Outputs
    static constexpr int RUNNING_LED    = 13;
    static constexpr int STOPPED_LED    = 12;
}

struct Vector3{
    double x = 0;
    double y = 0;
    double z = 0;
};

struct Twist{
    Vector3 linear;
    Vector3 angular;
};

enum GRIPPER { GRIPPER_OPEN = 0, GRIPPER_CLOSE = 40, GRIPPER_BASKET_OPEN = 20, GRIPPER_BASKET_CLOSE = 50, GRIPPER_GRAPE = 50, GRIPPER_BALL_SMALL = 45, GRIPPER_BALL_MEDIUM = 35, GRIPPER_BALL_BIG = 25};
