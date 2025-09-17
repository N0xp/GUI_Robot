/************************************
 * Author: Felipe Ferreira
 * Release version: 1.0.0.0
 * 
 * Modified by: 
 * Last modification date: 
 * New version:

*************************************/

#include "Robot.h"
#include "procedures.h"

int MainTask() { 


    A = { "A", {  35,  35,  -1 }, {&B, &C, &G}};
    B = { "B", {  35, 305,  -1 }, {&A, &C}    };
    C = { "C", {  35, 365,  -1 }, {&A, &B, &D}};
    D = { "D", { 370, 365,  -1 }, {&C, &E, &F}};
    E = { "E", { 370, 235,  -1 }, {&D, &F}    };
    F = { "F", { 370,  30,   0 }, {&D, &E, &A}};
    G = { "G", {  35,  35, 180 }, {&A, &H}    };
    H = { "H", { 200,  35, 180 }, {&G, &F}    };

    Coord take_basket_coord = { "t_b", { 40,  65, 0 }, {&F, &D, &E, &A} };
    Coord deli_basket_coord = { "d_b", { 40, 325, 0 }, {&F, &D, &E, &C} }; 

    Object obj1 = { grape_purple, {"obj1", {100, 365,   0}, {&C, &D}}};
    Object obj2 = { grape_yellow, {"obj2", {300, 365,   0}, {&C, &D}}};
    Object obj3 = { grape_green , {"obj3", {100, 305,   0}, {&B}    }};
    Object obj4 = { grape_yellow, {"obj4", {300, 235, 180}, {&E}    }};



    // // Start MockDS
    Robot r;
    r.ds.Enable();

    lidar.StartLidar();
    cam.StartCamera();

    delay(500);

    reset_height( 1 );
    set_base( -180 );
    set_arm( 300 );

    set_position( 30, 30, 270 );

    linear_align( 12, "front" );
    angular_align();

    robot_x = SR();                             // Left, Right or Front?
    robot_y = SFL();                              // Left, Right or Front?
    robot_th = setAngle();
    set_position( robot_x, robot_y, robot_th );  


    Coord current_point = { "cur", {get_x(),get_y(),get_th()}, {&A} };
    path_driver( best_way( obj4.coord, current_point ) );

    current_point       = { "cur", {get_x(),get_y(),get_th()}, {&E} };
    path_driver( best_way( A, current_point ) );


    // current_point = { "cur", {35,35,0}, {&F} };

    // path_driver( best_way( obj2.coord, current_point ) );


   return 0; 
}
