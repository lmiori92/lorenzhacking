//#define LOG_OUT 1 // use the log output function
#define LIN_OUT8 1
#define FFT_N 128 // set to 128 point fft

#include <FFT.h>
//#include <ffft.h>

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

const uint8_t LED_DUTY_TABLE[NUM_YELLOW_LED] = { 60, 77, 90, 101, 113, 124, 136, 147, 164, 173, 185, 200 };  /**< Lookup table for the duty cycles */
const uint8_t EXTRA_LED_PINS[NUM_RED_LED] = { 2, 3, 4, 6, 7 };                                                /**< Assigned pins for the extra LEDs */

/* ADC constants */

/* Different ADC prescaler values - suggested not to go above 1MHz */
const unsigned char PS_16 = (1 << ADPS2);                  //@16 MHz -> 1MHz
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);   //@16 MHz -> 500khz
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);   //@16 MHz -> 250khz
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // @16MHz -> 125khz

/* FFT Data */
//int16_t capture[FFT_N];			/* Wave captureing buffer */
//complex_t bfly_buff[FFT_N];		/* FFT buffer */
//uint16_t spektrum[FFT_N/2];		/* Spectrum output buffer */

/* APP constants */
enum
{
    STATE_PREOPERATIONAL,
    STATE_OPERATIONAL
};

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
    if (level > 0)
        analogWrite(5, LED_DUTY_TABLE[level - 1]);
    else
        analogWrite(5, 0);

}

void set_level(uint8_t level)
{
  
  /* set the level at the first 12 LEDs (internally clips afer) */
  set_led(level);
  
  /* set the level if > LED_DRIVER_DOTS also to the red LEDs (internally clips after) */
  set_extra_led(level > NUM_YELLOW_LED ? level - NUM_YELLOW_LED : 0);
  
}

volatile int samples = 0;
uint16_t analogVal = 0;
int k;
/* ADC: conversion complete interrupt routine */
ISR(ADC_vect)
{

  if (samples == (FFT_N*2))
  {
    // wait for the app to process the data
    goto adc_return;
  }
  
  /* ADC channel that has completed the reading */
  //adcChan = ADMUX & ((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
  /* ADC input value (0 - 1024) */
  analogVal = ADCL | (ADCH << 8);

  k = analogVal; // form into an int
  k -= 0x0200; // form into a signed int
  k <<= 6; // form into a 16b signed int
  fft_input[samples] = k; // put real data into even bins
  fft_input[samples+1] = 0; // set odd bins to 0 

  samples += 2;

/* Yes, guys, back in the 80's the goto statement was in fashion ! */
adc_return:

  /* Set ADSC in ADCSRA (0x7A) to start another ADC conversion */
  ADCSRA |= (1 << ADSC);

}

void adc_setup()
{
    /* disable interrupts */
    cli();

    /* set up the ADC */
    ADCSRA &= ~PS_128;  /* remove bits set by Arduino library */

    /* set the prescaler */
    ADCSRA |= PS_64;

    /* clear ADLAR in ADMUX (0x7C) to right-adjust the result */
    /* ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits) */
    ADMUX &= B11011111;

    /* Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the internal 1.1V reference */
    ADMUX |= B11000000;

    /* Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog channel */
    ADMUX &= B11110000;

    /* Set MUX3..0 in ADMUX (0x7C) to read from AD0
       Do not set above 15! You will overrun other parts of ADMUX. A full
       list of possible inputs is available in Table 24-4 of the ATMega328
       datasheet
    */
    ADMUX |= 0; /* it will be dynamically changed later */

    // Set ADEN in ADCSRA (0x7A) to enable the ADC.
    // Note, this instruction takes 12 ADC clocks to execute
    ADCSRA |= B10000000;

    // un-set ADATE in ADCSRA (0x7A) to disable auto-triggering.
    ADCSRA &= ~B00100000;
 
    // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
    // This means that as soon as an ADC has finished, the next will be
    // immediately started.
    ADCSRB &= B11111000;

    // Set the Prescaler to 64 (16000KHz/64 = 250KHz (that is a sampling freq of ~19khz)
    ADCSRA |= PS_64;

    // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
    // Without this, the internal interrupt will not trigger.
    ADCSRA |= B00001000;

    /* re-enable ISR */
    sei();

}

// only for test (perhaps?, at least it shall be disabled if not needed)

void establishContact() {
 while (Serial.available() <= 0) {
      Serial.write('A');   // send a capital A
      delay(300);
  }
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
    
    /* ADC Setup */
    adc_setup();
    
    /* Kickstart the first conversion */
    ADCSRA |= (1 << ADSC);

}

        int16_t audioget_getspl(float voltnow, float voltref, float dbref) {
        	int16_t ret = (20 * log10(voltnow/voltref)) +  dbref;
	  return ret;
        }

/* APP: main loop */
void loop()
{

  static uint8_t app_state = STATE_PREOPERATIONAL;
  static int i = 0;

  switch(app_state)
  {

    case STATE_PREOPERATIONAL:

      /* At startup do a operational LED test */
      set_level(i);
      i++;
      delay(100);

      if (i > 17)
      {
          app_state = STATE_OPERATIONAL;
      }
      
      /* Serial */
        Serial.begin(115200);
//      establishContact();

      break;
    case STATE_OPERATIONAL:

      int k;
      int magnitud = 0;
      int16_t getval = 0;
      static double retval = 0;
      if (samples == (FFT_N*2))
      {

        fft_window(); // window the data for better frequency response
        fft_reorder(); // reorder the data before doing the fft
        fft_run(); // process the data in the fft
        //fft_mag_log(); // take the output of the fft
        fft_mag_lin8();
        /* reset samples and restart the process */

        for (byte i = 0; i < FFT_N/2; i++)
        {
            magnitud += fft_lin_out8[i];
        }

	magnitud = magnitud*2;
	getval = sqrt(magnitud);

	//appy rms offset
	getval += -2;

#define AUGIOGET_TWFALPHA 0.652
#define AUGIOGET_TWSALPHA 0.956
	//time-weight filter
//	#if AUGIOGET_TW == AUGIOGET_TWF
//		retval = AUGIOGET_TWFALPHA*retval+(1-AUGIOGET_TWFALPHA)*getval;
//	#elif AUGIOGET_TW == AUGIOGET_TWS
		retval = AUGIOGET_TWSALPHA*retval+(1-AUGIOGET_TWSALPHA)*getval;
//	#endif
          //Serial.write((uint8_t)((fft_lin_out[i] / 65534.0f) * 255.0f));
          //Serial.write(fft_lin_out8[i]);
        
//        Serial.println(samples, DEC);
        //set_led(spektrum[52]/4);
        
        // INPUT hardware setup ----------------
        //define voltage reference and spl db reference
        #define AUDIOGET_VOLTREF 4.7 //0.000315//0.000315
        #define AUDIOGET_DBREF 32
        
        uint16_t spl = audioget_getspl(retval, AUDIOGET_VOLTREF, AUDIOGET_DBREF);
Serial.println(retval, DEC);
Serial.println(spl, DEC);
        set_level(retval);

        samples = 0;

      }
    
      break;

  }
  
}
