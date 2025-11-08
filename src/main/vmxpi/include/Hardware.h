/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#pragma once

#include <frc/smartdashboard/SmartDashboard.h>
#include <frc/DigitalInput.h>
#include <frc/DigitalOutput.h>
#include <frc/AnalogInput.h>
#include <frc/Ultrasonic.h>

#include "studica/TitanQuad.h"
#include "studica/TitanQuadEncoder.h"
#include "studica/Servo.h"
#include "studica/Cobra.h"

#include "Constants.h"
#include "Functions.h"

#include "AHRS.h"
#include <math.h>


class Hardware
{
    public:
        Hardware();
        double GetLeftEncoder(void);
        double GetBackEncoder(void);
        double GetRightEncoder(void);
        double GetElevatorEncoder(void);
        double GetYaw(void);
        double GetAngle(void);
        double GetCobra( int channel );
        void ResetYaw(void);
        void ResetEncoders(void);
        void SetGripper( double angle );
        void SetGripperOff(  );
        void SetBase( double angle );
        void SetBaseOff( );
        void SetArm( double angle );
        void SetArmOff( );
        void SetRunningLED(bool on);
        void SetStoppedLED(bool on);
        void SetLeft( double pwm );
        void SetRight( double pwm );
        void SetBack( double pwm );
        void SetElevator( double pwm );

        void StopActuators();
        void ReactivateActuators();


        bool GetStartButton();
        bool GetStopButton();
        bool GetLimitHigh();
        bool GetLimitLow();

        double GetRightSharp();
        double GetLeftSharp();
        double GetArmSharp();

        double GetRightSharpVoltage();
        double GetLeftSharpVoltage();
        double GetArmSharpVoltage();

        double GetRightUS();
        double GetLeftUS();



        int arm_ang  = 300;
        int grip_ang = 0;
        int base_ang = 150;



    private:
        studica::TitanQuad LeftMotor     {constant::TITAN_ID, 15600, constant::LEFT_MOTOR    };
        studica::TitanQuad BackMotor     {constant::TITAN_ID, 15600, constant::BACK_MOTOR    };
        studica::TitanQuad RightMotor    {constant::TITAN_ID, 15600, constant::RIGHT_MOTOR   };
        studica::TitanQuad ElevatorMotor {constant::TITAN_ID, 15600, constant::ELEVATOR_MOTOR};

        studica::TitanQuadEncoder LeftEncoder     {LeftMotor,     constant::LEFT_MOTOR,     constant::DIST_PER_TICK};
        studica::TitanQuadEncoder BackEncoder     {BackMotor,     constant::BACK_MOTOR,     constant::DIST_PER_TICK};
        studica::TitanQuadEncoder RightEncoder    {RightMotor,    constant::RIGHT_MOTOR,    constant::DIST_PER_TICK};
        studica::TitanQuadEncoder ElevatorEncoder {ElevatorMotor, constant::ELEVATOR_MOTOR, constant::DIST_PER_TICK};

        studica::Servo servo_gripper{4};
        studica::Servo servo_base{5}; 
        studica::Servo servo_arm{6}; 


        AHRS navX{frc::SPI::Port::kMXP};

        frc::DigitalInput startButton{constant::START_BUTTON};
        frc::DigitalInput stopButton {constant::STOP_BUTTON };
        frc::DigitalInput switchHigh {constant::LIMIT_HIGH  };
        frc::DigitalInput switchLow  {constant::LIMIT_LOW   };

        frc::DigitalOutput runningLED{constant::RUNNING_LED};
        frc::DigitalOutput stoppedLED{constant::STOPPED_LED};

        studica::Cobra cobra{};

        frc::AnalogInput sharp_right{constant::SHARP_RIGHT};
        frc::AnalogInput sharp_left {constant::SHARP_LEFT };
        frc::AnalogInput sharp_arm  {constant::SHARP_ARM  };

        frc::Ultrasonic us_l{ constant::US_LEFT_TRIG,  constant::US_LEFT_ECHO  };
        frc::Ultrasonic us_r{ constant::US_RIGHT_TRIG, constant::US_RIGHT_ECHO }; 




};

