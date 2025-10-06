#pragma once

#include <frc/smartdashboard/SmartDashboard.h>

#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cmath>

class OI
{
    public: 
        double GetRightDriveY(void);
        double GetRightDriveX(void);
        double GetLeftDriveY(void);
        double GetLeftDriveX(void);
        int GetDrivePOV(void);
        bool GetDriveRightTrigger(void);
        bool GetDriveRightBumper(void);
        bool GetDriveLeftTrigger(void);
        bool GetDriveLeftBumper(void);
        bool GetDriveXButton(void);
        bool GetDriveSquareButton(void);
        bool GetDriveCircleButton(void);
        bool GetDriveTriangleButton(void);
        bool GetDriveOptionsButton(void);
        bool GetDriveShareButton(void);
        bool GetDriveRightAnalogButton(void);
        bool GetDriveLeftAnalogButton(void);
        bool GetDrivePS4Button(void);
        bool GetDriveTouchpadButton(void);
        void ProcessJoystickEvents();

    
    private:
        // Mapping for axes and buttons (these will be updated with events)

        // Open the joystick device file
        int joystick_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);

        // Structure to hold joystick events
        struct js_event event;
        std::unordered_map<int, int16_t> axisMap;
        std::unordered_map<int, bool> buttonMap;
    // Constants for axes and buttons (update these based on your joystick mappings)
        #define RIGHT_ANALOG_X       3               //ok
        #define RIGHT_ANALOG_Y       4               //ok
        #define LEFT_ANALOG_Y        1               //ok
        #define LEFT_ANALOG_X        0               //ok
        #define RIGHT_TRIGGER        7               //ok
        #define RIGHT_BUMPER         5               //ok
        #define LEFT_TRIGGER         6               //ok
        #define LEFT_BUMPER          4               //ok
        #define X_BUTTON             0               //ok
        #define SQUARE_BUTTON        3               //ok
        #define CIRCLE_BUTTON        1               //ok
        #define TRIANGLE_BUTTON      2               //ok   
        #define OPTIONS_BUTTON       9               //ok
        #define SHARE_BUTTON         8               //ok
        #define RIGHT_ANALOG_BUTTON  12              //ok
        #define LEFT_ANALOG_BUTTON   11              //ok
        #define PS4_BUTTON           10              //ok
        #define TOUCHPAD_BUTTON      17             // not triggering any event
        #define POV_X                6  // Assuming it's mapped to a specific axis or button
        #define POV_Y                7  // Assuming it's mapped to a specific axis or button
};