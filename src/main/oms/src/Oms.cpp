/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#include "Oms.h"

void Oms::oms_driver( double desired_height ){

    float desired_speed = 0;

    const float enc_prop = 1.0;

    frc::Timer time;
    time.Start();

    double current_time  = time.Get();
    double previous_time = time.Get();

    int current_enc  = hardware->GetElevatorEncoder() * enc_prop;
    int previous_enc = hardware->GetElevatorEncoder() * enc_prop;

    pid_e.Reset();
    pid_e.SetIntegratorRange(-2.5, 2.5);

    if ( desired_height <= high_height && desired_height >= low_height ){
        
        do{
            current_time = time.Get();
            double delta_time = current_time - previous_time;                  // [s]
            previous_time = current_time;

            current_enc = hardware->GetElevatorEncoder() * enc_prop;
            float delta_enc = current_enc - previous_enc;
            previous_enc = current_enc;

            //Pinion Velocity
            double elevatorVelocity  = (((2 * M_PI * pinionRadius * delta_enc) / (constant::PULSE_PER_REV * delta_time)));   // [cm/s]

            if ( isnan(elevatorVelocity) || isinf(elevatorVelocity) ){ elevatorVelocity  = 0; }
            
            //Elevation Displacement
            float delta_elev  = elevatorVelocity  * delta_time; // Displacement per iteration

            height = height + delta_elev;   // [cm]

            if( !hardware->GetLimitHigh() ){ height = high_height; }
            if( !hardware->GetLimitLow()  ){ height =  low_height; }

            float elev_diff  = desired_height - height;

            float max_speed = 35;          // [cm/s]

            desired_speed = (elev_diff / 5) * max_speed;
            desired_speed = std::max( std::min( desired_speed, max_speed ), -1 * max_speed );
            if( abs(elev_diff) < tolerance ){ desired_speed = 0; }


            if ( hardware->GetStopButton() ){  // Stop the Motors when the Stop Button is pressed
                hardware->SetElevator( 0 );
                pid_e.Reset();
            }else{
                hardware->SetElevator( std::clamp(pid_e.Calculate(elevatorVelocity / 60.0, desired_speed / 60.0),  -0.75, 0.75) );
            }
            
            printf( "height: %f, desired_speed: %f\n", height, desired_speed ); 

            frc::SmartDashboard::PutNumber("height",  height );


            delay( 40 );

        }while( desired_speed != 0 );

    }else{
        printf( "Desired Position out of the range %f to %f", low_height, high_height );
    }

    hardware->SetElevator( 0 );
    delay(150);
 
}

void Oms::reset( int direction ){
    
    if( direction == 1 ){
        while( hardware->GetLimitHigh() ){ 
            if( hardware->GetStopButton() ){ hardware->SetElevator( 0.0 ); }
            else{ hardware->SetElevator( 0.4 ); }
            delay(50); 
        }
        height = high_height;
    }else if (direction == -1){
        while( hardware->GetLimitLow() ) { 
            if( hardware->GetStopButton() ){ hardware->SetElevator( 0.0 ); }
            else{ hardware->SetElevator( -0.4 ); }
            delay(50); }
        height = low_height;
    }

    hardware->SetElevator( 0 );

    printf("new height is %f", height);

    delay(150);

}

void Oms::set_gripper( int ang ){
    hardware->SetGripper( ang );
}

void Oms::set_base( int ang ){

    float angle = ang + 240;

    float increment = 4;

    for ( int i = 0; i <= abs( angle - prev_base_ang ) / increment; i++ ){
        if( angle > prev_base_ang){
            ang = prev_base_ang + (i * increment);
        }else{
            ang = prev_base_ang - (i * increment);
        }
        hardware->SetBase( ang );
        delay( 50 );
    }

    prev_base_ang = angle;
}