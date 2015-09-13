
/** This structure describes a menu entry */

#ifndef __MA_GUI__
#define __MA_GUI__

#define MENU_END_ENTRY    "MENU_END_ENTRY"

#define DEBOUNCE_BUTTONS   50

struct menu_entry_
{

    char*          label;
    struct menu_entry_* (*cb)(uint8_t id, struct menu_entry_* page);
    struct menu_entry_* (*cb_hoover)(uint8_t id, struct menu_entry_* page);

};

typedef struct menu_entry_ t_menu_entry;

typedef struct
{
    uint8_t index;
    t_menu_entry* page;
    t_menu_entry* page_previous;
} t_menu;

enum e_buttons_
{

    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SELECT,

    NUM_BUTTONS

};

typedef struct
{
    uint8_t  pins[NUM_BUTTONS];
    bool     buttons[NUM_BUTTONS];
    int      debounce[NUM_BUTTONS];
    bool     latches[NUM_BUTTONS];
} t_keypad;



typedef enum e_buttons_ t_button;

/* Menu - callbacks */
t_menu_entry* ma_gui_source_select(uint8_t id, uint8_t page);

#endif

