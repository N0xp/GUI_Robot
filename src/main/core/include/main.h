/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#include <string>
#include <iostream>
#include <vector>

#include <frc/TimedRobot.h>
#include <studica/MockDS.h>
#include <frc/smartdashboard/SmartDashboard.h>


int Inspection();
int SimpleTask();
int Navigation();
int MainTask();

const float offset_front = 15;
const float offset_side = 0;

static float robot_x, robot_y, robot_th;

static bool STOP = false;