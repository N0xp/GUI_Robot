/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/controller/PIDController.h>

#include "frc/Timer.h"

#include "Constants.h"
#include "Functions.h"
#include "Hardware.h"
#include "Sensors.h"
#include "PID.h"

#include <cmath>
#include <string>

class Movement
{
    public:
        Movement( Hardware * h, Sensor * s ) : hardware{h}, sensor{s}{ time.Start(); }
        ~Movement(){ time.Stop(); }

        void RobotPosition();
        void InverseKinematics(double x, double y, double z);
        void ForwardKinematics( double vl, double vr, double vb );
        void SetPosition( double x, double y, double th );
        void PositionDriver( double desired_x, double desired_y, double desired_th, bool forward );
        void linear_increment( float dist, std::string direction );
        void cmd_drive( float vx, float vy, float vth );
        double WheelSpeed( int encoder, double time );     // Returns the wheel speed based on the Encoder Ticks
        void ShuffleBoardUpdate();
        double get_x();
        double get_y();
        double get_th();

        void angular_align();
        
        void line_align(std::string direction);

        void UpdateWheelsSpeed();

        double desired_back_speed; 
        double desired_left_speed; 
        double desired_right_speed;

    private:

        Hardware * hardware;
        Sensor * sensor;

        double x_global  = 0;   //Robot Global Position on the X  axis  [cm]
        double y_global  = 0;   //Robot Global Position on the Y  axis  [cm]
        double th_global = 0;   //Robot Global Position on the Th axis  [degrees]

        double current_time = 0;
        double previous_time = 0;
        double delta_time = 0;

        int current_enc_l ;
        int previous_enc_l;
 
        int current_enc_r ;
        int previous_enc_r;

        int current_enc_b ;
        int previous_enc_b;

        const double max_motor_speed = 80.0;

        const double linear_tolerance  = 3.0;    // [cm]
        const double angular_tolerance = 2.0;    // [degrees]

        const double linear_slowdown_dist = 10;  // cm
        const double max_linear_speed = 35.0;    // cm/s
        const double min_linear_speed =  7.5;    // cm/s

        const double angular_slowdown_dist = 10; // degrees
        const double max_ang_speed = 1.5;        // rad/s
        const double min_ang_speed = 0.2;        // rad/s

        double leftVelocity;
        double rightVelocity;
        double backVelocity; 

        double desired_vx;
        double desired_vy;
        double desired_vth;

        double vx;
        double vy; 
        double vth;

        double offset_th;

        const double kP = 0.8;
        const double kI = 0.05;
        const double kD = 0.0;

        PID pid_l;
        PID pid_r;
        PID pid_b;

        // frc2::PIDController p_l{kP, kI, kD};
        // frc2::PIDController p_r{kP, kI, kD};
        // frc2::PIDController p_b{kP, kI, kD};

        frc::Timer time;

};

