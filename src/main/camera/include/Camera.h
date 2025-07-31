#pragma once

#include <frc/shuffleboard/Shuffleboard.h>
#include <cameraserver/CameraServer.h>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>

#include "Movement.h"

using namespace cv;
using namespace std;

struct Object {
    string name;
    Scalar lower_limit;
    Scalar upper_limit;
    Scalar lower_limit_2;
    Scalar upper_limit_2;
    int open_iteration;
    int close_iteration;
    int min_area;
    int max_area;
    int area;
    bool use_second_mask;
};

class Camera{
    public:
        Camera( Movement * m ) : move{m}{}
        void StartCamera();
        void DetectFruit( vector<string> obj_names, double angle, bool debug );

    private:
        Movement * move;

        double x, y, th;
        bool limit_switch;

        const vector<Object> objects = {
            {"grape_yellow", {16,25,25}, {27,255,255}, {}, {}, 2, 2, 500, 25000, 20000, false},
            {"grape_green",  {30,25,25}, {69,255,255}, {}, {}, 2, 1, 500, 25000, 20000, false},
            {"grape_purple", {0,25,0},   {10,255,100}, {170,25,0}, {180,255,100}, 2, 3, 500, 25000, 12000, true},
            {"banana",       {20,100,100},{40,255,255}, {}, {}, 2, 2, 500, 25000, 19500, false},
            {"lemon",        {40,100,0}, {75,255,100}, {}, {}, 2, 2, 500, 20000, 19500, false},
            {"basket_stand", {20,100,100},{40,255,255}, {}, {}, 2, 2, 500, 25000, 19500, false}
        };
    
};