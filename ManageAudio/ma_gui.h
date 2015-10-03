
/** This structure describes a menu entry */

#ifndef __MA_GUI__
#define __MA_GUI__

#define MENU_END_ENTRY    "MENU_END_ENTRY"

#define DEBOUNCE_BUTTONS   50

struct menu_entry_
{

    char*               label;
    struct menu_page_* (*cb)(uint8_t id, struct menu_page_* page);
    struct menu_page_* (*cb_hoover)(uint8_t id, struct menu_page_* page);

};

typedef struct menu_entry_ t_menu_entry;

/** Structure that holds information about a menu page */
struct menu_page_
{

    void (*pre)(void);          /**< Pointer to a function that is called when the menu page is first shown */
    t_menu_entry* entries;      /**< Menu entries */
    void (*post)(void);         /**< Pointer to a function that is called when the menu page is quit (back to another menu / shutdown) */
    menu_page_* page_previous;  /**< Pointer to the previous page */

};

typedef struct menu_page_ t_menu_page;

typedef struct
{
    uint8_t index;
    t_menu_page* page;
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

