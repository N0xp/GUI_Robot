#include "procedures.h"

void take_fruit( std::vector<std::string> fruits, std::string direction ){


    if      ( direction.compare( "front" ) == 0 ){
        set_base( 0 );
    }else if( direction.compare( "right" ) == 0 ){
        set_base( -90 );
    }else if( direction.compare( "left" )  == 0 ){
        set_base( 90 );
    }

    set_arm( 300 );
    oms_driver( 20 );

    DetectFruitSharp( fruits );

    int arm_ang = hard.arm_ang;

    set_arm( arm_ang + 100 );

    oms_driver( oms.height + 10 );

    set_arm( arm_ang );

    set_gripper( GRIPPER_GRAPE );
    delay(1000);

}

void store_fruit(){

    set_arm( 300 );

    reset_height( 1 );
    set_base( -180 );

    set_gripper( GRIPPER_OPEN );
    delay( 1000 );

}