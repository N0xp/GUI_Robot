/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#include "Functions.h"

void delay( float time ){   // Delay in milliseconds
    frc::Wait( time / 1000.0 );
}

void coord_rotation( double &x, double &y, double ang ){

    double ang_radius = ang * ( M_PI / 180.0 );

    double x_ = x * cos( ang_radius ) - y * sin( ang_radius );
    double y_ = x * sin( ang_radius ) + y * cos( ang_radius );

    x = x_;
    y = y_;
}

double Quotient_Remainder( double x, double y ){
  double Quotient = floor( x / y );
  double Remainder = x - (y * Quotient);

  return Remainder;
}

double close_angle( double ang ){
    
    if      ( ang < -180 ) { ang = ang + 360; }
    else if ( ang >  180 ) { ang = ang - 360; }

    return ang;
}

double sharp_function_left( double reading ){
    return std::pow(reading, -1.2045) * 27.726;
}
double sharp_function_right( double reading ){
    return std::pow(reading, -1.2045) * 27.726;
}
double sharp_function_arm( double reading ){
    return std::pow(reading, -1.2045) * 27.726;
}

float straight_ang( float angle ){
    double angle_wall = 0;
    const float split = 90;

    double minDiff = 0;
    for (int i = 0; i < (360.0 / split) + 1; i++){
        double diff = std::abs( close_angle( angle - (i * split)) );
        if (i == 0 || ( diff < minDiff)){
            angle_wall = i * split;
            minDiff = diff;
        }
    }
    return angle_wall;
}