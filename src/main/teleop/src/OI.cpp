#include "OI.h"

#define DEBUG

/**
 * Processes joystick events
 */
void OI::ProcessJoystickEvents() {
    while (read(joystick_fd, &event, sizeof(event)) > 0) {
        if (event.type == JS_EVENT_AXIS) {
            axisMap[event.number] = event.value;
        } else if (event.type == JS_EVENT_BUTTON) {
            buttonMap[event.number] = event.value;
        }
    }
    #ifdef DEBUG
    frc::SmartDashboard::PutNumber("Event Value", event.value);
    frc::SmartDashboard::PutNumber("Event Number", event.number);
    #endif
}

/**
 * @return the y-axis value from the right joystick
 */
double OI::GetRightDriveY(void) {
    ProcessJoystickEvents();
    double joy = axisMap[RIGHT_ANALOG_Y] / 32767.0; // Normalize to [-1.0, 1.0]
    if (fabs(joy) < 0.05)
        return 0.0;
    else
        return joy;
}

/**
 * @return the x-axis value from the right joystick
 */
double OI::GetRightDriveX(void) {
    ProcessJoystickEvents();
    double joy = axisMap[RIGHT_ANALOG_X] / 32767.0; // Normalize to [-1.0, 1.0]
    if (fabs(joy) < 0.05)
        return 0.0;
    else
        return joy;
}

/**
 * @return the y-axis value from the drivePad left joystick
 */
double OI::GetLeftDriveY(void)
{
    ProcessJoystickEvents();
    double joy = axisMap[LEFT_ANALOG_Y] / 32767.0; // Normalize to [-1.0, 1.0]
    if (fabs(joy) < 0.05)
        return 0.0;
    else
        return joy;
}

/**
 * @return the x-axis value from the drivePad left joystick
 */
double OI::GetLeftDriveX(void)
{
    ProcessJoystickEvents();
    double joy = axisMap[LEFT_ANALOG_X] / 32767.0; // Normalize to [-1.0, 1.0]
    if (fabs(joy) < 0.05)
        return 0.0;
    else
        return joy;
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveRightTrigger(void)
{
    ProcessJoystickEvents();
    return buttonMap[RIGHT_TRIGGER];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveRightBumper(void)
{
    ProcessJoystickEvents();
    return buttonMap[RIGHT_BUMPER];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveLeftTrigger(void)
{
    ProcessJoystickEvents();
    return buttonMap[LEFT_TRIGGER];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveLeftBumper(void)
{
    ProcessJoystickEvents();
    return buttonMap[LEFT_BUMPER];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveXButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[X_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveSquareButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[SQUARE_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveCircleButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[CIRCLE_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveTriangleButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[TRIANGLE_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveOptionsButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[OPTIONS_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveShareButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[SHARE_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveRightAnalogButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[RIGHT_ANALOG_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveLeftAnalogButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[LEFT_ANALOG_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDrivePS4Button(void)
{
    ProcessJoystickEvents();
    return buttonMap[PS4_BUTTON];
}

/**
 * @return a true or false depending on the input
 */
bool OI::GetDriveTouchpadButton(void)
{
    ProcessJoystickEvents();
    return buttonMap[TOUCHPAD_BUTTON];
}

/**
 * @return the POV hat switch position (assumed to be an axis)
 */
int OI::GetDrivePOV(void)
{
    ProcessJoystickEvents();
    if (axisMap[POV_Y] < 0 && axisMap[POV_X] == 0)
        return 0;
    else if (axisMap[POV_Y] < 0 && axisMap[POV_X] > 0)
        return 45;
    else if (axisMap[POV_Y] == 0 && axisMap[POV_X] > 0)
        return 90;
    else if (axisMap[POV_Y] > 0 && axisMap[POV_X] > 0)
        return 135;
    else if (axisMap[POV_Y] > 0 && axisMap[POV_X] == 0)
        return 180;
    else if (axisMap[POV_Y] > 0 && axisMap[POV_X] < 0)
        return 225;
    else if (axisMap[POV_Y] == 0 && axisMap[POV_X] < 0)
        return 270;
    else if (axisMap[POV_Y] < 0 && axisMap[POV_X] < 0)
        return 315;
    else
        return -1;
}