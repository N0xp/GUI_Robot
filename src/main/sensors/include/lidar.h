#pragma once

#include "studica/Lidar.h"

#include <string.h>
#include <iostream>
#include "math.h"

#include "Functions.h"
#include "Movement.h"
#include "Sensors.h"

#include <frc/smartdashboard/SmartDashboard.h>

class Lidar
{
    public:
        Lidar( Movement * m, Sensor * s ) : move{m}, sensor{s}{}
        void Periodic();
        void StartLidar();
        void StopLidar();
        float lidar_mean( double & sensor_dist, int & scan_ang );
        void linear_align( float dist, std::string direction );

        double GetLidarFront( );
        double GetLidarLeft( );
        double GetLidarRight( );

        studica::Lidar::ScanData getScan();


    private:
        Movement * move;
        Sensor * sensor;
        /**
         * kUSB1 = Top USB 2.0 port of VMX
         * kUSB2 = Bottom USB 2.0 port of VMX
         */ 
        studica::Lidar lidar{studica::Lidar::Port::kUSB1};
        // Scan data struct
        studica::Lidar::ScanData scanData;
        // Flag to prevent unessary updating of dashboard when not scanning
        bool lidarRunning = true;

        double right_scan = 0, left_scan = 0, front_scan = 0;

        int left_ang   = 180;
        int right_ang  = 360;
        int front_ang  = 270;

        int left_ang_  = 180;
        int right_ang_ = 360;
        int front_ang_ = 270;

};