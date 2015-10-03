#define F_CPU 16000000
#include "stdbool.h"

/* avr libs */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* user libs */
#include "binary.h"
#include "string.h"
#include "stdio.h"

#define NOP() {__asm__ __volatile__ ("nop");}

#define CLK		PD0
#define DTA		PD1
#define KEY             PD2

uint8_t segments;
uint8_t cathodes;
uint8_t not_defined;
uint8_t i;

#define TIMER_0_PRESCALER_8     (1 << CS01)
#define TIMER_0_PRESCALER_64    ((1 << CS01) | (1 << CS00))
#define TIMER_0_PRESCALER_256   (1 << CS02)

enum e_key
{

    K_NONE,
    K_CH_PLUS,
    K_POWER,
    K_CH_MINUS,

};

/*
    f__
  e |  | a
    |  |
    -g_-
  d |  | b
    |__|
      c
*/

/* NOTICE: not every caracter is filled or correct
 * Please do the corrections on demand
 */

uint8_t alphabet[] =
{

    /* CAPITAL */
    B01110111, //A
    B01111111, //B
    B00111100, //C
    B00111111, //D
    B10011110, //E
    B10001110, //F
    B01111101, //G
    B01101110, //H
    B00000110, //I
    B00001111, //J
    B01011000, //K
    B00111000, //L
    B00110111, //M
    B00000111, //N
    B00111111, //O
    B01110011, //P
    B11111100, //Q
    B11001110, //R
    B01101101, //S
    B00000111, //T
    B00111110, //U
    B00111110, //V
    B01111110, //W
    B00000000, //X
    B00000000, //Y
    B00000000, //Z

    /* special chars gap */
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,

    /* small */

    B01110111, //A
    B01111100, //b
    B00011010, //c
    B01111010, //d
    B10011110, //E
    B10001110, //F
    B01111101, //G
    B00101110, //h
    B00000110, //I
    B00001111, //J
    B01011000, //K
    B00011000, //l
    B00011110, //M
    B00000111, //N
    B00111010, //o
    B11001110, //P
    B11111100, //Q
    B11001110, //R
    B10110110, //S
    B00000000, //T
    B00111110, //U
    B00000000, //V
    B00000000, //W
    B00000000, //X
    B00000000, //Y
    B00000000, //Z

    B00000000, //shows nothing
};

uint8_t numeral[]= {
    B00111111, //0
    B00000110, //1
    B01011011, //2
    B01001111, //3
    B01100110, //4
    B01101101, //5
    B01111101, //6
    B00000111, //7
    B01111111, //8
    B01100111, //9
    B00000000, //shows nothing
};
/*
static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

uint8_t reverse(uint8_t n) {
   // Reverse the top and bottom nibble then swap them.
   return (lookup[n&0b1111] << 4) | lookup[n>>4];
}
*/
// global variable to count the number of overflows
volatile uint8_t tot_overflow;

/** Utilities **/

typedef struct
{
    bool     input_old;
    uint32_t rising_timeout;
    uint32_t falling_timeout;
    uint32_t timestamp;

} t_debounce;

static uint8_t buffer[4];   /**< display buffer */

uint8_t const keypad_shift[4] = { 1 << 0, 1 << 4, 1 << 7, 0 };    /* CH+ ; POWER ; CH- ; mock */

/** Keypad state */
typedef struct
{

    bool keypad[4];                 /**< keypad input state */
    t_debounce keypad_debounce[4];  /**< keypad debounce state */
    bool keypad_latch[4];           /**< keypad latch state */

    enum e_key pressed;

} t_keypad;

enum e_menu
{

    MENU_VOLTAGE = 0,
    MENU_CURRENT,
    MENU_POWER,
    MENU_LOAD_RESISTANCE,

    MENU_NUM,

};

typedef struct
{

    uint32_t timestamp;

    t_keypad keypad;
    enum e_menu menu;

} t_operational;

t_operational g_operational;

// initialize timer, interrupt and variable

void timer0_init()
{
    // set up timer with prescaler
    TCCR0A = (1 << 1);
    TCCR0B = TIMER_0_PRESCALER_8;

    // initialize counter
    TCNT0 = 0;

    // enable overflow interrupt
    TIMSK0 |= (1 << OCIE0A);

    TIFR0 = (1 << TOV0) | (1 << OCF0A);

    // 4 cycles at 16mhz/8
    // tops at OCRA
    OCR0A = 200;

    // enable global interrupts
    sei();

    // initialize overflow counter variable
    tot_overflow = 0;
}

//TIMER0 overflow interrupt service routine
// called whenever TCNT0 overflows
ISR(TIMER0_OVF_vect)
{
    // keep a track of number of overflows
    tot_overflow++;
}

ISR(TIMER0_COMPA_vect)
{
    g_operational.timestamp += 100;   /* 2us */
}

bool debounce(t_debounce *debounce, bool input)
{
    if (debounce == NULL)
    {
        /* Sorry (and screw you as well), NULL pointer! */
        return input;
    }
    else
    {
        if (input == true && input != debounce->input_old)
        {
            /* input rising edge detected */
            if (debounce->timestamp == 0U)
            {
               /* input has changed. Store the current timestamp. */
                debounce->timestamp = g_operational.timestamp;
            }
            else
            {
                /* already running */
            }

            if (g_operational.timestamp - debounce->timestamp >= debounce->rising_timeout)
            {
                return input;
            }
            else
            {
                /* no timeout yet */
            }

        }
        else if (input == false && input != debounce->input_old)
        {
            /* input falling edge detected */
            if (debounce->timestamp == 0U)
            {
               /* input has changed. Store the current timestamp. */
                debounce->timestamp = g_operational.timestamp;
            }
            else
            {
                /* already running */
            }

            if (g_operational.timestamp - debounce->timestamp >= debounce->rising_timeout)
            {
                return input;
            }
            else
            {
                /* no timeout yet */
            }
        }
        else
        {
            debounce->timestamp = 0U;
            return debounce->input_old;
        }

    }

    return debounce->input_old;

}

void write(uint8_t segments, uint8_t cathodes)
{
    uint8_t not_defined = 0;
    uint8_t id = 0;
    uint8_t *tmp = &cathodes;

    PORTD &= ~(1 << CLK);	/* LOW */
    PORTD |= (1 << DTA);    /* HIGH */

    for (i = 0; i < 18; i++)
    {
			if (i == 4)
			{
				tmp = &not_defined;
				id = 0;
			}
			else if (i == 10)
			{
				tmp = &segments;
				id = 0;
			}
			else
			{
			}

			/* set data bit */
			if (((*tmp) >> id) & 0x01)
				PORTD |= (1 << DTA);
			else
				PORTD &= ~(1 << DTA);

			PORTD &= ~(1 << CLK);
            PORTD |= (1 << CLK);

            id++;
		}

}



void write_string(uint8_t letters[4])
{

    uint8_t i = 0;

    for (i = 0; i < 4; i++)
    {
        write(letters[i], 0xF & ~(1<<i));
        _delay_us(500);
        write(keypad_shift[i], 0xFF);
        _delay_us(10);
        g_operational.keypad.keypad[i] = (PIND >> KEY) & 0x1;
        _delay_us(500);
    }

}

void d_print(char* s)
{

    uint8_t i = 0;

    while (*s != 0)
    {
      if ((*s == ',') || (*s == '.'))
      {
          /* separator character:
             - don't add the separator to the buffer
             - add the needed bit to the previous character
           */
          buffer[1] |= 1 << 7;
          s++;
      }
      else if (*s < 48)
      {
          /* leave blank */
          buffer[i] = 0;
          s++;
          i++;
      }
      else if (*s >= 65)
      {
          /* ASCII letter */
          buffer[i++] = alphabet[*(s++) - 65U];
      }
      else
      {
          /* ASCII number */
          buffer[i++] = numeral[*(s++) - 48U];
      }
    }
}
/*
void PWM_Duty_Change()
   {

	//Local variables for PID
	const float Kp = 0.01;		// The value for Proportional gain
	const float Ki = 0.01;		// The value for Integral gain
	const float Kd = 0.0001;	// The value for Differential gain

	int Set_Point = 353;	// The ADC reference point we are aiming to regulate to
	int iMax = 100;			// Used to prevent integral wind-up
	int iMin = -100;		// Used to prevent integral wind-up
	int Err_Value;			// Holds the calculated Error value
	int P_Term;			// Holds the calculated Proportional value
	int I_Term;			// Holds the calculated Integral value
	int D_Term;			// Holds the calculated Differential value
	int new_ADC_value;		// Holds the new ADC value
	int PWM_Duty;			// Holds the new PWM value

	// More efficient to read this once and store as used 3 times
	new_ADC_value = read_ADC();

	Err_Value = (Set_Point - new_ADC_value);

	// This calculates Proportional value, Kp is multiplied with Err_Value and the result is assigned to P_Term
	P_Term = Kp * Err_Value;

	// Prepare Integral value, add the current error value to the integral value and assign the total to i_Temp
	i_Temp += Err_Value;

	// Prevents integral wind-up, limits i_Temp from getting too positive or negative
	if (i_Temp > iMax)
	{i_Temp = iMax;}
	else if (i_Temp < iMin)
	{i_Temp = iMin;}

	// Calculates the Integral value, Ki is multiplied with i_Temp and the result is assigned to I_Term
	I_Term = Ki * i_Temp;

	// Calculates Differential value, Kd is multiplied with (d_Temp minus new_ADC_value) and the result is assigned to D_Term
	// The new_ADC_value will become the old ADC value on the next function call, this is assigned to d_Temp so it can be used
	D_Term = Kd * (d_Temp - Err_Value);
	d_Temp = Err_Value;

	// Now we have the P_Term, I_Term and D_Term
	PWM_Duty = PWM_Temp - (P_Term + I_Term + D_Term);

	// PWM overflow prevention
	if (PWM_Duty > 300)
	{PWM_Duty = 300;}
	else if (PWM_Duty < 30)
	{PWM_Duty = 30;}

	// Adjusts the PWM duty cycle
	adjust_PWM(PWM_Duty);
	// Assigns the current PWM duty cycle value to PWM_Temp
	PWM_Temp = PWM_Duty;
   }
   */

/*

1 . AVVIO (se ci saranno calibrazioni o cosi via)
     *display: "BOOT"

2 . MISURA (ADC0 -> voltage; ADC1 -> current)

3 . LIMITAZIONE I

4 . LIMITAZIONE V (io comando il pot analogico ma se per sbaglio esagero tengo una limitazione software!)

*/

void keypad_init()
{

    uint8_t i = 0;

    /* parse keypad state */
    for (i = 0; i < 4; i++)
    {
        /* set a 50 msec rising / falling debounce */
        g_operational.keypad.keypad_debounce[i].falling_timeout = 20000;
        g_operational.keypad.keypad_debounce[i].rising_timeout = 20000;
    }

}

#define SCROLL_WAIT     1500000

bool gui_show_msg(char* string, char* buffer, uint8_t len, bool trigger)
{

    static char* s = NULL;
    static uint8_t ts = 0;

    if (s == NULL && trigger == true)
    {
        /* start display a new message */
        ts = g_operational.timestamp;
        s = string;
    }
    else if (s == NULL && trigger == false)
    {
        /* idle */
        NOP();
    }
    else
    {

        memcpy(buffer, s, (s - string) < 4 ? (s - string) : 4);
        buffer[4] = '\0';

        /* continue display message */
        if (g_operational.timestamp - ts > SCROLL_WAIT)
        {
            /* time passed, continue scrolling */
            ts = g_operational.timestamp;
            s++;
            if ((string - s) > len)
            {
                s = NULL;
            }
        }
        else
        {
            /* time not passed, wait */
        }
    }

    return (s != NULL);

}

char string_buffer[5];
char status_buffer[5];
char* display_string = NULL;
void gui_menu(enum e_key key)
{
//    static bool message = false;
//    static bool monitor = false;
    bool trigger = false;
//    bool msg_show = false;

    switch(g_operational.menu)
    {
        case MENU_VOLTAGE:
            sprintf(status_buffer, "VOLT");
            break;
        case MENU_CURRENT:
            sprintf(status_buffer, "AMPS");
            break;
        case MENU_POWER:
            sprintf(status_buffer, "POWA");
            break;
        case MENU_LOAD_RESISTANCE:
            sprintf(status_buffer, "LOAD");
            break;
        default:
            break;
    }

    if (key == K_CH_PLUS)
    {
        trigger = true;
    }
    if (key == K_POWER)
    {
        g_operational.menu = (g_operational.menu + 1) % MENU_NUM;
    }

    /* handling of buttons done */
    g_operational.keypad.pressed = K_NONE;

    display_string = status_buffer;

//    msg_show = gui_show_msg("12345678", string_buffer, 8, trigger);

//    if (msg_show == false)
//    {
//        memcpy(string_buffer, status_buffer, 5);
//    }

    if (display_string != NULL)
    {
        /* better safe than sorry */
        display_string[5] = '\0';

        /* fill the display buffer */
        d_print(display_string);

        /* write display */
        write_string(buffer);
    }

}

int main(void)
{

    /* I/O init */

	DDRD |= 1<<DTA;  /* OUTPUT */
	DDRD |= 1<<CLK;  /* OUTPUT */
    DDRD &= ~(1<<KEY);  /* INPUT */

    /* TIMER init */
    timer0_init();

	/* KEYPAD */
    keypad_init();


char st[5];
int sec = 0;
bool tmp;
uint8_t i = 0;

        while(1)
        {

            /* parse keypad state */
            for (i = 0; i < 4; i++)
            {
                tmp = debounce(&g_operational.keypad.keypad_debounce[i], g_operational.keypad.keypad[i]);
                if (tmp == true && g_operational.keypad.keypad_latch[i] == false)
                {
                    switch(i)
                    {
                    case 0:
                        g_operational.keypad.pressed = K_CH_PLUS;
                        sec++;
                        break;
                    case 1:
                        g_operational.keypad.pressed = K_POWER;
                        sec++;
                        break;
                    case 2:
                        g_operational.keypad.pressed = K_CH_MINUS;
                        sec++;
                        break;
                    default:
                        g_operational.keypad.pressed = K_NONE;
                        break;
                    }

                    g_operational.keypad.keypad_latch[i] = true;

                }
                else
                {
                    if (tmp == false)
                    {
                        g_operational.keypad.keypad_latch[i] = false;
                    }
                }
            }

            gui_menu(g_operational.keypad.pressed);

        }

}
