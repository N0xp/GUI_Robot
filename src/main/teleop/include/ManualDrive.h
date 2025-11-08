/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#include "Hardware.h"
#include "Movement.h"
#include "OI.h"

#include <iostream>


class Drive
{
    public:
        Drive( Hardware * h, Movement * m, OI * o ) : hardware{h}, move{m}, oi{o}{}
        void Execute();
    
    private:
        Movement * move;
        Hardware * hardware;
        OI* oi;

        double inputLeftY = 0;
        double inputLeftX = 0;
        double inputRightY = 0;
        double inputRightX = 0;

        double prev_vx = 0;
        double prev_vy = 0;
        double prev_vth = 0;
        
        static constexpr double RAMP_UP = 0.05;

        static constexpr double RAMP_DOWN = 0.05;

        static constexpr double DELTA_LIMIT = 0.075;

};