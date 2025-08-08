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

int SimpleTask() { 

    // // Start MockDS
    Robot r;
    r.ds.Enable();

    lidar.StartLidar();
    cam.StartCamera();

    delay(500);

    reset_height( 1 );
    // set_base( -180 );
    set_arm( 300 );


    // set_position( 30, 30, 270 );

    // position_driver(  30, 370,  -1, true );
    // position_driver( 370, 370,  -1, true );
    // position_driver( 370, 230, 180, true );

    set_position( 370, 230, 180 );

    std::vector<std::string> fruits = {"grape_purple"};

    take_fruit( fruits, "right" );
    store_fruit();
    // take_fruit( fruits, "right" );
    // store_fruit();

    position_driver( 160, 230, 180, true );
    
    fruits = {"grape_purple"};

    take_fruit( fruits, "front" );
    linear_increment( 30, "back" );
    store_fruit();

    position_driver( 370, 230, 180, true );

    start_button();



    // reset_height( 1 );            // Goes UP
    // set_gripper ( GRIPPER_OPEN ); // Open Gripper
    // oms_driver( 35 );
    // set_base( -180 );             // Rotate Base to 180

    // led_red  ( false );
    // led_green( false );



    // start_button();
    // led_green  ( true );



    // /************ IDEAL INITIAL REFERENCE ************/

    // set_position( 30, 30, 180 );

    // /************ IDEAL INITIAL REFERENCE ************/


    // /**** INITIAL REFERENCE ****/
    // linear_align( 25, "left"  );                 // Left or Right?
    // linear_align( 12, "front" );
    // angular_align();

    // robot_x = SFR();                             // Left, Right or Front?
    // robot_y = SL();                              // Left, Right or Front?
    // robot_th = setAngle();
    // set_position( robot_x, robot_y, robot_th );  
    // /**** INITIAL REFERENCE ****/

    // start_button();



    // /***************** TAKE OBJECT 1 *****************/
    // /**** GO TO OBJECT 1 ****/
    // position_driver( 30, 30, 180, true );
    // position_driver( 50, 30, 180, true );
    // position_driver( 50, 130, 180, true );
    // position_driver( 50, 130, 90, true );



    // // start_button();

    // /**** TAKE OBJECT 1 ****/
    // set_base( 0 );
    // oms_driver( 25 );                                                   // CHECK
    // set_gripper( GRIPPER_OPEN );                                        // Open Gripper
    // linear_increment( 40, "front" );

    // // start_button();

    // set_gripper( GRIPPER_BALL_BIG );                                    // CHECK
    // delay( 1000 );
    // linear_increment( 40, "back" );
    // oms_driver( 25 );                                                   // CHECK
    // set_base( -180 );

    // /**** GO TO DELIVERY REFERENCE 1 ****/
    // position_driver( 150, 110,  move.get_th(), true );
    // position_driver( move.get_x(), move.get_y(), 270, true );
    // position_driver( move.get_x(),  50, 270, true );


    // /**** DELIVERY REFERENCE ****/
    // linear_align( 35, "right"  );                 // Left or Right?
    // linear_align( 15, "front" );
    // linear_align( 23, "right"  );                 // Left or Right?
    // angular_align();

    // robot_x = 100 + SR();                         // Left, Right or Front?
    // robot_y = SFR();                              // Left, Right or Front?
    // robot_th = setAngle();
    // set_position( robot_x, robot_y, robot_th );  

    // /**** GO TO DELIVERY PLACE 1 ****/
    // position_driver( 140, 45, 270, true );
    // set_base( 0 );
    // position_driver( 125, 45, 270, true );



    // /**** DELIVER OBJECT 1 ****/
    // set_base( 0 );
    // oms_driver( 20 );                                                   // CHECK
    
    // // start_button();

    // set_gripper( GRIPPER_OPEN ); // Open Gripper
    // delay( 1500 );

    // linear_increment( 15, "back" );

    // /***************** TAKE OBJECT 1 *****************/


    // /**** BACK to the Initial Position ****/

    // position_driver( 150, 110, 270, true );
    // reset_height( 1 );                                                   
    // set_base( -180 );
    // position_driver(  50, 110, 270, true );
    // position_driver(  30,  30, 180, true );

    // /**** BACK to the Initial Position ****/



    // lidar.StopLidar(); 
    r.ds.Disable();

    return 0; 
}
