
/*

    Sound Pressul Level Meter (MakerSpace!)
    
      Uses salvaged components and boards to create a sound pressure level meter to
      saveguard your hearing both in the industry (noisy tools) and enternainment (clubs)
      fields of application.

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

    History:

          2nd Sept 15; Version 0.1 - first revision, basic setup and program
          5th Nov 15;  Version 0.2 - improved sampling; improved calculations; github release!

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

/** FFT library set-up **/

/* Linear output, 16 bits width */
#define LIN_OUT 1
/* 64 FFT samples */
#define FFT_N 64

#include <FFT.h>

/*
 *
 * Concept
 * 
 * ADC input and sampling
 * 
 *  ADC subsystem is clocked at 1MHz in this program and this results in about 77kHz sampling.
 *  (16MHz/16-Prescaler/13us convertion time)
 *  Target frequency is 22050 kHz and to simplify the code a counter based on Arduino's micros() is done within
 *  the interrupt routine. Note that micros() has a resolution of 4us which means a theoretical bandwidth of more than
 *  250 kHz which is plenty to "contain" our target 22kHz bandwidth.
 * 
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

/* The constants below are kindly stolen from 

http://davidegironi.blogspot.it/2013/06/avr-atmega-audio-input-rma-using-fft.html

audiogetfft 0x04

copyright (c) Davide Gironi, 2013

Nice job!

*/

/*
define weighting gain table, must cointain AUDIOGET_SAMPLES/2 elements
define A-Weighting (Real and Imaginary part)
*/
#define AUDIOGET_WEIGHTINGTAR {0, 0.0363026, 0.513928541, 0.843129901, 1.023094514, 1.113238974, 1.150785316, 1.155376165, 1.13740991, 1.102630535, 1.05439208, 0.994794792, 0.925287976, 0.847016355, 0.76104051, 0.668494706, 0.570712944, 0.469336568, 0.366405293, 0.264424011, 0.16638812, 0.075740395, -0.003775804, -0.068404531, -0.114833072, -0.140808562, -0.145845064, -0.131862152, -0.103509898, -0.067907722, -0.033622556, -0.008963035}
#define AUDIOGET_WEIGHTINGTAI {0, 0.505227283, 0.673226268, 0.563062114, 0.390563392, 0.220544591, 0.066543122, -0.070793602, -0.193315087, -0.302824007, -0.40056018, -0.487184777, -0.562867671, -0.627380288, -0.680177014, -0.720469871, -0.747306718, -0.759664611, -0.756570026, -0.737256522, -0.701366932, -0.649199279, -0.581981015, -0.502132952, -0.413453151, -0.321118596, -0.231384828, -0.150886776, -0.085535271, -0.039169914, -0.012326333, -0.00159859}

/* Choose the desired weighing function (only A-Weighing available currently) */

#define AUDIOGET_WEIGHTINGTR AUDIOGET_WEIGHTINGTAR
#define AUDIOGET_WEIGHTINGTI AUDIOGET_WEIGHTINGTAI

/*
 * compute weighting (Davide Gironi)
 */
void audioget_doweighting(uint16_t *fft_input) {
	uint8_t i = 0;

	double weightingtabr[] = AUDIOGET_WEIGHTINGTR;
	double weightingtabi[] = AUDIOGET_WEIGHTINGTI;
	
	/* apply filter function to the real part first */
	for (i = 0; i < FFT_N/2; i += 2) {

            fft_input[i] *= weightingtabr[i];
            fft_input[FFT_N - (i + 1)] *= weightingtabr[i];

	}

        /* repeat for the imaginary part */
        for (i = 1; i < FFT_N/2; i += 2) {
          
          fft_input[i] *= weightingtabi[i];
          fft_input[FFT_N - (i + 1)] *= weightingtabi[i];
          
        }

}

/* APP constants */
enum
{
    STATE_PREOPERATIONAL,
    STATE_OPERATIONAL
};

void set_extra_led(uint8_t num)
{
    uint8_t i = 0;

    if (num > NUM_RED_LED)
    {
        /* Clip the value if bigger than the maximum allowed */
        num = NUM_RED_LED;
    }

    /* Turn all the LEDs off */
    for (i = 0; i < NUM_RED_LED; i++)
    {
        digitalWrite(EXTRA_LED_PINS[i], false);
    }

    /* Turn LEDs on when necessary */
    for (i = 0; i < num; i++)
    {
      digitalWrite(EXTRA_LED_PINS[i], true);
    }

}

void set_led(uint8_t level)
{

    /* set the correct duty cycle */
  
    if (level > NUM_YELLOW_LED - 1)
    {
        /* clip the level if above the max */
        level = NUM_YELLOW_LED;
    }

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

/* Counter for the samples in the interrupt routine - determines when it's time to compute SPL levels */
int samples = 0;

/* This buffer holds the spectrum with filtered values */
double spectrum[FFT_N / 2 + 1] = {0};  /* !you <<dirty>> initializer! */

uint16_t analogVal = 0;
uint32_t timestamp;
/* ADC: conversion complete interrupt routine */
ISR(ADC_vect)
{

  int k;  /* temporary variable */

  /* 45 --> sampling here happens at 77kHz, hence we need to slow the process down skiping 45us of samples which correspond to about 22kHz */
  if ((micros() - timestamp < 45) || (samples == FFT_N))
  {
      /* Update timestamp */
      timestamp = micros();
      /* wait for the app to process the data */
      goto adc_return;
      /* Kids, this is a GOTO statement! */
  }
  else
  {
      /* Go on! */
  }

  /* ADC input value (0 - 1023)
     Watch out! The reading sequence of the register is important!
     Please refer to the datasheet for more information about that.
   */
  k = ADCL | (ADCH << 8);

  k -= 512; /* form into a signed int (apply bias) */
  k <<= 6; /* form into a 16b signed int */
  fft_input[(samples * 2)] = k;       /* put real data into even bins */
  fft_input[(samples * 2) + 1] = 0;   /* set odd bins to 0 (imaginary part) */ 

  samples++;

/* Yes, guys, back in the 80's the goto statement was in fashion ! */
adc_return:

  /* Set ADSC in ADCSRA (0x7A) to start another ADC conversion */
  ADCSRA |= (1 << ADSC);

}

void io_setup()
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

void adc_setup()
{
    /* disable interrupts */
    cli();

    /* set up the ADC */
    ADCSRA &= ~PS_128;  /* remove bits set by Arduino library */

    /* clear ADLAR in ADMUX (0x7C) to right-adjust the result */
    /* ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits) */
    ADMUX &= B11011111;

    /* Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the internal 1.1V reference */
    ADMUX |= B11000000;

    /* Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog channel */
    ADMUX &= B11110000;

    /* Set MUX3..0 in ADMUX to read from AD0 */
    ADMUX |= 0;  // actually pretty much a NOP, kept just for reference...

    /* Set ADEN in ADCSRA (0x7A) to enable the ADC.
       Note, this instruction takes 12 ADC clocks to execute
     */
    ADCSRA |= (1 << ADEN);

    /* un-set ADATE in ADCSRA to disable auto-triggering */
    ADCSRA &= (1 << ADATE);
 
    /* Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
     This means that as soon as an ADC has finished, the next will be
     immediately started.
     */
    ADCSRB &= B11111000;

    // Set the Prescaler to 64 (16000KHz/64 = 250KHz (that is a sampling freq of ~19khz)
    ADCSRA |= PS_16;

    /* Set ADIE in ADCSRA to enable the ADC interrupt */
    ADCSRA |= (1 << ADIE);

    /* enable interrupts */
    sei();

}

/* APP: initialization of subsystems */
void setup()
{

    /* I/O pins setup */
    io_setup();

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

  /* Simple State Machine to keep application state */

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

      break;

    case STATE_OPERATIONAL:

      int k;
      double magnitud = 0;
//      int16_t getval = 0;
      static double retval = 0;
      if (samples == FFT_N)
      {

        /* window the data for better frequency response */
        fft_window();
        /* reorder the data before doing the fft */
        fft_reorder();
        /* process the data in the fft */
        fft_run();
        /* reset samples and restart the process */        
        fft_mag_lin();

        for (uint8_t i = 0; i < FFT_N/2; i++)
        {
            /* Construct the RMS values spectrum */
            spectrum[i] = 2 * sqrt(fft_lin_out[i]);
            magnitud += spectrum[i];
            /* 6 is log2 of 64 */
//            spectrum[i] = 2 * sqrt(((long)audioget_fr[i]*(long)audioget_fr[i] + (long)audioget_fi[i]*(long)audioget_fi[i]) >> (6 * 2));
        }

        /* Magnitude is now a value that is not reflecting the ADC sampled value, not a voltage value */
        if(magnitud == 0)
		magnitud = 0;
	else
// TODO defines for the 5V reference and the number of steps (1024)
		magnitud = (double)(magnitud * 5.0 / (double)1024);

//	magnitud = magnitud*2;
//	getval = sqrt(magnitud);

	//appy rms offset
	//getval += -2;

#define AUGIOGET_TWFALPHA 0.652
#define AUGIOGET_TWSALPHA 0.956
	/* time-weight filter (RC filter) */
	retval = AUGIOGET_TWSALPHA * retval + (1 - AUGIOGET_TWSALPHA) * magnitud;
        
        //define voltage reference and spl db reference
        #define AUDIOGET_VOLTREF 0.000315
        #define AUDIOGET_DBREF 32
        
        uint16_t spl = audioget_getspl(retval, AUDIOGET_VOLTREF, AUDIOGET_DBREF);
        set_level(9 - retval);

        cli();  /* disable interrupts */

        /* Shared variables with the interrupts shall be protected */

        /* Reset samples */
        samples = 0;
        timestamp = micros();

        sei();  /* enable interrupts */

      }
    
      break;

  }
  
}
