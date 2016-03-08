
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
#include <EEPROM.h>

/************************/
/*     DEFINITIONS      */
/************************/

#define BUTTON_START_PIN    7U             /**< Button start PIN [#] */
#define DEBOUNCE_BUTTONS    100U * 1000U   /**< Button debounce time [us] */

/**< Enumerations for the logic state machine */
typedef enum
{

    ARM_NONE,
    ARM_STARTUP,
    ARM_WAIT_BUTTON,
    ARM_RELEASE_FULL,
    ARM_CLOSE,
    ARM_GRAB,
    ARM_ROTATE,
    ARM_ROTATE_BACK,
    ARM_OPEN,
    ARM_RELEASE_HALF

} e_arm_state;

/**< Enumeration of buttons */
enum e_buttons_
{

    BUTTON_START,

    NUM_BUTTONS

};

/************************/
/*      STRUCTURES      */
/************************/

/**< Keyboard status */
typedef struct _t_keypad
{

    bool       input[NUM_BUTTONS];
    uint32_t   debounce[NUM_BUTTONS];
    bool       latches[NUM_BUTTONS];
    bool       buttons[NUM_BUTTONS];

} t_keypad;

/** Rotation movement configuration */
typedef struct _t_rotation_movement
{

    uint8_t  angle_closed;         /**< Swing start agular position [°] */
    uint8_t  angle_open;           /**< Swing end agular position [°] */
    uint8_t  angle_step;           /**< Swing angular step [°] */
    uint32_t angle_step_delay;     /**< Arm swing movement step delay [us] */
  
} t_rotation_movement;

/** Structure holding configuration angles */
typedef struct _t_arm_config
{

    t_rotation_movement swing;      /**< Arm swing movement */    
    t_rotation_movement rotation;   /**< Arm rotation movement */

} t_arm_config;

/** Structure holding the status of the associated servo motor */
typedef struct _t_servo_movement
{

    uint8_t  step_val;        /**< The angular step that is added/subracted every step_delay [°] */
    uint32_t step_delay;      /**< The delay between each step [us] */
    uint32_t start;           /**< The start timestamp [us] */
    uint8_t  angle_position;  /**< Current angular position [°] */
    uint8_t  angle_end;       /**< Destination angular position [°] */

} t_servo_movement;

/************************/
/*      GLOBALS         */
/************************/

t_keypad keypad;            /**< Keypad driver status */

Servo arm_swing_servo;      /**< Servo motor control for the arm swing position */
Servo arm_rotation_servo;   /**< Servo motor control for the arm rotation position */
t_arm_config arm_config;    /**< Arm configuration (angles and timings) */

t_servo_movement arm_swing;     /**< Status of the arm swing servo movement */
t_servo_movement arm_rotation;  /**< Status of the arm rotation servo movement */

/************************/
/*     DECLARATIONS     */
/************************/

/* Function declarations */
void arm_init(t_arm_config *config, Servo swing_servo, Servo rotation_servo, t_servo_movement *arm_swing, t_servo_movement *arm_rotation);
void arm_logic(uint32_t timestamp);
bool servo_movement(t_servo_movement *movement, uint32_t timestamp);
void movement_close(t_rotation_movement *config, t_servo_movement *movement);
void movement_open(t_rotation_movement *config, t_servo_movement *movement);
void keypad_periodic(t_keypad* keypad, uint32_t timestamp);
void eeprom_write_position(uint8_t address, uint8_t position, uint8_t *prev_position);
void servo_set_position(t_servo_movement *servo, uint8_t pos);
bool timer_run(bool reset, uint32_t *timestamp, uint32_t *start, uint32_t timer);

/************************/
/*      FUNCTIONS       */
/************************/

bool timer_run(bool reset, uint32_t *timestamp, uint32_t *start, uint32_t timer)
{

    bool elapsed = false;
    
    if (reset == true)
    {
        /* reset the timer for a new firing */
        *start = 0;
        elapsed = true;
    }
    else if (*start == 0U)
    {
        /* start the timer */
        *start = *timestamp;
    }
    else if (*timestamp >= (*start + timer))
    {
        /* elapsed! */
        elapsed = true;
    }
 
    return elapsed;

}

void servo_set_position(t_servo_movement *servo, uint8_t pos)
{
    servo->angle_position = pos;
}

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
            /* movement completed */
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
            /* movement completed */
            movement_end = true;
            movement->angle_position = movement->angle_end;
            movement->start = 0;
        }
    }

    return movement_end;

}

void arm_init(t_arm_config *config, Servo swing_servo, Servo rotation_servo, t_servo_movement *arm_swing, t_servo_movement *arm_rotation)
{

    /* hardware */
    swing_servo.attach(9);
    rotation_servo.attach(10);

    /* positions */
    config->swing.angle_closed = 15;
    config->swing.angle_open = 100;
    config->rotation.angle_closed = 80;
    config->rotation.angle_open = 180;

    /* timings */
    config->swing.angle_step = 2;
    config->swing.angle_step_delay = 50000;
    config->rotation.angle_step = 1;
    config->rotation.angle_step_delay = 28000;

    /* configure the servo movement routine */
    arm_swing->step_val = config->swing.angle_step;
    arm_swing->step_delay = config->swing.angle_step_delay;
    arm_swing->start = 0;
    arm_swing->angle_position = config->swing.angle_closed;
    arm_swing->angle_end = config->swing.angle_closed;
    
    arm_rotation->step_val = config->rotation.angle_step;
    arm_rotation->step_delay = config->rotation.angle_step_delay;
    arm_rotation->start = 0;
    arm_rotation->angle_position = config->rotation.angle_closed;
    arm_rotation->angle_end = config->rotation.angle_closed;

}

void movement_close(t_rotation_movement *config, t_servo_movement *movement)
{
    movement->angle_end = config->angle_closed;
}

void movement_open(t_rotation_movement *config, t_servo_movement *movement)
{
    movement->angle_end = config->angle_open;
}

void arm_logic(uint32_t timestamp)
{

    bool elapsed;
    bool swing_movement_done = false;
    bool rot_movement_done = false;
    bool first_cycle = false;
    static e_arm_state state = ARM_STARTUP;
    static e_arm_state old_state = ARM_NONE;
    static uint8_t prev_pos_swing = 0;
    static uint8_t prev_pos_rot = 0;
    static uint32_t timer_start = 0;
    static uint32_t timer = 0;

    /* general purpouse timer */
    elapsed = timer_run(timer == 0 ? true : false, &timestamp, &timer_start, timer);
    if (elapsed == false) return;
    else timer = 0;

    /* ramp up-down the movement */
    swing_movement_done = servo_movement(&arm_swing, timestamp);
    rot_movement_done = servo_movement(&arm_rotation, timestamp);

    /* write position history when neeeded */
    if (state != ARM_STARTUP)
    {
        eeprom_write_position(0, arm_swing.angle_position, &prev_pos_swing);
        eeprom_write_position(1, arm_rotation.angle_position, &prev_pos_rot);
    }
    
    if (old_state != state)
    {
        swing_movement_done = false;
        rot_movement_done = false;
        first_cycle = true;
    }

    old_state = state;

    switch(state)
    {

        case ARM_STARTUP:

            /* last position is saved in the EEPROM */
            if (first_cycle == true)
            {
                /* read from eeprom */
                eeprom_read_position(0, &prev_pos_swing);
                eeprom_read_position(1, &prev_pos_rot);
                /* set servo position from the eeprom */
                servo_set_position(&arm_swing, prev_pos_swing);
                servo_set_position(&arm_rotation, prev_pos_rot);
                /* open the arm */
                movement_open(&arm_config.swing, &arm_swing);
                /* rotate the arm */
                movement_open(&arm_config.rotation, &arm_rotation);
            }

            if ((swing_movement_done == true) && (rot_movement_done == true))
            {
                /* boot phase completed */
                state = ARM_WAIT_BUTTON;
            }

            break;
      
        case ARM_WAIT_BUTTON:
            /* Wait for the button */
            if (keypad.buttons[BUTTON_START] == true)
            {
                /* Start pressed */
                state = ARM_CLOSE;
                timer = 500000;
            }
            break;
        case ARM_RELEASE_FULL:
            /* Open the hand */
            /* TODO */
            break;
        case ARM_CLOSE:
            /* Close the arm */
            movement_close(&arm_config.swing, &arm_swing);
            if (swing_movement_done == true)
            {
                /* movement completed, go to next state */
                state = ARM_ROTATE;
                timer = 500000;
            }
            break;
        case ARM_ROTATE:
            /* Arm rotate */
            movement_close(&arm_config.rotation, &arm_rotation);
            if (rot_movement_done == true)
            {
                /* movement completed, go to next state */
                state = ARM_ROTATE_BACK;
                timer = 500000;
            }
            break;
        case ARM_GRAB:
            /* Close the hand */
            /* TODO */
            break;
        case ARM_ROTATE_BACK:
            movement_open(&arm_config.rotation, &arm_rotation);
            if (rot_movement_done == true)
            {
                /* movement completed, go to next state */
                state = ARM_OPEN;
                timer = 500000;
            }
            break;
        case ARM_OPEN:
            /* Reopen the arm swing */
            movement_open(&arm_config.swing, &arm_swing);
            if (swing_movement_done == true)
            {
                /* movement completed, go to next state */
                state = ARM_WAIT_BUTTON;
            }
            break;
        case ARM_RELEASE_HALF:
            break;
        default:
            break;

    }

}

/* Read the keypad, apply debounce to inputs and detect the rising edge */
void keypad_periodic(t_keypad* keypad, uint32_t timestamp)
{

  uint8_t i = 0;
  bool t = false;

  for (i = 0; i < NUM_BUTTONS; i++)
  {
      t = keypad->input[i];
      
      if (t == true)
      {
          t = false;

          /* debounce the raw input */
          if (keypad->debounce[i] == 0)
              keypad->debounce[i] = timestamp;
          else
              if ((timestamp - keypad->debounce[i]) > DEBOUNCE_BUTTONS)
                  t = true;
      }
      else
      {
          keypad->debounce[i] = 0;
          t = false;
      }
      
      if ((t == true) && (keypad->latches[i] == false))
      {
          /* Falling edge */
          keypad->buttons[i] = true;
      }
      else
      {
          keypad->buttons[i] = false;
      }

      keypad->latches[i] = t;
  }

}

void eeprom_read_position(uint8_t address, uint8_t *prev_position)
{
    *prev_position = EEPROM.read(address);
    if (*prev_position > 180)
    {
        *prev_position = 0;
    }
}

void eeprom_write_position(uint8_t address, uint8_t position, uint8_t *prev_position)
{

    /* Frequently using the EEPROM is not very wise, so at least add
     * some "intelligence" to the process... */

    int16_t diff = position - *prev_position;
    diff = abs(diff);

    if (diff > 5U)
    {
        /* Movement done, write position */
        EEPROM.write(address, position);
        *prev_position = position;
    }
    else
    {
        /* Do not write the position yet */
    }
}

void setup()
{
    /* init structures */
    memset(&keypad, 0, sizeof(t_keypad));

    /* ARM hardware and software initialization */
    arm_init(&arm_config, arm_swing_servo, arm_rotation_servo, &arm_swing, &arm_rotation);

    /* I/O init */
    pinMode(BUTTON_START_PIN, INPUT_PULLUP);    /* start/stop button */

    /* debug serial port */
    Serial.begin(9600);

}

void loop()
{

    uint32_t ts = micros();

    /* Input processing */
    keypad.input[BUTTON_START] = digitalRead(BUTTON_START_PIN) ? false : true;
    keypad_periodic(&keypad, ts); 

    /* execute the main logic */
    arm_logic(ts);
    
    /* output processing */

    /* operate the servo */
    arm_swing_servo.write(arm_swing.angle_position);
    arm_rotation_servo.write(arm_rotation.angle_position);

}


