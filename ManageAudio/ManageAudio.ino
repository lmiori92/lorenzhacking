
//#define LC75710_CE              2   /**< Chip Enable pin. */
//#define LC75710_DI              3   /**< Serial Data Input pin.*/
//#define LC75710_CL              4   /**< Serial Clock pin. */
#include "lc75710.h"
#include "ma_gui.h"
#include "ma_util.h"
#include "stdint.h"

#include "avr/eeprom.h"

/* Program's global variables */
static t_menu   menu;    /**< Global menu state */
static t_keypad keypad;  /**< Global keypad state */
static t_persistent persistent;  /**< Global persistent app state */

/* PIN definitions */
#define RLY_1    8
#define RLY_2    9
#define RLY_3    10

/* GUI definitions */
#define MENU_BACK_ENTRY    { .label = "BACK", .cb = &ma_gui_menu_goto_previous }

/* Source menu entries */
t_menu_entry MENU_SOURCE[] = {
                            { .label = "AUX",            .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = "CD",             .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = "RADIO",          .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = "TAPE",           .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                            { .label = MENU_END_ENTRY,   .cb = &ma_gui_menu_goto_main, .cb_hoover = &ma_gui_source_select },
                           };

/* Source menu page */
t_menu_page PAGE_SOURCE = {
    .pre     = NULL,
    .entries = MENU_SOURCE,
    .post    = NULL,
    .page_previous = NULL,
};

t_menu_entry MENU_SETTINGS[] = {
                            { .label = "Sources",        .cb = NULL, .cb_hoover = NULL },
                            { .label = "Display",        .cb = &ma_gui_menu_goto_sett_display, .cb_hoover = NULL  },
                            { .label = "FFT",        .cb = &ma_gui_menu_goto_main, .cb_hoover = NULL    },
                            { .label = "VU-Meter",        .cb = &ma_gui_menu_goto_main, .cb_hoover = NULL     },
                            { .label = "Tools",        .cb = &ma_gui_menu_goto_tools, .cb_hoover = NULL     },
                            MENU_BACK_ENTRY,
                            { .label = MENU_END_ENTRY, .cb = NULL },
                           };

t_menu_page PAGE_SETTINGS = {
    .pre     = NULL,
    .entries = MENU_SETTINGS,
    .post    = NULL,
    .page_previous = &PAGE_SOURCE,
};

t_menu_entry MENU_SETTINGS_DISPLAY[] = {
                            { .label = "Brightness",        .cb = &ma_gui_menu_goto_sett_brightness, .cb_hoover = NULL  },
                            { .label = "Menu Style",        .cb = NULL, .cb_hoover = NULL  },
                            MENU_BACK_ENTRY,
                            { .label = MENU_END_ENTRY, .cb = NULL },
                           };

t_menu_page PAGE_SETTINGS_DISPLAY = {
    .pre     = NULL,
    .entries = MENU_SETTINGS_DISPLAY,
    .post    = NULL,
    .page_previous = &PAGE_SETTINGS,
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

t_menu_page PAGE_SETTINGS_BRIGHTNESS = {
    .pre     = ma_gui_settings_brightness_pre,
    .entries = MENU_SETTINGS_BRIGHTNESS,
    .post    = NULL,
    .page_previous = &PAGE_SETTINGS_DISPLAY,
};

t_menu_entry MENU_SETTINGS_TOOLS[] = {
                            { .label = "Reboot",        .cb = &ma_gui_menu_tools_selection, .cb_hoover = NULL  },
                            MENU_BACK_ENTRY,
                            { .label = MENU_END_ENTRY, .cb = NULL },
                           };

t_menu_page PAGE_SETTINGS_TOOLS = {
    .pre     = NULL,
    .entries = MENU_SETTINGS_TOOLS,
    .post    = NULL,
    .page_previous = &PAGE_SETTINGS,
};

void ma_gui_settings_brightness_pre()
{
    menu.index = persistent.brightness;
}

t_menu_page* ma_gui_source_select(uint8_t id, t_menu_page* page)
{

    bool outputs[SOURCE_MAX];

    switch(id)
    {
        case 0:
            source_select(0U, outputs);
            break;
        case 1:
            source_select(1, outputs);
            break;
        case 2:
            source_select(2, outputs);
            break;
        case 3:
            source_select(3, outputs);
            break;
    }

    digitalWrite(RLY_1, outputs[0]);
    digitalWrite(RLY_2, outputs[1]);
    digitalWrite(RLY_3, outputs[2]);
  
    return NULL;
}

/* MENU: callbacks */

t_menu_page* ma_gui_menu_goto_previous(uint8_t id, t_menu_page* page)
{
    return page->page_previous;
}

t_menu_page* ma_gui_menu_goto_main(uint8_t id, t_menu_page* page)
{
    return &PAGE_SETTINGS;
}

t_menu_page* ma_gui_menu_goto_sett_display(uint8_t id, t_menu_page* page)
{
    return &PAGE_SETTINGS_DISPLAY;
}

t_menu_page* ma_gui_menu_goto_sett_brightness(uint8_t id, t_menu_page* page)
{
    return &PAGE_SETTINGS_BRIGHTNESS;
}

t_menu_page* ma_gui_menu_set_brightness(uint8_t id, t_menu_page* page)
{
    set_display_brightness(id);
    return NULL;
}

t_menu_page* ma_gui_menu_goto_tools(uint8_t id, t_menu_page* page)
{
    return NULL;
}

t_menu_page* ma_gui_menu_tools_selection(uint8_t id, t_menu_page* page)
{
    void (*start)(void) = 0;
    switch(id)
    {
        case 0:
            start();
            break;
        default:
            break;
    }
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
        
        persistent.brightness = level;
    write_to_persistent(&persistent);
}

/**
* read_from_persistent
*
* @brief Read persistent variables from the non volatile storage
* @param persistent the persistent variables to write to
*/
void read_from_persistent(t_persistent* persistent)
{
    //eeprom_read_block(persistent, (void*)5, sizeof(t_persistent));
    persistent->brightness = eeprom_read_byte((const uint8_t*)5);
    Serial.print("I:");
    Serial.println(persistent->brightness, DEC);
}

/**
* write_to_persistent
*
* @brief Write persistent variables to the non volatile storage
* @param persistent the persistent variables to read from
*/
void write_to_persistent(t_persistent* persistent)
{
    //eeprom_write_block((void*)5, persistent, sizeof(t_persistent));
    eeprom_write_byte((uint8_t*)5, persistent->brightness);
    Serial.print("O:");
    Serial.println(persistent->brightness, DEC);
}

void source_relays_init()
{
    pinMode(RLY_1, OUTPUT);
    pinMode(RLY_2, OUTPUT);
    pinMode(RLY_3, OUTPUT);
}

void setup()
{

    /* Initialize the serial port */
    Serial.begin(9600);

    /* Load persistent data */
    read_from_persistent(&persistent);

    /* Initialze the display */
    lc75710_init();

    /* Audio Switch Relays init */
    source_relays_init();

    /* Initialize the GUI */
    ma_gui_init(&menu, &keypad, &PAGE_SOURCE);

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
}
