
/*

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


    Copyright (C) Lorenzo Miori lorenzo.miori[at]gmail[dot]com

    #    #   ##   #    # ###### #####   ####  #####    ##    ####  ###### 
    ##  ##  #  #  #   #  #      #    # #      #    #  #  #  #    # #      
    # ## # #    # ####   #####  #    #  ####  #    # #    # #      #####  
    #    # ###### #  #   #      #####       # #####  ###### #      #      
    #    # #    # #   #  #      #   #  #    # #      #    # #    # #      
    #    # #    # #    # ###### #    #  ####  #      #    #  ####  ###### 

    http://makerspace.inf.unibz.it/

    learning by doing

    ! Come and visit us !

*/

#include <Arduino.h>
#include <Servo.h>

/** Structure holding configuration angles */
typedef struct _t_arm_config
{

    uint8_t swing_closed;         /**< Arm swing start agular position [°] */
    uint8_t swing_open;           /**< Arm swing end agular position [°] */
    uint8_t rotation_closed;      /**< Arm rotation start agular position [°] */
    uint8_t rotation_open;        /**< Arm rotation end agular position [°] */

    uint8_t swing_step;           /**< Arm swing movement step [°] */
    uint32_t swing_step_delay;    /**< Arm swing movement step delay [us] */
    uint8_t rotation_step;        /**< Arm rotation movement step [°] */
    uint32_t rotation_step_delay; /**< Arm rotation movement step delay [us] */

} t_arm_config;

/** Structure holding the status of the associated servo motor */
typedef struct _t_servo_movement
{
    uint8_t step_val;        /**< The angular step that is added/subracted every step_delay [°] */
    uint32_t step_delay;     /**< The delay between each step [us] */
    uint32_t start;          /**< The start timestamp [us] */
    uint8_t angle_position;  /**< Current angular position [°] */
    uint8_t angle_end;       /**< Destination angular position [°] */
} t_servo_movement;

Servo arm_swing_servo;      /**< Servo motor control for the arm swing position */
Servo arm_rotation_servo;   /**< Servo motor control for the arm rotation position */
t_arm_config arm_config;    /**< Arm configuration (angles and timings) */

t_servo_movement arm_swing;     /**< Status of the arm swing servo movement */
t_servo_movement arm_rotation;  /**< Status of the arm rotation servo movement */

/* Function declarations */
void arm_init(t_arm_config *config, Servo swing_servo, Servo rotation_servo, t_servo_movement *arm_swing);
void arm_logic(uint32_t timestamp);
bool servo_movement(t_servo_movement *movement, uint32_t timestamp);
void arm_close(t_arm_config *config, t_servo_movement *movement);
void arm_open(t_arm_config *config, t_servo_movement *movement);

bool servo_movement(t_servo_movement *movement, uint32_t timestamp)
{

    bool movement_end = false;
    uint32_t end_time;

    if (movement->start == 0U)
    {
        movement->start = timestamp;
    }

    end_time = movement->start + movement->step_delay;

    if (movement->angle_position > movement->angle_end)
    {
        /* negative swing */
        if (movement->angle_position >= movement->angle_end)
        {
            if (timestamp >= end_time)
            {
                movement->angle_position -= movement->step_val;
                movement->start = 0;
            }
        }

        if (movement->angle_position <= movement->angle_end)
        {
            movement_end = true;
            movement->angle_position = movement->angle_end;
            movement->start = 0;
        }
    }
    else
    {
        /* positive swing */
        if (movement->angle_position <= movement->angle_end)
        {
            if (timestamp >= end_time)
            {
                movement->angle_position += movement->step_val;
                movement->start = 0;
            }
        }

        if (movement->angle_position >= movement->angle_end)
        {
            movement_end = true;
            movement->angle_position = movement->angle_end;
            movement->start = 0;
        }
    }

    return movement_end;

}

void arm_init(t_arm_config *config, Servo swing_servo, Servo rotation_servo, t_servo_movement *arm_swing)
{

    /* hardware */
    swing_servo.attach(9);
    rotation_servo.attach(10);
  
    /* positions */
    config->swing_closed = 15;
    config->swing_open = 100;
    config->rotation_closed = 80;
    config->rotation_open = 180;

    /* timings */
    config->swing_step = 2;
    config->swing_step_delay = 50000;
    config->rotation_step = 1;
    config->rotation_step_delay = 28000;
 
    /* configure the servo movement routine */
    arm_swing->step_val = config->swing_step;
    arm_swing->step_delay = config->swing_step_delay;
    arm_swing->start = 0;
    arm_swing->angle_position = 0U;
    arm_swing->angle_end = config->swing_closed;

}

void arm_close(t_arm_config *config, t_servo_movement *movement)
{
    movement->angle_end = config->swing_closed;
}

void arm_open(t_arm_config *config, t_servo_movement *movement)
{
    movement->angle_end = config->swing_open;
}

void arm_logic(uint32_t timestamp)
{

    static uint32_t old_timestamp = 0;
    uint32_t elapsed = 0;
    static bool mov1_done = false;
    static bool mov2_done = false;

    /* measure elapsed time */
    elapsed = timestamp - old_timestamp;
    old_timestamp = timestamp;

    if (mov1_done == true)
    {
        arm_close(&arm_config, &arm_swing);
        mov2_done = servo_movement(&arm_swing, timestamp);
        if (mov2_done == true)
        {
            mov1_done = false;
            mov2_done = false;
        }
    }
    else
    {
        arm_open(&arm_config, &arm_swing);
        mov1_done = servo_movement(&arm_swing, timestamp);
    }

    /* operate the servo */
    arm_swing_servo.write(arm_swing.angle_position);

}


void setup() {
    arm_init(&arm_config, arm_swing_servo, arm_rotation_servo, &arm_swing);
    Serial.begin(9600);
}

void loop()
{
    /* execute the main logic */
    arm_logic(micros());
}
