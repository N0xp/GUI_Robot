/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#include "Hardware.h"

#define DEBUG true

Hardware::Hardware()
{
    ResetEncoders();
    ResetYaw();
}

void Hardware::ResetEncoders()
{
    LeftEncoder.Reset();
    BackEncoder.Reset();
    RightEncoder.Reset();
    ElevatorEncoder.Reset();
}

void Hardware::ResetYaw()
{
    navX.ZeroYaw();
}
void Hardware::SetLeft( double pwm ){
    if( pwm == 0 ){ LeftMotor.StopMotor();}
    else{ LeftMotor.Set( pwm ); }
}
void Hardware::SetRight( double pwm ){
    if( pwm == 0 ){ RightMotor.StopMotor();}
    else{ RightMotor.Set( -pwm ); }
}
void Hardware::SetBack( double pwm ){
    if( pwm == 0 ){ BackMotor.StopMotor();}
    else{ BackMotor.Set( pwm ); }
}
void Hardware::SetElevator( double pwm ){
    if( pwm == 0 ){ ElevatorMotor.StopMotor();}
    else{ ElevatorMotor.Set( -pwm ); }
}

void Hardware::SetGripper( double angle ){

    if     ( angle > 300 ){ angle = 300; }
    else if( angle < 0 )  { angle = 0; }

    grip_ang = angle;
    servo_gripper.SetAngle( angle );
}
void Hardware::SetGripperOff(  ){
    servo_gripper.SetOffline( );
}
void Hardware::SetBase( double angle ){

    if     ( angle > 300 ){ angle = 300; }
    else if( angle < 0 )  { angle = 0; }

    base_ang = angle;
    servo_base.SetAngle( angle );
}
void Hardware::SetBaseOff( ){
    servo_base.SetOffline( );
}
void Hardware::SetArm( double angle ){

    if     ( angle > 300 ){ angle = 300; }
    else if( angle < 0 )  { angle = 0; }

    arm_ang = angle;
    servo_arm.SetAngle( angle );
}
void Hardware::SetArmOff( ){
    servo_arm.SetOffline( );
}

void Hardware::SetRunningLED(bool on)
{
    runningLED.Set(on);
}

void Hardware::SetStoppedLED(bool on)
{
    stoppedLED.Set(on);
}

double Hardware::GetLeftEncoder()
{
    return LeftEncoder.GetRaw();
}

double Hardware::GetBackEncoder()
{
    return BackEncoder.GetRaw();
}

double Hardware::GetRightEncoder()
{
    return -RightEncoder.GetRaw();
}

double Hardware::GetElevatorEncoder()
{
    return -ElevatorEncoder.GetRaw();
}

double Hardware::GetYaw()
{
    return navX.GetYaw();
}

double Hardware::GetAngle()
{
    return navX.GetAngle();
}

bool Hardware::GetStopButton(){
    return stopButton.Get();
}
bool Hardware::GetStartButton(){
    return startButton.Get();
}

bool Hardware::GetLimitHigh(){
    return switchHigh.Get();
}
bool Hardware::GetLimitLow(){
    return switchLow.Get();
}

double Hardware::GetCobra( int channel ){
    return cobra.GetVoltage(channel);
}

double Hardware::GetRightSharp(){
    return sharp_function_right( sharp_right.GetVoltage() );
}
double Hardware::GetRightSharpVoltage(){
    return sharp_right.GetVoltage();
}
double Hardware::GetLeftSharp(){
    return sharp_function_left( sharp_left.GetVoltage() );
}
double Hardware::GetLeftSharpVoltage(){
    return sharp_left.GetVoltage();
}
double Hardware::GetArmSharp(){
    return sharp_function_left( sharp_arm.GetVoltage() );
}
double Hardware::GetArmSharpVoltage(){
    return sharp_arm.GetVoltage();
}

double Hardware::GetRightUS(){
    us_r.Ping();
    return us_r.GetRangeMM() / 10.0;
}
double Hardware::GetLeftUS(){
    us_l.Ping();
    return us_l.GetRangeMM() / 10.0;
}

void Hardware::StopActuators(){
    
    SetElevator( 0 );
    SetLeft ( 0 );
    SetRight( 0 );
    SetGripperOff( );
    SetBaseOff( );
    SetArmOff( );
}

void Hardware::ReactivateActuators(){
    SetGripper( grip_ang );
    SetBase( base_ang );
    SetArm( arm_ang );
}