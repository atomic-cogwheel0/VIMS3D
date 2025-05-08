#ifndef UI_H
#define UI_H

#include "VIMS_defs.h"
#include "settings.h"

// menu state
#define MENU_CLOSED 0
#define MENU_OPEN 1

// menu element types
#define MENUELEMENT_BUTTON 1
#define MENUELEMENT_LABEL 2
#define MENUELEMENT_TITLE 3

#define MENUELEMENT_SETUP_BOOL 17

// the height of a menu element is 11 pixels (inclusive)
typedef struct _menuelement_t {
    bool (*onclick)(struct _menuelement_t *obj); // onclick function, gets object from which it was called, returns whether the action was run successfully
    int x1, y1; // coordinates of the top-right corner
    int width; // width in pixels of the box
    char *text;
    uint8_t type; // MENUELEMENT
    uint8_t setupkey; // used only if this is a SETUP element
} menuelement_t;

// create a menu element with entirely arbitrary data (-1 width means calculate from text length, -1 to x1 means the element should be centered)
menuelement_t ielement(bool (*onclick)(struct _menuelement_t *obj), int x1, int y1, int width, char *text, uint8_t type, int setupkey);

typedef struct _menudef_t {
    menuelement_t *elements;
    uint8_t element_cnt;
    struct _menudef_t *prev_menu; // menu to return to if this one is closed
    bool has_selectable; // does this menu contain any selectables (VERY IMPORTANT THAT THIS IS SET CORRECTLY, ELSE ANY ARROW KEY PRESSES IN THIS MENU WILL FREEZE THE GAME)
} menudef_t;

// initializes a new menu object (not an instance)
menudef_t imenu(menuelement_t *elements, uint8_t element_cnt, menudef_t *prev);

// has a single static instance, in which the global state is stored
typedef struct {
    menudef_t *menu; // the currently open menu
    int selected; // number of the selected selectable (beneficial if the given index really is selectable; -1 means current menu has no selectables at all)
} menu_instance_t;

// returns whether a menu is open and thus the game should be paused
int ui_getmenustatus(void);

// renders the menu onto the screen, with all labels and buttons
int ui_rendermenu(void);

// sets the global state to be in the given menu
int ui_entermenu(menudef_t *menu);

// returns to the parent of the current menu (returns to game if NULL)
int ui_closemenu(void);

// handles keypresses in every tick if a menu is on (call from w_tick)
void menu_keyboard_handler(void);

// changes the current global button index to the index of the next / previous MENUELEMENT_BUTTON
int ui_inc_button_index(void);
int ui_dec_button_index(void);

bool ui_is_selectable(menuelement_t e);

// ----- specific object handlers -----

extern menudef_t menu_settings;

bool onclick_closemenu(void *unused);
bool onclick_quit(void *unused);
bool onclick_open_settings(void *unused);
// handler of any bool setup onclicks
bool onclick_setup_bool(menuelement_t *el);

#endif