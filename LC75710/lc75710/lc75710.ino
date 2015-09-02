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

void lc75710_write(uint32_t data)
{

    uint8_t i = 0;

    /* Address goes out first... */
    digitalWrite(CE, LOW);
    for (i = 0; i < 8; i++)
    {
        digitalWrite(CL, LOW);
//        delayMicroseconds(1);
        digitalWrite(DI, (ADDRESS >> 7-i) & 0x1);
//        delayMicroseconds(1);
        digitalWrite(CL, HIGH);
    }
    digitalWrite(CL, LOW);

//    delayMicroseconds(1);

    /* Then data follows after, CE goes high */
    digitalWrite(CE, HIGH);
//    delayMicroseconds(1);

    for (i = 0; i < 24; i++)
    {
        digitalWrite(CL, LOW);
//        delayMicroseconds(1);
        digitalWrite(DI, (data >> i) & 0x1);
//        delayMicroseconds(1);
        digitalWrite(CL, HIGH);
    }

//    delayMicroseconds(1);
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

    /* DCRAM address */
    temp |= (uint32_t)(dcram & 0xF) << 16;

    /* ADRAM address */
    temp |= (uint32_t)(adram & 0x3F) << 8;

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
    temp |= (uint32_t)(data & 0x7F) << 8;

    /* Write to IC */
    lc75710_write(temp);

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
    lc75710_intensity(128);

    /* Turn the display ON */
    lc75710_on_off(MDATA_AND_ADATA, true, 0xFFFF);
    display_clear(64);
    display_string("Hacked!");
}

void display_string(char* string)
{
    uint8_t len = 0;
    char* str = string;
    while (*(string++) != 0) len++;
    display_string_len(str, len);
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

void setup()
{

  Serial.begin(9600);
  lc75710_init();
  
}

void loop()
{
  char in;
  /* The app ! */
  if (Serial.available() > 0)
  {
      in = Serial.read();
      switch (in)
      {
      
        case '1':
          display_string("1. Setup");
          break;
        case '2':
          display_string("2. Audio");
          break;
        case '3':
          display_string("3. Timer");
          break;
        case '4':
          display_string("4. Mixer");
          break;        
      }
  }

}


