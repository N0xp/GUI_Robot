/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#include <string.h>
#include <iostream>
#include "math.h"

#include "Functions.h"
#include "Hardware.h"
#include "Constants.h"

#include "frc/Timer.h"

#include <frc/controller/PIDController.h>

class Oms
{
    public:
        Oms( Hardware * h ) : hardware{h}{
            prev_base_ang = hardware->base_ang;
            prev_grip_ang = hardware->grip_ang;
            prev_arm_ang  = hardware->arm_ang;
        }

        void oms_driver( double desired_height, double speed );
        void reset( int direction );
        void set_gripper( int ang );
        void set_base( int ang );
        void set_arm( int ang );

        double prev_base_ang = 0;
        double prev_arm_ang  = 0;
        double prev_grip_ang = 0;

        float height = 1000;         // OMS height [cm]
        float base = 0;

        const int base_ang_offset = 240;

    private:

        Hardware * hardware;


        float low_height = 17.5;     // [cm]
        float high_height = 40;      // [cm]

        float tolerance  = 0.5;     // [cm]

        bool limit_high_state;
        bool limit_low_state;
        bool limit_base_state;

        int elevator_enc;

        const float pinionRadius = 1.25;   // Pinion's radius [cm]

        const double kP = 0.9;
        const double kI = 0.3;
        const double kD = 0;

        frc2::PIDController pid_e{kP, kI, kD};




};
