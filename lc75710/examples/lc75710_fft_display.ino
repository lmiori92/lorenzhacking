/* ADD here pin defines, if you are using different pins */

#include "lc75710.h"    /* Include LC75710 functions and definitions */

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

void setup()
{
    /* Clear internal RAM */
    display_clear(64);
    /* Turn all the display digits on */
    lc75710_on_off(MDATA_AND_ADATA, true, 0xFFFF);
    /* Display the string */
    display_string("Hacked!");
}

void loop()
{
    /* This example does only display a static string
     * No periodic logic to be run ! */
}
