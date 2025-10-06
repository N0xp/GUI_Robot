/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#include "ManualDrive.h"

void Drive::Execute()
{
    /**
     * Get Joystick Data
     */
    inputLeftY  = oi->GetLeftDriveY();
    inputLeftX  = oi->GetLeftDriveX();
    inputRightY = oi->GetRightDriveY();
    inputRightX = oi->GetRightDriveX();
    
    double vx = 0;
    double vy = 0;
    double vth = 0;

    double xy_speed = 40;   // [cm/s]
    double th_speed = 20;   // [degrees/s]

    if(inputLeftX >= DELTA_LIMIT){
        vx = xy_speed;}
    else if (inputLeftX <= -DELTA_LIMIT){
        vx = -xy_speed;}
    if(inputLeftY >= DELTA_LIMIT){
        vy = xy_speed;}
    else if (inputLeftY <= -DELTA_LIMIT){
        vy = -xy_speed;}
    if(inputRightX >= DELTA_LIMIT){
        vth = th_speed;}
    else if (inputRightX <= -DELTA_LIMIT){
        vth = -th_speed;}

    if ( (vx - prev_vx) > 0 )       { vx  = prev_vx  + RAMP_UP;   }
    else if ( (vx - prev_vx) < 0 )  { vx  = prev_vx  - RAMP_DOWN; }
    if ( (vy - prev_vy) > 0 )       { vy  = prev_vy  + RAMP_UP;   }
    else if ( (vy - prev_vy) < 0 )  { vy  = prev_vy  - RAMP_DOWN; }
    if ( (vth - prev_vth) > 0 )     { vth = prev_vth + RAMP_UP;   }
    else if ( (vth - prev_vth) < 0 ){ vth = prev_vth - RAMP_DOWN; }

    std::clamp( vx,  xy_speed, -xy_speed );
    std::clamp( vy,  xy_speed, -xy_speed );
    std::clamp( vth, th_speed, -th_speed );

    move->InverseKinematics(vx, vy, vth);       // Base on the desired vx, vy and vth calculates the PWM to be applied to each wheel

    hardware->SetLeft ( move->desired_left_speed );
    hardware->SetBack ( move->desired_back_speed );
    hardware->SetRight( move->desired_right_speed );

    prev_vx = vx;
    prev_vy = vy;
    prev_vth = vth;

    std::cout << "vx: " << vx << " vy: " << vy << " vth: " << vth << std::endl;
}   
