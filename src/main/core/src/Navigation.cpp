/************************************
 *  Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * Release date: 12/08/2024
 * 
 * Modified by: Felipe Ferreira
 * Last modification date: 
 * New version:
*************************************/

#include "Robot.h"

#include "Constants.h"

#include "Hardware.h"
#include "Movement.h"
#include "Sensors.h"
#include "lidar.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#define follow_angle       -25
#define front_angle         90
#define split               32
#define offset_dist_to_wall 300
#define dist_to_offset      150

#define max_vel_theta   90   // [ degrees / s ]
#define max_dist_theta  30   // [ degrees ]
#define max_move_theta  25   // [ degrees ]
#define max_vel_x       200  // [ mm/s ]

#define max_angle 270
#define min_angle 90

#define laser_to_center_x 160
#define laser_to_center_y 0


// Lidar data container
static std::vector<std::vector<int>> lidar_point(360, std::vector<int>(2));

// Simulated scan callback
void scanCallback( ) {

    studica::Lidar::ScanData scan = lidar.getScan();

    int range = 360;
    double angle_increment = 1;

    for (int i = 0; i < range; i++) {
        float angle = -scan.angle[i]; //std::round(((i * angle_increment) + msg.angle_min) * (180.0 / M_PI));
        angle = (angle < 0) ? (360 + angle) : angle;
        angle -= front_angle;
        angle = (angle < 0) ? (360 + angle) : angle;

        if ((angle < min_angle || angle > max_angle) && scan.distance[i] != -1 ) {
            float x = scan.distance[i] * cos(angle * (M_PI / 180.0));
            float y = scan.distance[i] * sin(angle * (M_PI / 180.0));

            // x += laser_to_center_x;
            // y += laser_to_center_y;

            float angle_center = atan2(y, x) * (180.0 / M_PI);
            float dist_center = std::sqrt(x * x + y * y);

            angle_center = (angle_center < 0) ? (360 + angle_center) : angle_center;

            // lidar_point[i][0] = angle_center;
            // lidar_point[i][1] = dist_center;
            lidar_point[i][0] = angle;
            lidar_point[i][1] = scan.distance[i];

        }else{
            lidar_point[i][0] = angle;
            lidar_point[i][1] = 0;
        }
    }
}

int Navigation() {

    // Start MockDS
    Robot r;
    r.ds.Enable();

    cam.StartCamera();
    lidar.StartLidar();

    int img_h = 480;
    int img_w = 640;


    cs::CvSource scanStream = frc::CameraServer::GetInstance()->PutVideo("Frame", img_w, img_h);
    frc::Shuffleboard::GetTab("MainData").Add("CameraProcess", scanStream).WithPosition(0, 0).WithSize(10, 5);

    while (true) {
        scanCallback();

        std::vector<std::vector<float>> points(360, std::vector<float>(2));
        for (int i = 0; i < lidar_point.size(); i++) {
            double dist = lidar_point[i][1];
            points[i][0] = dist * cos(lidar_point[i][0] * (M_PI / 180.0));
            points[i][1] = dist * sin(lidar_point[i][0] * (M_PI / 180.0));
        }

        std::vector<int> points_per_quadrant(split, 0);
        std::vector<float> quadrant_length(split, 0.0f);

        for (int i = 0; i < lidar_point.size(); i++) {
            const int number_of_points = 3;
            bool greatest_point = false;

            for (int j = 0; j < number_of_points && (i + j + 1) < lidar_point.size(); j++) {
                double mag = std::sqrt(std::pow(points[i][0] - points[i + j + 1][0], 2.0) + std::pow(points[i][1] - points[i + j + 1][1], 2.0));

                if (mag > (100 * (j + 1))) {
                    greatest_point = true;
                } else {
                    greatest_point = false;
                    break;
                }
            }

            if (greatest_point || lidar_point[i][1] < 50) {
                points[i][0] = -1;
                points[i][1] = -1;
            } else {
                float quadrant = lidar_point[i][0] + ((360.0 / split) / 2.0);
                quadrant = (quadrant < 0) ? (360 + quadrant) : quadrant;
                quadrant = int(quadrant / (360.0 / split));

                if (quadrant < 0)
                    quadrant += split;
                else if (quadrant >= split)
                    quadrant -= split;

                quadrant = std::abs(quadrant);

                points_per_quadrant[quadrant]++;
                quadrant_length[quadrant] += lidar_point[i][1];
            }
        }

        std::vector<float> move_vector(2, 0.0f);
        std::vector<float> quadrant_dist(split, 0.0f);

        for (int i = 0; i < split; i++) {
            if (points_per_quadrant[i] != 0) {
                quadrant_dist[i] = quadrant_length[i] / points_per_quadrant[i];
                float split_angle = i * (360.0f / split);
                float split_angle_rad = split_angle * M_PI / 180.0;

                float ideal_split_angle = split_angle_rad - (M_PI / 2.0) * round(split_angle_rad / (M_PI / 2.0));
                float ideal_split_dist = offset_dist_to_wall;

                if (ideal_split_angle != 0) {
                    if (ideal_split_angle > M_PI / 4.0)
                        ideal_split_dist = std::abs(offset_dist_to_wall / sin(ideal_split_angle));
                    else
                        ideal_split_dist = std::abs(offset_dist_to_wall / cos(ideal_split_angle));
                }

                float weight_offset = std::abs(quadrant_dist[i] - ideal_split_dist);
                weight_offset = (weight_offset > dist_to_offset) ? dist_to_offset : weight_offset;
                weight_offset /= dist_to_offset;

                float angle_diff = std::abs(follow_angle - split_angle);
                float weight_angle = 1 - std::abs(((angle_diff > 180) ? (angle_diff - 360) : (angle_diff < -180) ? (360 + angle_diff) : angle_diff) / 180.0);

                float vector_angle = follow_angle - (180.0 * floor(follow_angle / 180.0));
                vector_angle = (vector_angle < 90) ? -M_PI / 2.0 : M_PI / 2.0;

                if ((quadrant_dist[i] - ideal_split_dist) > 0)
                    vector_angle = split_angle_rad;
                else
                    vector_angle += split_angle_rad;

                float angle_weight = weight_angle * weight_offset;
                move_vector[0] += angle_weight * cos(vector_angle);
                move_vector[1] += angle_weight * sin(vector_angle);
            }
        }

        float move_angle = atan2(move_vector[1], move_vector[0]) * 180.0 / M_PI;
        move_angle = (move_angle > 180) ? (move_angle - 360) : (move_angle < -180) ? (360 + move_angle) : move_angle;

        float move_linear = (max_vel_theta * (M_PI / 180.0)) * max_vel_x;

        if (std::abs(move_angle) > max_move_theta)
            move_linear = 0;

        move_angle /= max_dist_theta;
        move_angle = std::max(std::min(move_angle, 1.0f), -1.0f);
        move_angle *= max_vel_theta;

        Twist cmd;
        cmd.linear.x = move_linear / 10.0f;
        cmd.linear.y = 0.0f;
        cmd.angular.z = move_angle * (M_PI / 180.0f);

        movement.cmd_drive( cmd.linear.x, cmd.linear.y, cmd.angular.z );

        std::cout << "vx: " << cmd.linear.x << " vy: " << cmd.linear.y << " vh: " << cmd.angular.z << std::endl;


        Mat frame(img_h, img_w, CV_8UC3, cv::Scalar( 255,255,255 ));


        for( int i = 0; i < 360; i++ ){
            int env_h = 4000;        // Environment Height [mm]
            int env_w = 4000;        // Environment Width [mm]

            float x = lidar_point[i][1] * cos(lidar_point[i][0] * (M_PI / 180.0));
            float y = lidar_point[i][1] * sin(lidar_point[i][0] * (M_PI / 180.0));   

            double mm_to_pixel_h = ( double(img_h) / double(env_h) );
            double mm_to_pixel_w = ( double(img_w) / double(env_w) );

            x = (img_w / 2) + ( x * mm_to_pixel_w );
            y = (img_h / 2) + ( y * mm_to_pixel_h );         
            
            cv::Point point_to_draw( x, y); 

            cv::circle(frame, point_to_draw, 3, cv::Scalar(255,0,0), -1);

            // std::cout << "x: " << x << " y: " << y << std::endl;

        }

        scanStream.PutFrame(frame);

        
        frc::SmartDashboard::PutNumber("vel_x",  cmd.linear.x );
        frc::SmartDashboard::PutNumber("vel_y",  cmd.linear.y );
        frc::SmartDashboard::PutNumber("vel_z",  cmd.angular.z );

        delay(20);
    }
    return 0;
}
