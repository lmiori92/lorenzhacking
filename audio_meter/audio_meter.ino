/*

    VU-Meter like (it is more a PPM meter now) with VFD display salvaged from
    an old and broken office calculator.

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


    Copyright (C) Lorenzo Miori, 27th July 2015

    History:

          Version 0.1 - start!

*/

#include <math.h>

#define CHANNELS      2     /**< total number of channels (and so CS) */
#define DIGITS        10    /**< total number of used digits */
#define SAMPLE_COUNT  256   /**< number of ADC readings before computing the peak value */

/* Different ADC prescaler values - suggested not to go above 1MHz */
const unsigned char PS_16 = (1 << ADPS2);                  //@16 MHz -> 1MHz
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);   //@16 MHz -> 500khz
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);   //@16 MHz -> 250khz
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // @16MHz -> 125khz

/* APP: application states */
enum
{
  STATE_INIT,
  STATE_PREOPERATIONAL,
  STATE_OPERATIONAL,
  STATE_POSTOPERATIONAL
};

/* DISPLAY DRIVER: Mapping to display pins and levels */
const int LEVEL_PINS[DIGITS] = { A5, A4, 9, 10, 11, 12, 13, A3, A2, 8 };
/* DISPLAY DRIVER: Mapping to line select pins */
const int CS_PINS[CHANNELS] = { 2, 7 };
/* DISPLAY DRIVER: current active line */
uint8_t displayCh = 0;
/* DISPLAY DRIVER: buffer */
bool displayBuffer[2][10] = { { 1, 1, 0, 1, 1, 0, 1, 1, 1, 1 }, { 0, 1, 1, 1, 1, 0, 1, 0, 1, 0 } };

/* ADC: offset (bias) */
const float inputOffset = 512;

/* DISPLAY: dB scale */
const int dbPerLed = 3;

/* APP: current application state */
int appState = STATE_INIT;

/* PPM: structure to hold sampling information */
typedef struct
{
    uint16_t    sampleCount;
    uint16_t    maxAudio;
    bool        audioSampled;
    float       dBAudio;
    
} t_ppm_ch;

/* PPM: sampling status for both channels */
t_ppm_ch ppmState[CHANNELS];

/* ADC: conversion complete interrupt routine */
ISR(ADC_vect)
{

  uint8_t adcChan;
  uint16_t analogVal = 0;
  
  /* ADC channel that has completed the reading */
  adcChan = ADMUX & ((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
  /* ADC input value (0 - 1024) */
  analogVal = ADCL | (ADCH << 8);

  /* increment sample count */
  ppmState[adcChan].sampleCount++;

  /* save the peak value */
  if (analogVal > ppmState[adcChan].maxAudio)
  {
      ppmState[adcChan].maxAudio = analogVal;
  }

  /* keep a certain number of readings */
  if (ppmState[adcChan].sampleCount > 512)
  {
      ppmState[adcChan].dBAudio = 20 * log10(abs(ppmState[adcChan].maxAudio - inputOffset) / inputOffset);
      ppmState[adcChan].audioSampled = true;
      
      /* Reset values for the next round */
      ppmState[adcChan].maxAudio = 0;
      ppmState[adcChan].sampleCount = 0;
  }
  
  /* Toggle the other channel */
  ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
  ADMUX |= (adcChan == ADCH0) ? ADCH1 : ADCH0;

  /* Set ADSC in ADCSRA (0x7A) to start another ADC conversion */
  ADCSRA |= (1 << ADSC);

}

/* DISPLAY DRIVER: Timer0 comparator A triggered (250Hz) */
ISR(TIMER0_COMPA_vect)
{

    uint8_t i = 0;

    /* disable both lines (channels) */
    digitalWrite(2, HIGH);
    digitalWrite(7, HIGH);

    /* set digit outputs */
    for (i = 0; i < DIGITS; i++)
        digitalWrite(LEVEL_PINS[i], displayBuffer[displayCh][i]);
  
    /* drive the current channel */
    if (displayCh == 0)
    {
        digitalWrite(2, LOW);
        digitalWrite(7, HIGH);
    }
    else
    {
        digitalWrite(2, HIGH);
        digitalWrite(7, LOW);
    }  

    /* toggle channel for the next interrupt */
    displayCh ^= 1;

}

/* PPM task: update display buffer based on sampling value */
void ppm_task()
{

    uint8_t ch = 0;
    uint8_t i = 0;

    for (ch = 0; ch < 2; ch++)
    {
        /* audio sampled for the channel */
        if (ppmState[ch].audioSampled == true)
        {

            for (i = 0; i < DIGITS; i++)
            {
                if (abs(ppmState[ch].dBAudio) <= (i * dbPerLed) + 0.1 )
                {
                    displayBuffer[ch][(DIGITS - 1) - i] = 1;
                }
                else
                {
                    displayBuffer[ch][(DIGITS - 1) - i] = 0;
                }
            }
        }
    }

}

/* APP: initialization of subsystems */
void setup()
{

    uint8_t i;

    for (i = 0; i < CHANNELS; i++)
    {
        pinMode(CS_PINS[i], OUTPUT);
        digitalWrite(CS_PINS[i], HIGH);
    }

    for (i = 0; i < DIGITS; i++)
    {
      pinMode(LEVEL_PINS[i], OUTPUT);
      digitalWrite(LEVEL_PINS[i], LOW);
    }

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

    // Set the Prescaler to 64 (16000KHz/64 = 250KHz)
    ADCSRA |= PS_64;

    // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
    // Without this, the internal interrupt will not trigger.
    ADCSRA |= B00001000;

    /* set timer0 to interrupt at 250Hz */

    TCCR0A = 0;    /* clear register */
    TCCR0B = 0;    /* clear register */
    TCNT0  = 0;    /* initialize counter value to 0 */
    OCR0A = 249;   /* = (16000000) / (250Hz * 256prescaler) - 1 */
    TCCR0A |= (1 << WGM01);    /* turn on CTC mode */
    TCCR0B |= (1 << CS02);     /* Set CS01 and CS00 bits for 256 prescaler */
    TIMSK0 |= (1 << OCIE0A);   /* enable timer compare interrupt */

    /* enable global interrupts */
    sei();

}

/* Set all level segments to OFF */
void display_clear()
{
    int i = 0;

    for (i = 0; i < DIGITS; i++)
    {
        displayBuffer[0][i] = 0;
        displayBuffer[1][i] = 0;
    }
}

/* Single level indication test */
void test_0()
{
    uint8_t i = 0;
    uint8_t j = 0;

    display_clear();

    for (j = 0; j < CHANNELS; j++)
    {
        for (i = 0; i < DIGITS; i++)
        {
            displayBuffer[j][i] = 1;
            delay(250);
            displayBuffer[j][i] = 0;
            delay(250);
        }
    }
}

/* Screen fill test */
void test_1()
{
    uint8_t i = 0;
    uint8_t j = 0;

    display_clear();

    for (j = 0; j < CHANNELS; j++)
    {
        for (i = 0; i < DIGITS; i++)
        {
            displayBuffer[j][i] = 1;
            delay(250);
        }
    }
}

/* Start the ADC sampling (interrupt routine) */
void adc_start()
{
    /* Set ADSC in ADCSRA (0x7A) to start the ADC conversion */
    ADCSRA |= B01000000;
}

/* APP: main loop */
void loop()
{

    switch(appState)
    {
        case STATE_INIT:

            appState = STATE_PREOPERATIONAL;
            break;
            
        case STATE_PREOPERATIONAL:

            /* test routines */
            test_0();
            test_1();

            /* Start sampling after initial tests */
            adc_start();

            appState = STATE_OPERATIONAL;
            
            break;
            
        case STATE_OPERATIONAL:

            ppm_task();

            break;
            
        case STATE_POSTOPERATIONAL:

            break;
            
        default:

            break;
    }

}
