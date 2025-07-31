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
    servo_gripper.SetAngle( angle );
}

void Hardware::SetBase( double angle ){
    servo_base.SetAngle( angle );
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
    return sharp_function( sharp_right.GetVoltage() );
}

double Hardware::GetLeftSharp(){
    return sharp_function( sharp_left.GetVoltage() );
}