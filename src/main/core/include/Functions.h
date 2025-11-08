/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#include "frc/Timer.h"

void delay( float time );   // Delay in milliseconds
void coord_rotation( double &x, double &y, double ang );
double Quotient_Remainder( double x, double y );
double sharp_function_left( double reading );
double sharp_function_right( double reading );
double sharp_function_arm( double reading );

double close_angle( double ang );
float straight_ang( float angle );