/* MENU */
typedef struct
{
    char*   label;
} t_menu_entry;

t_menu_entry SOURCE_MENU[] = {
                            { .label = "AUX"      },
                            { .label = "CD"       },
                            { .label = "RADIO"    },
                            { .label = "TAPE"     },
                           };

t_menu_entry SETTINGS_MENU[] = {
                            { .label = "Sources"  },
                            { .label = "Display"  },
                            { .label = "FFT"    },
                            { .label = "Vu-Meter"     },
                           };

typedef struct
{
    uint8_t index;
    uint8_t elements;
    uint8_t page;
} t_menu;

t_menu menu;

#define DEBOUNCE_BUTTONS   50
#define NUM_BUTTONS        3

typedef struct
{
    uint8_t  pins[NUM_BUTTONS];
    bool     buttons[NUM_BUTTONS];
    int      debounce[NUM_BUTTONS];
    bool     latches[NUM_BUTTONS];
} t_keypad;

t_keypad keypad;

/* Chip address */
#define ADDRESS     B11100110

/* Modes */
#define NO_MDATA_NOR_ADATA      0x0
#define ADATA_ONLY              0x1
#define MDATA_ONLY              0x2
#define MDATA_AND_ADATA         0x3

/* Hardware pins */

#define CE                      2   /* Chip Enable */
#define DI                      3   /* Data Input */
#define CL                      4   /* Clock */

/**
    This function writes the serial data (low-level) to the chip.
    The datasheet specifies about 0.5us delay between the edges of inputs.
    In any case, it has been tested that delays are not needed between cals,
    and this might be due to the "slow" access to pins that the Arduino library operates.
    This note is left for a future bare-metal implementation.
*/
void lc75710_write(uint32_t data)
{

    uint8_t i = 0;

    /* Address goes out first... */
    digitalWrite(CE, LOW);

    for (i = 0; i < 8; i++)
    {
        digitalWrite(CL, LOW);
/*        delayMicroseconds(1);*/
        digitalWrite(DI, (ADDRESS >> 7-i) & 0x1);
//        delayMicroseconds(1);*/
        digitalWrite(CL, HIGH);
    }
    digitalWrite(CL, LOW);

//    delayMicroseconds(1);*/

    /* Then data follows after, CE goes high */
    digitalWrite(CE, HIGH);
//    delayMicroseconds(1);

    for (i = 0; i < 24; i++)
    {
        digitalWrite(CL, LOW);
//        delayMicroseconds(1);*/
        digitalWrite(DI, (data >> i) & 0x1);
//        delayMicroseconds(1);*/
        digitalWrite(CL, HIGH);
    }

//    delayMicroseconds(1);*/
    digitalWrite(CE, LOW);

}




void lc75710_write_56bits(uint64_t data)
{

    uint8_t i = 0;

    /* Address goes out first... */
    digitalWrite(CE, LOW);
    for (i = 0; i < 8; i++)
    {
        digitalWrite(CL, LOW);
        digitalWrite(DI, (ADDRESS >> 7-i) & 0x1);
        digitalWrite(CL, HIGH);
    }
    digitalWrite(CL, LOW);

    /* Then data follows after, CE goes high */
    digitalWrite(CE, HIGH);

    for (i = 0; i < 56; i++)
    {
        digitalWrite(CL, LOW);
        digitalWrite(DI, (data >> i) & 0x1);
        digitalWrite(CL, HIGH);
    }

    digitalWrite(CE, LOW);

}



void lc75710_blink(uint8_t operation, uint8_t period, uint16_t digits)
{
    
    uint32_t temp = 0;

    /* Instruction */
    temp = (uint32_t)0x5 << 21;

    /* Data that specifies the blinking operation */
    temp = temp | ((uint32_t)(operation & 0x3) << 19);

    /* Blink period setting */
    temp |= (uint32_t)(period & 0x7) << 16;

    /*  Blinking digit specification */
    temp |= digits;

    /* Write to IC */
    lc75710_write(temp);

}

void lc75710_on_off(uint8_t operation, bool mode, uint16_t grids)
{

    uint32_t temp = 0;

    /* Instruction */
    temp  = (uint32_t)0x1 << 20;

    /* Specifies the data to be turned on or off */
    temp |= (uint32_t)(operation & 0x3) << 17;

    /* Toggle */
    temp |= (uint32_t)mode << 16;
    
    /* Grid selection */
    temp |= grids;
    
    /* Write to IC */
    lc75710_write(temp);

}


void lc75710_shift(uint8_t operation, bool direction)
{

    uint32_t temp = 0;

    /* Instruction */
    temp  = (uint32_t)0x2 << 20;

    /* Specifies the data to be shifted */
    temp |= (uint32_t)(operation & 0x3) << 17;

    /* Direction */
    temp |= (uint32_t)direction << 16;

    /* Write to IC */
    lc75710_write(temp);

}

void lc75710_grid_register_load(uint8_t grids)
{
    
    uint32_t temp = 0;

    /* Instruction */
    temp  = (uint32_t)0x3 << 20;

    /* Specifies the data to be shifted */
    temp |= (uint32_t)(grids & 0xF) << 16;

    /* Write to IC */
    lc75710_write(temp);
    
}

void lc75710_set_ac_address(uint8_t dcram, uint8_t adram)
{

    uint32_t temp = 0;

    /* Instruction */
    temp  = (uint32_t)0x4 << 20;

    /* ADRAM address */
    temp |= (uint32_t)(adram & 0xF) << 16;

    /* DCRAM address */
    temp |= (uint32_t)(dcram & 0x3F) << 8;

    /* Write to IC */
    lc75710_write(temp);
    
}

void lc75710_intensity(uint8_t intensity)
{

    uint32_t temp = 0;

    /* Instruction */
    temp  = (uint32_t)0x5 << 20;

    /* ADRAM address */
    temp |= (uint32_t)(intensity & 0xFF) << 8;

    /* Write to IC */
    lc75710_write(temp);
    
}

void lc75710_dcram_write(uint8_t addr, uint8_t data)
{

    uint32_t temp = 0;

    /* Instruction */
    temp  = (uint32_t)0x6 << 20;

    /* DCRAM address */
    temp |= (uint32_t)(addr & 0x3F) << 8;

    /* ADRAM address */
    temp |= (uint32_t)(data & 0xFF) << 0;

    /* Write to IC */
    lc75710_write(temp);

}

void lc75710_adram_write(uint8_t addr, uint8_t data)
{

    uint32_t temp = 0;

    /* Instruction */
    temp  = (uint32_t)0x7 << 20;

    /* DCRAM address */
    temp |= (uint32_t)(addr & 0xF) << 16;

    /* ADRAM address */
    temp |= (uint32_t)(data & 0xFF) << 8;

    /* Write to IC */
    lc75710_write(temp);

}

void lc75710_cgram_write(uint8_t addr, uint64_t data)
{

    uint64_t temp = 0;

    /* Instruction */
    temp  = (uint64_t)0x8 << 52;

    /* CGRAM address */
    temp |= (uint64_t)(addr & 0xFF) << 40;

    /* ADRAM address */
    temp |= data;

    /* Write to IC */
    lc75710_write_56bits(temp);

}

void lc75710_init()
{

    /* Pin configuration */
    pinMode(CL, OUTPUT);
    pinMode(DI, OUTPUT);
    pinMode(CE, OUTPUT);
    digitalWrite(CL, LOW);
    digitalWrite(DI, LOW);
    digitalWrite(CE, LOW);

    /* After powerup the display shall be initialized,
       otherwise registers may contain garbage data */

    /* No blink enabled by default */
    lc75710_blink(MDATA_AND_ADATA, 0, 0xFFFF);
    lc75710_dcram_write(0, 0x20);
    lc75710_dcram_write(1, 0x20);
    lc75710_dcram_write(2, 0x4F);
    lc75710_dcram_write(3, 0x59);
    lc75710_dcram_write(4, 0x4E);
    lc75710_dcram_write(5, 0x41);
    lc75710_dcram_write(6, 0x53);
    lc75710_dcram_write(7, 0x20);
    lc75710_dcram_write(8, 0x20);
    lc75710_dcram_write(9, 0x20);
    lc75710_dcram_write(10, 0x20);
    lc75710_set_ac_address(0, 0);
    lc75710_grid_register_load(10);
    lc75710_intensity(200);

    /* Turn the display ON */
    lc75710_on_off(MDATA_AND_ADATA, true, 0xFFFF);
    display_clear(64);
    display_string("Hacked!");
//    lc75710_cgram_write(0, 0x3FFFFF);
//display_load_bars_in_ram();
//display_load_vumeter_bars_in_ram();
display_load_vertical_bars_in_ram();
    lc75710_dcram_write(5, 3);
//lc75710_dcram_write(0, B01111111);
lc75710_dcram_write(9, B01111111);
}

// wops was already included :)
//int strlen(const char* string)
//{
//    uint8_t len = 0;
//    char* str = string;
//    while (*(str++) != 0) len++;
//    return len;
//}

void display_string(char* string)
{
    uint8_t len = 0;
    len = strlen(string);
    display_string_len(string, len);
}

void display_string_len(char* string, uint8_t len)
{
  uint8_t addr = 0;
  lc75710_set_ac_address(0, 0);
  do
  {
      lc75710_dcram_write(len - 1 - addr++, *string);
  }
  while (*(++string) != '\0');
  
}

void display_string_center(char* string)
{
  uint8_t addr = 0;
  uint8_t len  = 0;
  len = strlen(string);
  len += (10 - len) / 2;
  lc75710_set_ac_address(0, 0);
  do
  {
      lc75710_dcram_write(len - 1 - addr++, *string);
  }
  while (*(++string) != '\0');
}

void display_clear(uint8_t len)
{
    lc75710_set_ac_address(0, 0);
    do
    {
        /* Fill with spaces */
        lc75710_dcram_write(len, 0x20);
    }
    while (len-- != 0);
}

/* TODO refactor these functions below */

/* Load vertical bars in the CGRAM of the chip */
void display_load_bars_in_ram()
{
    
    uint8_t i = 0;
    uint64_t c = 0;
    
    for (i = 0; i < 7; i++)
    {
        c |= (uint64_t)0x1F << (30 - (i*5));
        lc75710_cgram_write(i, c);
    }
    
}

/* Load horizontal bars in the CGRAM of the chip */
void display_load_vertical_bars_in_ram()
{
    
    uint8_t i = 0;
    uint64_t c = 0;
    
    for (i = 0; i < 7; i++)
    {
        c |= (uint64_t)((uint64_t)0x1 << 4-i);
        c |= (uint64_t)((uint64_t)0x1 << (9-i));
        c |= (uint64_t)((uint64_t)0x1 << (14-i));
        c |= (uint64_t)((uint64_t)0x1 << (19-i));
        c |= (uint64_t)((uint64_t)0x1 << (24-i));
        c |= (uint64_t)((uint64_t)0x1 << (29-i));
        c |= (uint64_t)((uint64_t)0x1 << (34-i));
        lc75710_cgram_write(i, c);
    }
    
}

void display_load_vumeter_bars_in_ram()
{
    
    uint8_t i = 0;
    uint64_t c = 0;
    
    c = (uint64_t) 0xCC3;
    for (i = 0; i < 7; i++)
    {
        lc75710_cgram_write(i, c);//(uint64_t)0x7FFFFFFFFL << (64-35)) << (i * 5));
    }
    
}

void display_show_horizontal_bar(uint8_t level)
{
    
    uint8_t i = 0;

    display_clear(10);
    lc75710_set_ac_address(0, 0);

    /* Full bars */
    for (i = 0; i < level / 5; i++)
    {
        lc75710_dcram_write(i, 4);
    }

    lc75710_dcram_write(i, level%5-1);

}


void display_show_bars(uint8_t bar, uint8_t level)
{
    lc75710_dcram_write(bar%11, level%7);
}

void setup()
{

  uint8_t i = 0;
  
  Serial.begin(9600);
  lc75710_init();
  
  /* Init keypad */
  keypad.pins[0] = 7;
  keypad.pins[1] = 8;
  keypad.pins[2] = 9;
  
  for (i = 0; i < NUM_BUTTONS; i++)
  {
      pinMode(keypad.pins[i], OUTPUT);
      keypad.buttons[i]  = false;
      keypad.latches[i]  = false;
      keypad.debounce[i] = millis();
  }
  
}

/* Read the keypad, apply debounce to inputs and detect the rising edge */
void keypad_read()
{

  uint8_t i = 0;
  bool t = false;

  for (i = 0; i < NUM_BUTTONS; i++)
  {
      t = digitalRead(keypad.pins[i]);
      
      if (t == true)
      {
        t = false;
        if (keypad.debounce[i] == 0)
          keypad.debounce[i] = millis();
        else
          if (millis() - keypad.debounce[i] > DEBOUNCE_BUTTONS)
            t = true;            
      }
      else
      {
//          keypad.debounce[i] = 0;
          t = false;
      }
      
      if (t == true && keypad.latches[i] == false)
      {
          keypad.buttons[i] = true;
      }
      else
      {
          keypad.buttons[i] = false;
      }
      keypad.latches[i] = t;
  }

}

/* Manage the Audio Source selection menu */
void menu_page_sources()
{
    menu.elements = sizeof(SOURCE_MENU) / sizeof(t_menu_entry);

    if (keypad.buttons[0])
    {
          menu.index = ++menu.index % menu.elements;
          display_clear(10);
          display_string_center(SOURCE_MENU[menu.index].label);
    }
    else if (keypad.buttons[1])
    {
        if (menu.index > 0)
            menu.index--;
        else
            menu.index = menu.elements - 1;
        display_clear(10);
        display_string_center(SOURCE_MENU[menu.index].label);
    }
}

void loop()
{
  char in;
  static int i = 0;
  i++;
//  delay(5);
//  display_show_horizontal_bar(i%51);
//lc75710_dcram_write(0, 0);
/*
    lc75710_dcram_write(0, i%1);
  lc75710_dcram_write(1, i%2);
    lc75710_dcram_write(2, i%3);
      lc75710_dcram_write(3, i%4);
        lc75710_dcram_write(4, i%5);
          lc75710_dcram_write(5, i%6);
            lc75710_dcram_write(6, i%4);
              lc75710_dcram_write(7, i%5);
                lc75710_dcram_write(8, i%2);
              lc75710_dcram_write(9, i%1);
*/
  /* The app ! */

  menu_page_sources();
  
  /* TEST CODE, will be removed (adapted to real inputs :) )*/
  if (Serial.available() > 0)
  {
      in = Serial.read();
      switch (in)
      {
        case '1':
          digitalWrite(keypad.pins[0], true);
          break;
        case '2':
          digitalWrite(keypad.pins[1], true);
          break;
      }

  }
  else
  {
    digitalWrite(keypad.pins[0], false);
    digitalWrite(keypad.pins[1], false);
  }
  
  keypad_read();

}


