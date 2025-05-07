#ifndef UI_H
#define UI_H

#include "VIMS_defs.h"

#define MENU_CLOSED 0
#define MENU_OPEN 1

#define MENUELEMENT_BUTTON 1
#define MENUELEMENT_LABEL 2

typedef struct _menuelement_t {
    bool (*onclick)(struct _menuelement_t *obj);
    int x1, y1;
    int width;
    char *text;
    uint8_t type;
} menuelement_t;

menuelement_t ielement_centered(bool (*onclick)(struct _menuelement_t *obj), int y, char *text, uint8_t type);
menuelement_t ielement(bool (*onclick)(struct _menuelement_t *obj), int x1, int y1, int width, char *text, uint8_t type);

typedef struct _menudef_t {
    menuelement_t *elements;
    uint8_t element_cnt;
    struct _menudef_t *prev_menu; // menu to return to if this one is closed
    bool has_button; // does this menu contain any buttons (VERY IMPORTANT THAT THIS IS SET CORRECTLY, ELSE ANY ARROW KEY PRESSES IN THIS MENU WILL FREEZE THE GAME)
} menudef_t;

menudef_t imenu(menuelement_t *elements, uint8_t element_cnt, menudef_t *prev);

typedef struct {
    menudef_t *menu;
    int selected_button;
} menu_instance_t;

int ui_getmenustatus(void);
int ui_rendermenu(void);

int ui_entermenu(menudef_t *menu);
int ui_closemenu(void);

void menu_keyboard_handler(void);

#endif