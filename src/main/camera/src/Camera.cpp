#include "Camera.h"

void Camera::StartCamera(){
    cs::UsbCamera mainCamera = frc::CameraServer::GetInstance() -> StartAutomaticCapture();
    mainCamera.SetResolution(640, 480);

    frc::Shuffleboard::GetTab("MainData").Add("Camera", mainCamera).WithPosition(0, 0).WithSize(10, 5);
}

void Camera::DetectFruit( vector<string> obj_names, double angle, bool debug ){
    cs::CvSource frameStream = frc::CameraServer::GetInstance()->PutVideo("Frame", 640, 480);

    frc::Shuffleboard::GetTab("MainData").Add("CameraProcess", frameStream).WithPosition(0, 0).WithSize(10, 5);


    cs::CvSink cvSink = frc::CameraServer::GetInstance()->GetVideo();
    Mat frame;

    double position[3];
    int count = 0, step = 0;
    bool find_obj = false;

    while (true) {
        if (cvSink.GrabFrame(frame) == 0) {
            frameStream.NotifyError(cvSink.GetError());
            continue;
        }

        //Camera Code Implementation -> From here:

        resize(frame, frame, Size(), 0.5, 0.5);
        rectangle(frame, Rect(0, 0, frame.cols, 60), Scalar(0, 0, 0), FILLED);

        Mat hsv;
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        vector<pair<Object, vector<vector<Point>>>> contour_list;


        for (const auto& obj : objects) {
            if (find(obj_names.begin(), obj_names.end(), obj.name) != obj_names.end()) {
                Mat mask, mask2;
                inRange(hsv, obj.lower_limit, obj.upper_limit, mask);
                if (obj.use_second_mask) {
                    inRange(hsv, obj.lower_limit_2, obj.upper_limit_2, mask2);
                    bitwise_or(mask, mask2, mask);
                }

                Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
                morphologyEx(mask, mask, MORPH_OPEN, kernel, Point(-1, -1), obj.open_iteration);
                morphologyEx(mask, mask, MORPH_CLOSE, kernel, Point(-1, -1), obj.close_iteration);

                vector<vector<Point>> contours;
                findContours(mask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
                contour_list.emplace_back(obj, contours);
            }
        }

        int max_area = 0, x = 0, y = 0, w = 0, h = 0;
        string color = "";
        int des_area = 0;
        vector<vector<Point>> contour;


        for (auto& item : contour_list) {
            const Object& obj = item.first;
            for (const auto& cnt : item.second) {
                double area = contourArea(cnt);
                Rect rect = boundingRect(cnt);
                if (area > max_area && area > obj.min_area && area < obj.max_area) {
                    x = rect.x; y = rect.y;
                    w = rect.width; h = rect.height;
                    max_area = area;
                    color = obj.name;
                    des_area = obj.area;
                    contour = item.second;
                }
            }
        }

        int obj_x = x + w / 2;
        int obj_y = y + h / 2;
        int offset_x = frame.cols / 2;
        int offset_y = 153;

        double vel_x = 0, vel_y = 0, vel_z = 0, vth = 0;
        double max_vel_x = 10, max_vel_y = 7.5, max_vel_z = 20, max_angular_speed = 0.75;

        double vx = max_vel_x * fabs(obj_x - offset_x) / 100.0;
        vx = min(vx, max_vel_x);

        double vy = max_vel_y * fabs(max_area - des_area) / 7500.0;
        vy = min(vy, max_vel_y);

        double vz = max_vel_z * fabs(obj_y - offset_y) / 50.0;
        vz = min(vz, max_vel_z);

        if (obj_x > offset_x + 30) {
            vel_x = -vx;
        } else if (obj_x < offset_x - 30) {
            vel_x = vx;
            find_obj = true;
        } else if (obj_y > offset_y + 35 || (obj_y > offset_y + 20 && abs(max_area - des_area) < 5000)) {
            vel_z = vz;
            find_obj = true;
        } else if ((obj_y < offset_y - 35 || (obj_y > offset_y + 20 && abs(max_area - des_area) < 5000)) && limit_switch) {
            vel_z = -vz;
        } else if (max_area > des_area + 2000) {
            vel_y = vy;
        } else if (max_area < des_area - 2000) {
            vel_y = -vy;
        }

        double th_diff = angle - th;
        if (th_diff > 180) th_diff -= 360;
        else if (th_diff < -180) th_diff += 360;

        if (fabs(th_diff) > 2.5) {
            vth = (th_diff / 15.0) * max_angular_speed;
            vth = max(-max_angular_speed, min(max_angular_speed, vth));
        }

        if (vel_x == 0 && vel_y == 0 && vel_z == 0 && vth == 0) {
            count++;
        }

        if (!find_obj) {
            position[0] = x;
            position[1] = y;
            position[2] = th;

        }

        if (count > 10) {
            step++;
            count = 0;
        }

        move->cmd_drive( vel_x, vel_y, vth );
        move->RobotPosition();  // Updates Robot position


        drawContours(frame, contour, -1, cv::Scalar(0, 255, 0), 2);


        frameStream.PutFrame(frame);
        // cout << "Color: " << color << " Area: " << max_area << endl;
        cout << "vel_x: " << vel_x << " vel_y: " << vel_y << " vel_z: " << vel_z << " vth: " << vth << endl;


        if (step > 3) break;
    }

    destroyAllWindows();
}


