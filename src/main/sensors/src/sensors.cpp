#include "Sensors.h"

void Sensor::Periodic(){
    sharp_right_dist = hardware->GetRightSharp();
    sharp_left_dist  = hardware->GetLeftSharp();

    cobra_l  = hardware->GetCobra(0) > 2.5;
    cobra_r  = hardware->GetCobra(3) > 2.5;
    cobra_cl = hardware->GetCobra(1) > 2.5;
    cobra_cr = hardware->GetCobra(2) > 2.5;

    frc::SmartDashboard::PutNumber("sharp_right_dist", sharp_right_dist );
    frc::SmartDashboard::PutNumber("sharp_left_dist", sharp_left_dist );
    frc::SmartDashboard::PutNumber("cobra_l",  hardware->GetCobra(0) );
    frc::SmartDashboard::PutNumber("cobra_r",  hardware->GetCobra(3) );
    frc::SmartDashboard::PutNumber("cobra_cl", hardware->GetCobra(1) );
    frc::SmartDashboard::PutNumber("cobra_cr", hardware->GetCobra(2) ); 

}

float Sensor::straight_ang( float angle ){
    double angle_wall = 0;
    const float split = 45;

    double minDiff = 0;
    for (int i = 0; i < (360.0 / split) + 1; i++){
        double diff = std::abs(angle - (i * split));
        if (i == 0 || ( diff < minDiff)){
            angle_wall = i * split;
            minDiff = diff;
        }
    }
    return angle_wall;
}

double Sensor::GetRightSharp(){
    return sensor_mean( sharp_right_dist, 10 );
}

double Sensor::GetLeftSharp(){
    return sensor_mean( sharp_left_dist, 10 );
}

float Sensor::sensor_mean( double & sensor_dist, int samples ){
    int count = 0;

    float average = 0;
    float prev_dist = 0;

    while ( count < samples ){
        Periodic();

        float dist = sensor_dist;

        if( prev_dist != dist ){
            average += ( dist / samples);
            count++;
        }
        delay(50);
    }

    return average;
}

float Sensor::get_angle_wall( int sample ){
    Periodic();

    float baseline_distance = 14;

    float sensor_difference = sensor_mean(sharp_right_dist, sample) - sensor_mean(sharp_left_dist, sample);
    float angle_diff = atan(sensor_difference / baseline_distance);
    angle_diff = angle_diff * (180.0 / M_PI);

    return angle_diff;
}

float Sensor::setAngle( float angle ){
    Periodic();

    double angle_diff = get_angle_wall( 10 );

    double angle_wall = 0;

    angle_wall = straight_ang( angle );

    double minDiff = 0;
    for (int i = 0; i < 5; i++){
        double diff = abs( angle - (i * 90));
        if (i == 0 || ( diff < minDiff)){
            angle_wall = i * 90;
            minDiff = diff;
        }
    }
    float estimated_angle_degrees = angle_wall - angle_diff;
    estimated_angle_degrees = Quotient_Remainder( estimated_angle_degrees, 360 );

    return estimated_angle_degrees;
}

