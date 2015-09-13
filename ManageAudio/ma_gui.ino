
#include "ma_gui.h"

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

void ma_gui_init(t_menu* menu, t_keypad* keypad, t_menu_entry* start_page)
{

  uint8_t i = 0;
  
  Serial.begin(9600);
  lc75710_init();
  
  /* Init keypad */
  keypad->pins[BUTTON_UP] = 7;
  keypad->pins[BUTTON_DOWN] = 8;
  keypad->pins[BUTTON_SELECT] = 9;
  
  for (i = 0; i < NUM_BUTTONS; i++)
  {
      pinMode(keypad->pins[i], OUTPUT);
      keypad->buttons[i]  = false;
      keypad->latches[i]  = false;
      keypad->debounce[i] = millis();
  }
  
  /* First selected page: audio sources */
  menu->page          = start_page;
  menu->page_previous = start_page;
  menu->index         = 0;

  ma_gui_menu_update(menu);

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

void ma_gui_menu_update(t_menu* menu)
{
    display_clear(10);
    display_string_center(menu->page[menu->index].label);
}

void ma_gui_menu(t_menu* menu)
{

    uint8_t i = 0;
    bool inhibit_up;
    bool inhibit_down;
    bool refresh = false;
    t_menu_entry* page_next = NULL;

    while (menu->page[i].label != MENU_END_ENTRY)
    {
        i++;
    }
    i--;

    inhibit_up = (menu->index == 0) ? true : false;
    inhibit_down = (menu->index == i) ? true : false;

    if (inhibit_down == false && keypad.buttons[BUTTON_DOWN])
    {
        menu->index++;
        refresh = true;
    }
    else if (inhibit_up == false && keypad.buttons[BUTTON_UP])
    {
        menu->index--;
        refresh = true;
    }
    else if (keypad.buttons[BUTTON_SELECT])
    {
        if (menu->page[menu->index].cb != NULL)
        {
            page_next = menu->page[menu->index].cb(menu->index, menu->page);
        }
    }
    else
    {
        /* WTF */
    }
    
    if (page_next != NULL)
    {
        menu->page_previous = menu->page;
        menu->page = page_next;
        menu->index = 0;
        refresh = true;
    }
    
    if (refresh == true)
    {
        ma_gui_menu_update(menu);
        if (menu->page[menu->index].cb_hoover != NULL)
            page_next = menu->page[menu->index].cb_hoover(menu->index, menu->page);
    }

}

void ma_gui_periodic(t_menu* menu, t_keypad* keypad)
{
  char in;
  static int i = 0;
  i++;

  /* Run menu logic */
  ma_gui_menu(menu);

  /* TEST CODE, will be removed (adapted to real inputs :) )*/
  if (Serial.available() > 0)
  {
      in = Serial.read();
      switch (in)
      {
        case '1':
          digitalWrite(keypad->pins[0], true);
          break;
        case '2':
          digitalWrite(keypad->pins[1], true);
          break;
        case '3':
          digitalWrite(keypad->pins[2], true);
          break;
      }

  }
  else
  {
    digitalWrite(keypad->pins[0], false);
    digitalWrite(keypad->pins[1], false);
    digitalWrite(keypad->pins[2], false);
  }

  keypad_read();

}


