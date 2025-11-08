/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/DigitalInput.h>
#include <frc/DigitalOutput.h>
#include <frc/AnalogInput.h>

#include "studica/TitanQuad.h"
#include "studica/TitanQuadEncoder.h"
#include "studica/Servo.h"
#include "studica/Cobra.h"

#include "AHRS.h"

#include "Constants.h"
#include "Functions.h"

#include <studica/MockDS.h>


#include "Robot.h"


int Inspection() { 

    // Start MockDS
    Robot r;
    r.ds.Enable();

    lidar.StartLidar();

    delay(500);

    frc::SmartDashboard::PutNumber("Servo Gripper", -1 );
    frc::SmartDashboard::PutNumber("Servo Base", -1 );
    frc::SmartDashboard::PutNumber("Servo Arm", -1 );

    int servo_grip_ang = -1;
    int servo_base_ang = -1;
    int servo_arm_ang  = -1;


    frc::SmartDashboard::PutNumber("Motor Left", 0 );
    frc::SmartDashboard::PutNumber("Motor Right", 0 );
    frc::SmartDashboard::PutNumber("Motor Back", 0 );
    frc::SmartDashboard::PutNumber("Motor Elevetor", 0 );

    float ml = 0;
    float mr = 0;
    float mb = 0;
    float me = 0;

    frc::SmartDashboard::PutBoolean("Stop", false );

    bool STOP = false;

    std::cout << "************* INSPECTION *************" << std::endl;


    while( !STOP ){

        lidar.Periodic();

        /******* Sharp Sensor *******/
        frc::SmartDashboard::PutNumber("Analog Right [cm]", hard.GetRightSharp() );
        frc::SmartDashboard::PutNumber("Analog Left [cm]" , hard.GetLeftSharp()  );
        frc::SmartDashboard::PutNumber("Analog Arm [cm]" ,  hard.GetArmSharp()   );

        frc::SmartDashboard::PutNumber("Analog Right [V]", hard.GetRightSharpVoltage() );
        frc::SmartDashboard::PutNumber("Analog Left [V]" , hard.GetLeftSharpVoltage()  );
        frc::SmartDashboard::PutNumber("Analog Arm [V]" ,  hard.GetArmSharpVoltage()   );

        /******* US Sensor *******/
        frc::SmartDashboard::PutNumber("US Left [cm]" ,   hard.GetLeftUS()  );
        frc::SmartDashboard::PutNumber("US Right [cm]" ,  hard.GetRightUS() );


        /******* Encoder *******/
        frc::SmartDashboard::PutNumber("Encoder Left",     hard.GetLeftEncoder()     );
        frc::SmartDashboard::PutNumber("Encoder Right",    hard.GetRightEncoder()    );
        frc::SmartDashboard::PutNumber("Encoder Back",     hard.GetBackEncoder()     );
        frc::SmartDashboard::PutNumber("Encoder Elevator", hard.GetElevatorEncoder() );

        /******* Digital Input *******/
        frc::SmartDashboard::PutNumber("Digital Input Start",      hard.GetStartButton());
        frc::SmartDashboard::PutNumber("Digital Input Stop",       hard.GetStopButton() );
        frc::SmartDashboard::PutNumber("Digital Input switchHigh", hard.GetLimitHigh()  );
        frc::SmartDashboard::PutNumber("Digital Input switchLow",  hard.GetLimitLow()   );

        /******* Cobra *******/
        frc::SmartDashboard::PutNumber("cobra_l",  hard.GetCobra(0) );
        frc::SmartDashboard::PutNumber("cobra_r",  hard.GetCobra(3) );
        frc::SmartDashboard::PutNumber("cobra_cl", hard.GetCobra(1) );
        frc::SmartDashboard::PutNumber("cobra_cr", hard.GetCobra(2) ); 

        /******* navX *******/
        frc::SmartDashboard::PutNumber("Yaw",  hard.GetYaw() );

        /******* Servo *******/
        servo_grip_ang = frc::SmartDashboard::GetNumber("Servo Gripper", -1 );
        servo_base_ang = frc::SmartDashboard::GetNumber("Servo Base", -1 );
        servo_arm_ang  = frc::SmartDashboard::GetNumber("Servo Arm", -1 );

        if( servo_base_ang != -1 && !hard.GetStopButton() ){ hard.SetBase( servo_base_ang );
        }else{ hard.SetBaseOff( ); }

        if( servo_grip_ang != -1 && !hard.GetStopButton() ){ hard.SetGripper( servo_grip_ang );
        }else{ hard.SetGripperOff( ); }

        if( servo_arm_ang != -1 && !hard.GetStopButton() ) { hard.SetArm( servo_arm_ang );
        }else{ hard.SetArmOff( ); }


        /******* Motor *******/
        ml = frc::SmartDashboard::GetNumber("Motor Left", 0 );
        mr = frc::SmartDashboard::GetNumber("Motor Right", 0 );
        mb = frc::SmartDashboard::GetNumber("Motor Back", 0 );
        me = frc::SmartDashboard::GetNumber("Motor Elevetor", 0 );

        if( hard.GetStopButton() ){ ml = mr = mb = me = 0; }

        hard.SetLeft( ml );
        hard.SetRight( mr );
        hard.SetBack( mb );
        hard.SetElevator( me );        

        // std::cout << "************* INSPECTION *************" << std::endl;



        STOP = frc::SmartDashboard::GetBoolean("Stop", false );


        delay(750);
    }

    // lidar.StopLidar(); 
    r.ds.Disable();

    return 0;
}