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
        Oms( Hardware * h ) : hardware{h}{}

        void oms_driver( double desired_height );
        void reset( int direction );
        void set_gripper( int ang );
        void set_base( int ang );


    private:

        Hardware * hardware;

        float height = 1000;         // OMS height [cm]

        float low_height = 23.5;     // [cm]
        float high_height = 71;      // [cm]

        float tolerance  = 0.5;     // [cm]

        bool limit_high_state;
        bool limit_low_state;
        bool limit_base_state;

        int elevator_enc;

        const float pinionRadius = 1.25;   // Pinion's radius [cm]

        const double kP = 0.8;
        const double kI = 0.2;
        const double kD = 0;

        frc2::PIDController pid_e{kP, kI, kD};

        double prev_base_ang = 150;

};
