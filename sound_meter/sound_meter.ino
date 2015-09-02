/*

    Sound Pressul Level Meter
    
      Uses salvaged components and boards to create a sound pressure level meter to
      saveguard your hearing.

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


    Copyright (C) Lorenzo Miori, 2nd Sept 2015

    History:

          Version 0.1 - first revision

*/

#define NUM_YELLOW_LED     12       /**< Total discrete levels */
#define NUM_RED_LED        5        /**< Number of extra red LEDs */

/*

After some testing, a lookup table with fixed duty cycles have been used.
This limits the adaptability to other boards / power supplies but is more flexible, especially when it comes to a quick fine tuning
of the values. Reference to the old formulas are kept as reference (actually, they have been used to create the starting values)

The principle behind these numbers is the following:
 The LED board has a "signal" pin that lights up a bar of LEDs to a certain level. All the 12 leds are lit when a level of about
 3.7V is applied to this signal pin. At about 1V, no LEDs are lit. Hence, roughly dividing this range (3.7 - 1.0)V by 12, you have the
 necessary steps to lit each one. This applies to the analog voltage domain, of course, and a conversion to the digital domain has to
 be applied. As a simplification, we always expect the Vcc of the microcontroller to be 5V sharp, and this is a pretty good choice and even tough
 we might recon a couple of hundrets of millivots of difference, this does not hugely influence our calculations.
 In few words, one can expect the RMS value of a PWM encoded signal to be 5V * duty cycle percentage (2.5V at 50% duty (128 as value)
 With this in mind, all the necessary steps are easily encoded to a duty cycle value ready to be fed to the Arduino library.

#define POWER_VCC           5000
#define FULL_SCALE_OUT      3770
#define DUTY_MAX            255
#define DUTY_PER_DOT        (uint8_t)((((float)FULL_SCALE_OUT / (float)POWER_VCC) * (float)DUTY_MAX) / LED_DRIVER_DOTS)
#define DUTY_OFFSET         (uint8_t)((1100.0f / (float)POWER_VCC) * (float)DUTY_MAX)
*/

const uint8_t LED_DUTY_TABLE[NUM_YELLOW_LED] = { 60, 75, 90, 101, 113, 124, 136, 147, 164, 173, 185, 200 };  /**< Lookup table for the duty cycles */
const uint8_t EXTRA_LED_PINS[NUM_RED_LED] = { 2, 3, 4, 6, 7 };                                                /**< Assigned pins for the extra LEDs */

void set_extra_led(uint8_t num)
{
    uint8_t i = 0;

    /* Turn all the LEDs off */
    for (i = 0; i < NUM_RED_LED; i++)
    {
        digitalWrite(EXTRA_LED_PINS[i], false);
    }

    if (num > NUM_RED_LED)
    {
        /* Clip the value if bigger than the maximum allowed */
        num = NUM_RED_LED;
    }

    /* Turn LEDs on when necessary */
    for (i = 0; i < num; i++)
    {
      digitalWrite(EXTRA_LED_PINS[i], true);
    }

}

void set_led(uint8_t level)
{

    if (level > NUM_YELLOW_LED)
    {
        /* clip the level if above the max */
        level = NUM_YELLOW_LED;
    }

    /* set the correct duty cycle */
    /* analogWrite(5, DUTY_OFFSET-DUTY_PER_DOT + (DUTY_PER_DOT * level)); */
    analogWrite(5, LED_DUTY_TABLE[level - 1]);

}

void set_level(uint8_t level)
{
  
  /* set the level at the first 12 LEDs (internally clips afer) */
  set_led(level);
  
  /* set the level if > LED_DRIVER_DOTS also to the red LEDs (internally clips after) */
  set_extra_led(level > NUM_YELLOW_LED ? level - NUM_YELLOW_LED : 0);
  
}

/* APP: initialization of subsystems */
void setup()
{

    uint8_t i;

    /* This output pin is used to generate the "analog" voltage for the LED-bar driver chip
     *  The board is setup to have a full-scale voltage of 3.77V @ 10V Vcc
     */
    pinMode(5, OUTPUT);

    /* Digital pins for the extra LEDs
     *  Set as (High Side) digital OUT: electrically connected via a 560 ohm resistor to the LEDs 
     */
    for (i = 0; i < NUM_RED_LED; i++)
    {
        pinMode(EXTRA_LED_PINS[i], OUTPUT);
    }

}

/* APP: main loop */
bool toggle;
int i = 0;
void loop()
{
  delay(100);
  set_level(i%18);
  i++;
  toggle ^= 1;
}
