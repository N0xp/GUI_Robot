#include "lidar.h"

#define DEBUG true

void Lidar::StartLidar()
{
    
    lidar.Start();
    lidarRunning = true;
    lidar.ClusterConfig(50.0f, 5);
    lidar.EnableFilter(studica::Lidar::Filter::kCLUSTER, true);

    lidar_mean( right_scan, right_ang );
    lidar_mean( left_scan, left_ang );
    lidar_mean( front_scan, front_ang );

    for( int i = 0; i < 360; i++){
        std::cout << "angle: " << i << " dist: " <<  scanData.distance[i] << std::endl;
    }

}

void Lidar::StopLidar()
{
    lidar.Stop();
    lidarRunning = false;
}

void Lidar::Periodic()
{
    if (lidarRunning)
        scanData = lidar.GetData(); // Update scanData struct

    #if DEBUG //Print out sensor info

        frc::SmartDashboard::PutNumber("Angle Left",        left_ang   );
        frc::SmartDashboard::PutNumber("Distance Left",     left_scan  );
        frc::SmartDashboard::PutNumber("Angle Right",       right_ang  );
        frc::SmartDashboard::PutNumber("Distance Right",    right_scan );
        frc::SmartDashboard::PutNumber("Angle Front",       front_ang  );
        frc::SmartDashboard::PutNumber("Distance Front",    front_scan );

    #endif

    left_scan  = scanData.distance[left_ang]  / 1000.0;
    right_scan = scanData.distance[right_ang] / 1000.0;
    front_scan = scanData.distance[front_ang] / 1000.0;
}



void Lidar::linear_align( float dist, std::string direction ){

    frc::SmartDashboard::PutString("Process",  "Linear Align" );

    std::cout << "Starting Sensor Alignment " << direction << std::endl;
    
    float sensor_dist = 0;
    int count = 0;
    int count_zero = 0;

    if (dist < 10) {
        dist = 12;
    }

    dist = dist / 100.0; // to meters

    float prev_speed = 0;

    
    int des_ang = sensor->straight_ang( move->get_th() );


    if(       direction.compare( "front" ) == 0 ){
        front_ang = front_ang_;
        do{
            Periodic();
            sensor_dist = (lidar_mean( front_scan,  front_ang ));
            // ROS_INFO( "sensor_dist: %f", sensor_dist );
            if( sensor_dist == -1 ){
                move->linear_increment( 5, "back" );
            }
        }while( sensor_dist == -1 );
    }else if( direction.compare( "left" )  == 0 ){
        left_ang = left_ang_;
        do{  
            Periodic();
            sensor_dist = (lidar_mean( left_scan,   left_ang  ));
            if( sensor_dist == -1 ){
                move->linear_increment( 5, "right" );
            }
        }while( sensor_dist == -1 );
    }else if( direction.compare( "right" ) == 0 ){
        right_ang = right_ang_;
        do{
            Periodic();
            sensor_dist = (lidar_mean( right_scan,  right_ang ));
            if( sensor_dist == -1 ){
                move->linear_increment( 5, "left" );
            }
        }while( sensor_dist == -1 );
    }

    Twist cmd;

    while( count < 5 ){

        Periodic();

        if(       direction.compare( "front" ) == 0 ){
            sensor_dist = front_scan;
        }else if( direction.compare( "left" )  == 0 ){
            sensor_dist = left_scan;
        }else if( direction.compare( "right" ) == 0 ){
            sensor_dist = right_scan;
        }

        double linear_dist_offset = 0.05;  // [m]
        double max_linear_speed   = 15.0;  // [cm/s]
        double min_linear_speed   =  5.0;  // [cm/s]
        float linear_tolerance    = 0.03;  // [m]

        double desired_v = (-(dist - sensor_dist) / linear_dist_offset) * max_linear_speed;
        desired_v =  std::max( std::min( desired_v, max_linear_speed ), -1 * max_linear_speed );
        if( abs(desired_v) < min_linear_speed ){
          desired_v > 0 ? desired_v = min_linear_speed : desired_v = -min_linear_speed; }

        if( abs((dist - sensor_dist)) < linear_tolerance ){ count++; }
        else if( sensor_dist > 0 ){ count = 0; }

        if( abs((dist - sensor_dist)) < linear_tolerance / 2.0 && sensor_dist != 0 ){ desired_v = 0; }

        if( sensor_dist <= 0 ){
            desired_v = prev_speed;
            count_zero++;
        }else{
            count_zero = 0;
        }

        if( count_zero > 5 ){
            desired_v = -min_linear_speed;
        }

        if(       direction.compare( "front" ) == 0 ){
            cmd.linear.x = desired_v;
            cmd.linear.y  = 0;
        }else if( direction.compare( "left" )  == 0 ){
            cmd.linear.x = 0;
            cmd.linear.y  = desired_v;
        }else if( direction.compare( "right" ) == 0 ){
            cmd.linear.x = 0;
            cmd.linear.y  = -desired_v;
        }

        float th_diff = des_ang - move->get_th();

        if      ( th_diff < -180 ) { th_diff = th_diff + 360; }
        else if ( th_diff >  180 ) { th_diff = th_diff - 360; }

        double max_ang_speed = 0.75;         // rad/s
        double min_ang_speed = 0.25;         // rad/s
        double angular_dist_offset = 10.0;   // [degrees]
        double angular_tolerance = 1.5;

        double desired_vth = (th_diff / angular_dist_offset) * max_ang_speed; 
        desired_vth =  std::max(  std::min( desired_vth, max_ang_speed ), -1 * max_ang_speed );
        if( abs(desired_vth) < min_ang_speed ){
          desired_vth > 0 ? desired_vth = min_ang_speed : desired_vth = -min_ang_speed; }
        if( abs(th_diff) < angular_tolerance ){ desired_vth = 0; }

        cmd.angular.z  = desired_vth;

        // std::cout << "vx: " << cmd.linear.x << " vy: " << cmd.linear.y << " vth: " << cmd.angular.z << " dist: " << sensor_dist << " th_diff: " << th_diff << std::endl;
        std::cout << "des_ang: " << des_ang << " get_th(): " << move->get_th() << std::endl;

        move->cmd_drive( cmd.linear.x, cmd.linear.y, cmd.angular.z );

        prev_speed = desired_vth;

        delay(20);
    }

    move->cmd_drive(0,0,0);
    delay(250);

}

float Lidar::lidar_mean( double & sensor_dist, int & scan_ang ){
   
    const int n_samples = 5;

    float dist = 0;

    int count = 0;
    int count_n = 0;
    int count_out = 0;

    float average = 0;
    // float prev_dist = sensor_dist;
    
    while ( count < n_samples && count_out < 30 ){

        Periodic();

        dist = sensor_dist;

        if( dist > 0 && dist < 5 ){ //prev_dist != dist && dist != 0 && dist < 5 ){
            average += ( dist / n_samples);
            count++;
            // prev_dist = dist;
            count_n = 0;
            count_out = 0;
        }else{
            count_n++;
            count_out++;
        }

        if( count_n > 3 ){
            if( scan_ang > 270 ){
                scan_ang = scan_ang - 1;
            }else if(scan_ang < 270){
                scan_ang = scan_ang + 1;
            }else{
                scan_ang = scan_ang + 10;
            }
            count_n = 0;
            count = 0;
            average = 0;
        }

        delay( 100 );
    }
    dist = average;

    if( count_out >= 30 ){
        dist = -1;
    }

    std::cout << "Lidar mean is " << dist << std::endl;

    return dist;
}
double Lidar::GetLidarFront( ){
    front_ang = front_ang_;
    return lidar_mean( front_scan, front_ang );
}
double Lidar::GetLidarLeft( ){
    left_ang = left_ang_;
    return lidar_mean( left_scan, left_ang );
}
double Lidar::GetLidarRight( ){
    right_ang = right_ang_;
    return lidar_mean( right_scan, right_ang );
}