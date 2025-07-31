#pragma once

#include "Hardware.h"
#include "Functions.h"

#include <math.h>

class Sensor
{
    public:
        Sensor( Hardware * h ) : hardware{h}{  }
        ~Sensor(){ }

        void Periodic();

        double GetRightSharp();
        double GetLeftSharp();

        float straight_ang( float angle );
        float sensor_mean( double & sensor_dist, int samples );

        float get_angle_wall( int sample );
        float setAngle( float angle);

        double cobra_l  = 0;
        double cobra_r  = 0;
        double cobra_cl = 0;
        double cobra_cr = 0;

    private:
        Hardware * hardware;

        double sharp_right_dist = 0;
        double sharp_left_dist = 0;



};
