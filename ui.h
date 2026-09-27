#ifndef UI_H
#define UI_H

#include "VIMS_defs.h"
#include "settings.h"

// menu state
#define MENU_CLOSED 0
#define MENU_OPEN 1

// menu element types
enum menuelement_id {
    MENUELEMENT_BUTTON = 1,
    MENUELEMENT_LABEL,
    MENUELEMENT_TITLE,

    MENUELEMENT_SETUP_BOOL,
    MENUELEMENT_SETUP_SLIDER,
};

// the height of a menu element is 11 pixels (inclusive)
typedef struct _menu_element {
    bool (*onclick)(struct _menu_element *this); // onclick function, gets object from which it was called, returns whether the action was run successfully
    int x1, y1; // coordinates of the top-right corner
    int width; // width in pixels of the box
    char *text;
    enum menuelement_id type; // MENUELEMENT
    enum setup_key_id setupkey; // used only if this is a SETUP element
} menu_element;

// create a menu element with entirely arbitrary data (-1 width means calculate from text length, -1 to x1 means the element should be centered)
menu_element ielement(bool (*onclick)(struct _menu_element *this), int x1, int y1, int width, char *text, enum menuelement_id type, enum setup_key_id setupkey);

// a collection of menu elements, a page
typedef struct _menu_page {
    menu_element *elements;
    uint8_t element_cnt;
    bool has_selectable; // does this menu page contain any selectables (VERY IMPORTANT THAT THIS IS SET CORRECTLY, ELSE ANY ARROW KEY PRESSES IN THIS PAGE WILL FREEZE THE GAME)
} menu_page;

menu_page imenupage(menu_element *elements, uint8_t element_cnt);

// collection of pages
typedef struct _menu {
    menu_page *pages;
    uint8_t page_cnt;
    struct _menu *prev_menu; // menu to return to if this one is closed
} menu;

// initializes a new menu object (not an instance)
menu imenu(menu_page *pages, uint8_t page_cnt, menu *prev);

// returns whether a menu is open and thus the game should be paused
int ui_getmenustatus(void);

// renders the menu onto the screen, with all labels and buttons
int ui_rendermenu(void);

int ui_rendermenu_slider(void);

// sets the global state to be in the given menu
int ui_entermenu(menu *menu);

// switch pages
int ui_prevpage(void);
int ui_nextpage(void);
// initialise current page (set selected index to first selectable if it exists)
int ui_initpage(void);

// returns to the parent of the current menu (returns to game if NULL)
int ui_closemenu(void);

// handles keypresses in every tick if a menu is on (call from w_tick)
void menu_keyboard_handler(void);

// changes the current global button index to the index of the next / previous MENUELEMENT_BUTTON
int ui_nextbutton(void);
int ui_prevbutton(void);

bool ui_is_selectable(menu_element e);

// ----- specific object handlers -----

extern menu menu_settings;

bool onclick_closemenu(menu_element *this);
bool onclick_quit(menu_element *this);
bool onclick_open_settings(menu_element *this);
// can handle any setup-type element
bool onclick_setup_bool(menu_element *this);
bool onclick_setup_slider(menu_element *this);

#endif