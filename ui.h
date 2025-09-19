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
#define MENUELEMENT_SETUP_SLIDER 18

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

// a collection of menu elements, a page
typedef struct _menupage_t {
    menuelement_t *elements;
    uint8_t element_cnt;
    bool has_selectable; // does this menu page contain any selectables (VERY IMPORTANT THAT THIS IS SET CORRECTLY, ELSE ANY ARROW KEY PRESSES IN THIS PAGE WILL FREEZE THE GAME)
} menupage_t;

menupage_t imenupage(menuelement_t *elements, uint8_t element_cnt);

// collection of pages
typedef struct _menu_t {
    menupage_t *pages;
    uint8_t page_cnt;
    struct _menu_t *prev_menu; // menu to return to if this one is closed
} menu_t;

// initializes a new menu object (not an instance)
menu_t imenu(menupage_t *pages, uint8_t page_cnt, menu_t *prev);

// has a single static instance, in which the global state is stored
typedef struct {
    menu_t *menu; // the currently open menu
    int page; // currently viewed page
    int selected; // number of the selected selectable (this element should be selectable; -1 means current menu has no selectables at all)
    menuelement_t *in_slider;
    int sliderpos;
} menu_instance_t;

// returns whether a menu is open and thus the game should be paused
int ui_getmenustatus(void);

// renders the menu onto the screen, with all labels and buttons
int ui_rendermenu(void);

int ui_rendermenu_slider(void);

// sets the global state to be in the given menu
int ui_entermenu(menu_t *menu);

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

bool ui_is_selectable(menuelement_t e);

// ----- specific object handlers -----

extern menu_t menu_settings;

bool onclick_closemenu(void *unused);
bool onclick_quit(void *unused);
bool onclick_open_settings(void *unused);
// handler of any bool setup onclicks
bool onclick_setup_bool(menuelement_t *el);
bool onclick_setup_slider(menuelement_t *el);

#endif