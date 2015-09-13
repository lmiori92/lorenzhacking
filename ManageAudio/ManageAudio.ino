#include "lc75710.h"
#include "ma_gui.h"
#include "ma_util.h"

/* Definitions for main program (TODO move me!) */


/* Program's global variables */
static t_menu   menu;    /**< Global menu state */
static t_keypad keypad;  /**< Global keypad state */
static t_persistent persistent;  /**< Global persistent app state */

/* GUI definitions */
#define MENU_BACK_ENTRY    { .label = "BACK", .cb = &ma_gui_menu_goto_previous }

t_menu_entry MENU_SOURCE[] = {
                            { .label = "AUX",            .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = "CD",             .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = "RADIO",          .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = "TAPE",           .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = MENU_END_ENTRY,   .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                           };

t_menu_entry MENU_SETTINGS[] = {
                            { .label = "Sources",        .cb = NULL, .cb_hoover = NULL },
                            { .label = "Display",        .cb = &ma_gui_menu_goto_sett_display, .cb_hoover = NULL  },
                            { .label = "FFT",        .cb = &ma_gui_menu_goto_main, .cb_hoover = NULL    },
                            { .label = "VU-Meter",        .cb = &ma_gui_menu_goto_main, .cb_hoover = NULL     },
                            MENU_BACK_ENTRY,
                            { .label = MENU_END_ENTRY, .cb = NULL },
                           };

t_menu_entry MENU_SETTINGS_DISPLAY[] = {
                            { .label = "Brightness",        .cb = &ma_gui_menu_goto_sett_brightness, .cb_hoover = NULL  },
                            { .label = "Menu Style",        .cb = NULL, .cb_hoover = NULL  },
                            MENU_BACK_ENTRY,
                            { .label = MENU_END_ENTRY, .cb = NULL },
                           };
                           
t_menu_entry MENU_SETTINGS_BRIGHTNESS[] = {
                            { .label = "1: TeSt!*",        .cb = &ma_gui_menu_goto_previous, .cb_hoover = &ma_gui_menu_set_brightness  },
                            { .label = "2: TeSt!*",        .cb = &ma_gui_menu_goto_previous, .cb_hoover = &ma_gui_menu_set_brightness  },
                            { .label = "3: TeSt!*",        .cb = &ma_gui_menu_goto_previous, .cb_hoover = &ma_gui_menu_set_brightness  },
                            { .label = "4: TeSt!*",        .cb = &ma_gui_menu_goto_previous, .cb_hoover = &ma_gui_menu_set_brightness  },
                            { .label = "5: TeSt!*",        .cb = &ma_gui_menu_goto_previous, .cb_hoover = &ma_gui_menu_set_brightness  },
                            MENU_BACK_ENTRY,
                            { .label = MENU_END_ENTRY, .cb = NULL },
                           };

t_menu_entry* ma_gui_source_select(uint8_t id, t_menu_entry* page)
{
    switch(id)
    {
        case 0:
            Serial.println("Switch to: AUX");
            break;
        case 1:
            Serial.println("Switch to: CD");
            break;
        case 2:
            Serial.println("Switch to: RADIO");
            break;
        case 3:
            Serial.println("Switch to: TAPE");
            break;
    }
  
    return NULL;
}

/* MENU: callbacks */

t_menu_entry* ma_gui_menu_goto_previous(uint8_t id, t_menu_entry* page)
{
    return menu.page_previous;
}

t_menu_entry* ma_gui_menu_goto_main(uint8_t id, t_menu_entry* page)
{
    return MENU_SETTINGS;
}

t_menu_entry* ma_gui_menu_goto_sett_display(uint8_t id, t_menu_entry* page)
{
    return MENU_SETTINGS_DISPLAY;
}

t_menu_entry* ma_gui_menu_goto_sett_brightness(uint8_t id, t_menu_entry* page)
{
    return MENU_SETTINGS_BRIGHTNESS;
}

t_menu_entry* ma_gui_menu_set_brightness(uint8_t id, t_menu_entry* page)
{
    set_display_brightness(id);
    return NULL;
}

/**
* set_display_brightness
*
* @brief Set application display brightness
*/
void set_display_brightness(uint8_t level)
{
    static uint8_t brightness_levels[5] = { 48, 96, 144, 192, 240 };
    
    if (level < 5)
        lc75710_intensity(brightness_levels[level]);
}

/**
* read_from_persistent
*
* @brief Read persistent variables from the non volatile storage
* @param persistent the persistent variables to write to
*/
void read_from_persistent(t_persistent* persistent)
{
    (void)persistent;
}

/**
* write_to_persistent
*
* @brief Write persistent variables to the non volatile storage
* @param persistent the persistent variables to read from
*/
void write_to_persistent(t_persistent* persistent)
{
    (void)persistent;
}

void setup()
{

    /* Initialize the serial port */
    Serial.begin(9600);

    /* Load persistent data */
    read_from_persistent(&persistent);

    /* Initialze the display */
    lc75710_init();

    /* Initialize the GUI */
    ma_gui_init(&menu, &keypad, MENU_SOURCE);
    
    /* Apply persistent data */
    set_display_brightness(persistent.brightness);

}

void loop()
{

    /* Run the periodic GUI logic */
    ma_gui_periodic(&menu, &keypad);

}

void shutdown()
{
    /* Save to EEPROM */
    write_to_persistent(&persistent);
}
