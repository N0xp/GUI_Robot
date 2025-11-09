/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#include "Movement.h"
#include "Robot.h"

void Movement::PositionDriver( double desired_x, double desired_y, double desired_th ) {


    const float period = 20;  // ms

    bool reach_linear_tol  = false;

    frc::SmartDashboard::PutString("Process",  "Position Driver" );


    std::cout << "Move Goal x: " << desired_x << " y: " << desired_y << " th: " << desired_th << std::endl;

    previous_enc_l = hardware->GetLeftEncoder();
    previous_enc_r = hardware->GetRightEncoder();

    pid_l.Reset();
    pid_r.Reset();

    bool forward = true;
    static int update_counter = 0;  // For periodic GUI updates

    while(true){
        
        RobotPosition();    // Calculates Robot Position based on the wheels speed and displacement

        // Send position to GUI every 10 iterations (~200ms)
        update_counter++;
        if (update_counter % 10 == 0) {
            pathplanner_update_odometry(false);
        }

        double desired_position[3] = { desired_x, desired_y, desired_th };   // [cm], [cm], [degrees]
        double current_position[3] = { x_global , y_global, th_global };     // [cm], [cm], [degrees]

        double x_diff  = desired_position[0] - current_position[0];
        double y_diff  = desired_position[1] - current_position[1];
        double th_diff = desired_position[2] - current_position[2];

        float move_vector_ang = atan2( y_diff, x_diff ); // [rad]
        move_vector_ang = (move_vector_ang / M_PI) * 180;         // [degrees]

        float move_vector_magnitude = sqrt( pow( x_diff, 2 ) + pow( y_diff, 2 ) ); // [cm]

        double des_ang = desired_position[2];
        if( desired_position[2] == -1 ){ des_ang = current_position[2]; }

        float forward_ang  = close_angle( des_ang - move_vector_ang );
        float backward_ang = close_angle( des_ang - move_vector_ang + 180 );


        // Check if going backward is better than going forward. A offset of 20% was add to make sure the priority is forward,
        // So in case forward and backward are close it is going to select forward
        if( abs(forward_ang) > (abs(backward_ang) * 1.2) ){
            forward = false;
        }

        if( !forward ){
            move_vector_magnitude = move_vector_magnitude * -1;
            move_vector_ang = move_vector_ang + 180;
        }


        //When the robot reaches the linear goal tries to reach the desired angle
        if ( abs(move_vector_magnitude) < linear_tolerance || reach_linear_tol ){
          reach_linear_tol = true;
          move_vector_magnitude = 0;
          if ( desired_position[2] == -1 ){ th_diff = 0; }
        }
        else{
          th_diff = move_vector_ang - current_position[2];
        }

        if      ( th_diff < -180 ) { th_diff = th_diff + 360; }
        else if ( th_diff >  180 ) { th_diff = th_diff - 360; }


        double setPoint_angular = (abs(th_diff) / angular_slowdown_dist) * max_ang_speed;               // [rad/s]
        setPoint_angular = std::max( std::min( setPoint_angular, max_ang_speed ), min_ang_speed );
        if( th_diff < 0 ){ setPoint_angular = setPoint_angular * -1; } 

        double setPoint_linear  = (abs(move_vector_magnitude) / linear_slowdown_dist ) * max_linear_speed;     // [cm/s]
        setPoint_linear  = std::max( std::min( setPoint_linear, max_linear_speed ), min_linear_speed );
        if( move_vector_magnitude < 0 ){ setPoint_linear = setPoint_linear * -1; } 


        if ( abs(th_diff) > 10 || move_vector_magnitude == 0 ){ setPoint_linear = 0; }
        if ( abs(move_vector_magnitude) < linear_tolerance && abs(th_diff) < angular_tolerance ) { setPoint_linear  = 0; setPoint_angular = 0; }


        // coord_rotation( desired_vx, desired_vy, -th_global );   // From Global to Local

        cmd_drive( setPoint_linear, 0, setPoint_angular );

        if( setPoint_linear == 0 && setPoint_angular == 0 && leftVelocity == 0 && rightVelocity == 0 && backVelocity == 0 )
            { break; }

        delay( period ); 

    }

    hardware->SetLeft ( 0 );
    hardware->SetRight( 0 );

    pid_l.Reset();
    pid_r.Reset();

    delay(250);

    frc::SmartDashboard::PutNumber("leftVelocity",  -1  );
    frc::SmartDashboard::PutNumber("rightVelocity", -1 );

    frc::SmartDashboard::PutNumber("delta_enc_l", -1 );
    frc::SmartDashboard::PutNumber("delta_enc_r", -1 );

    frc::SmartDashboard::PutNumber("vl", -1 );
    frc::SmartDashboard::PutNumber("vr", -1 );

}

void Movement::RobotPosition(){
    //Forward Kinematics
    ForwardKinematics( leftVelocity, rightVelocity, backVelocity );

    coord_rotation( vx, vy, th_global );   // From Local to Global

    //Robot Displacement
    double delta_x  = vx  * delta_time;
    double delta_y  = vy  * delta_time;
    double delta_th = vth * delta_time;

    //Robot Position update
    x_global  = x_global  + delta_x;
    y_global  = y_global  + delta_y;
    // th_global = th_global + ((delta_th / M_PI) * 180);   // Angle based on the encoder

    th_global = -hardware->GetYaw() - offset_th;            // Angle based on the Gyro

    if      ( th_global <  0  ) { th_global = th_global + 360; }
    else if ( th_global > 360 ) { th_global = th_global - 360; }
}

void Movement::cmd_drive( float x, float y, float th ){

    pid_l.setPID(0.6, 0.3, 0.0);
    pid_l.setPIDLimits(-0.7, 0.7);

    pid_r.setPID(0.6, 0.3, 0.0);
    pid_r.setPIDLimits(-0.7, 0.7);

    desired_vx = x;
    desired_vy = y;
    desired_vth = th;

    UpdateWheelsSpeed();              

    InverseKinematics( x, y, th );

    double vl = (pid_l.Calculate(leftVelocity  / 100.0, (desired_left_speed  * max_motor_speed) / 100.0) * 100 )/ max_motor_speed;
    double vr = (pid_r.Calculate(rightVelocity / 100.0, (desired_right_speed * max_motor_speed) / 100.0) * 100 )/ max_motor_speed;
    
    frc::SmartDashboard::PutNumber("vl", vl );
    frc::SmartDashboard::PutNumber("vr", vr );



    if( hardware->GetStopButton() ){  // Stop the Motors when the Stop Button is pressed
        hardware->SetLeft ( 0 );
        hardware->SetRight( 0 );
        pid_l.Reset();
        pid_r.Reset();
        hardware->StopActuators();
    }else{
        if( desired_left_speed == 0 ){ hardware->SetLeft ( 0 );  pid_l.Reset();
        }else{ hardware->SetLeft ( std::clamp(vl, -1.0, 1.0) ); }
        if( desired_right_speed == 0 ){ hardware->SetRight( 0 ); pid_r.Reset();
        }else{ hardware->SetRight( std::clamp(vr, -1.0, 1.0) ); }
        hardware->ReactivateActuators();
    }

    ShuffleBoardUpdate();
}

void Movement::linear_increment( float dist, std::string direction ){

    double ang = 0;

    frc::SmartDashboard::PutString("Process",  "Linear Increment" );


    if     ( direction.compare( "front" ) == 0 ){ ang =   0; }
    else if( direction.compare( "left"  ) == 0 ){ ang =  90; }
    else if( direction.compare( "back"  ) == 0 ){ ang = 180; }
    else if( direction.compare( "right" ) == 0 ){ ang = -90; }

    float dx = get_x() + ( dist * std::cos( (ang + get_th()) * ( M_PI / 180.0 ) ));
    float dy = get_y() + ( dist * std::sin( (ang + get_th()) * ( M_PI / 180.0 ) ));

    std::cout << "Linear Increment Goal x: " << dx << " y: " << dy << std::endl;

    PositionDriver( dx, dy, get_th() );

}

void Movement::InverseKinematics(double x, double y, double z){
    
    desired_right_speed = ((2.0 * x) + (z * constant::FRAME_RADIUS * 2.0)) / 2.0;   // [cm/s]
    desired_left_speed  = ((2.0 * x) - (z * constant::FRAME_RADIUS * 2.0)) / 2.0;   // [cm/s]

    desired_left_speed  = desired_left_speed  / max_motor_speed;   // cm/s to PWM [0-1]    
    desired_right_speed = desired_right_speed / max_motor_speed;   // cm/s to PWM [0-1]
   
}

void Movement::ForwardKinematics( double vl, double vr, double vb ){

    vx = (( vr + vl ) / 2);  // [cm/s]
    vy = 0;
    vth = (( vr - vl ) / constant::FRAME_RADIUS);             // [rad/s]

}

double Movement::WheelSpeed( int encoder, double time ){
        double speed  = ((2 * M_PI * constant::WHEEL_RADIUS * encoder) / (constant::ENCODER_PULSE_RATIO * time));   // [cm/s]

        if ( time == 0 ) { speed  = 0; }

        return speed;
}

void Movement::SetPosition( double x, double y, double th ){
  x_global  = x;
  y_global  = y;
  offset_th = -hardware->GetYaw() - th;
  th_global = th;

  ShuffleBoardUpdate();
}

void Movement::ShuffleBoardUpdate(){

    th_global = -hardware->GetYaw() - offset_th;            // Angle based on the Gyro

    if      ( th_global <  0  ) { th_global = th_global + 360; }
    else if ( th_global > 360 ) { th_global = th_global - 360; }

    frc::SmartDashboard::PutNumber("desired_left_speed",  desired_left_speed );
    frc::SmartDashboard::PutNumber("desired_right_speed", desired_right_speed);

    frc::SmartDashboard::PutNumber("robot_x",  x_global );
    frc::SmartDashboard::PutNumber("robot_y",  y_global );
    frc::SmartDashboard::PutNumber("robot_th", th_global);

    frc::SmartDashboard::PutNumber("vx",  vx );
    frc::SmartDashboard::PutNumber("vy",  vy );
    frc::SmartDashboard::PutNumber("vth", vth);

    frc::SmartDashboard::PutBoolean("Stop Button",  hardware->GetStopButton() );

    frc::SmartDashboard::PutNumber("desired_vx",  desired_vx );
    frc::SmartDashboard::PutNumber("desired_vy",  desired_vy );
    frc::SmartDashboard::PutNumber("desired_vth", desired_vth );

    std::cout << "x: " << x_global << " y: " << y_global << " th: " << th_global << std::endl;

}

double Movement::get_x() { return x_global;  }

double Movement::get_y() { return y_global;  }

double Movement::get_th(){ return th_global; }

void Movement::angular_align(){

    printf("Starting Angular Aligment");

    frc::SmartDashboard::PutString("Process",  "Angular Align" );

    int count = 0;

    Twist cmd;

    while( count < 3 ){

        sensor->Periodic();

        double th_diff = sensor->get_angle_wall( 2 );

        double dist_offset = 5;    // [degrees]
        double max_speed   = 0.75;   // [rad/s]
        double min_ang_speed = 0.4;     // [rad/s] 
        float tolerance    = 3;     // [degrees]
 
        double desired_v = ( th_diff / dist_offset) * max_speed;
        desired_v =  std::max( std::min( desired_v, max_speed ), -1 * max_speed );
        if( abs(desired_v) < min_ang_speed ){
          desired_v > 0 ? desired_v = min_ang_speed : desired_v = -min_ang_speed; }

        if( abs(th_diff) < tolerance ){ desired_v = 0; count++; }
        else{ count = 0; }

        cmd.linear.x = 0;
        cmd.linear.y = 0;
        cmd.angular.z = desired_v;

        cmd_drive( cmd.linear.x, cmd.linear.y, cmd.angular.z );

        delay(20);
    }
}

void Movement::line_align( std::string direction ){

    frc::SmartDashboard::PutString("Process",  "Cobra Align" );

    bool cobra_l  = false;
    bool cobra_r  = false;
    bool cobra_cl = false;
    bool cobra_cr = false;


    while( !cobra_cl || !cobra_cr ){
        sensor->Periodic();

        cobra_l  = sensor->cobra_l;
        cobra_r  = sensor->cobra_r;
        cobra_cl = sensor->cobra_cl;
        cobra_cr = sensor->cobra_cr;

        float des_ang = straight_ang( get_th() );
        float th_diff = des_ang - get_th();

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

        if      ( direction.compare( "left" ) == 0){
            cmd_drive( 0,  20, desired_vth );  
        }else if( direction.compare( "right" ) == 0 ){
            cmd_drive( 0, -20, desired_vth );  
        }else{
            break;
        }

        frc::SmartDashboard::PutNumber("des_ang", des_ang );
        frc::SmartDashboard::PutNumber("get_th()", get_th() );

        delay(50);
    }
    cmd_drive( 0, 0, 0 );  
    delay(500);


}

void Movement::UpdateWheelsSpeed(){

    current_time = time.Get();
    delta_time = current_time - previous_time; // [s]
    previous_time = current_time;

    if( delta_time > 0.5 ){ delta_time = 0; }

    current_enc_l = hardware->GetLeftEncoder();
    double delta_enc_l = current_enc_l - previous_enc_l;
    previous_enc_l = current_enc_l;

    current_enc_r = hardware->GetRightEncoder();
    double delta_enc_r = current_enc_r - previous_enc_r;
    previous_enc_r = current_enc_r;

    //Wheels Velocity
    leftVelocity  = WheelSpeed(delta_enc_l, delta_time );     // [cm/s]
    rightVelocity = WheelSpeed(delta_enc_r, delta_time );     // [cm/s]

}
