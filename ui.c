#include "ui.h"

static volatile bool inmenu = FALSE;
menu_instance_t current_menu = {NULL, 0};

menuelement_t ielement(bool (*onclick)(struct _menuelement_t *obj), int x1, int y1, int width, char *text, uint8_t type) {
    menuelement_t e;
    e.onclick = onclick;
    e.x1 = x1;
    e.y1 = y1;
    e.width = width;
    e.text = text;
    e.type = type;
    e.setupkey = 0;
    return e;
}

menuelement_t ielement_centered(bool (*onclick)(struct _menuelement_t *obj), int y, char *text, uint8_t type) {
    int xc, wc;

    if (text == NULL) {
        wc = 7;
    }
    else {
        wc = strlen(text) * 6 + 4;
    }

    xc = 63 - wc/2;

    return ielement(onclick, xc, y, wc, text, type);
}

menuelement_t ielement_setup(int y, char *text, uint8_t type, uint8_t setupkey) {
    menuelement_t e;
    e.x1 = 1;
    e.y1 = y;
    e.width = type * 6 + 4;
    e.text = text;
    e.type = type;
    if (type == MENUELEMENT_SETUP_BOOL) {
        e.onclick = onclick_setup_bool;
    }
    e.setupkey = setupkey;
    return e;
}

menudef_t imenu(menuelement_t *elements, uint8_t element_cnt, menudef_t *prev) {
    menudef_t d;
    int i;
    d.elements = elements;
    d.element_cnt = element_cnt;
    d.prev_menu = prev;
    d.has_selectable = FALSE;
    if (element_cnt > 0 && elements != NULL) {
        for (i = 0; i < element_cnt; i++) {
            if (ui_is_selectable(elements[i])) {
                d.has_selectable = TRUE;
            }
        }
    }
    return d;
}

int ui_getmenustatus(void) {
    if (!inmenu || current_menu.menu == NULL) {
        return MENU_CLOSED;
    }
    else return MENU_OPEN;
}

int ui_rendermenu(void) {
    uint8_t i;
    menuelement_t curr;
    if (!inmenu || current_menu.menu == NULL) return S_ENOMENU;
    
    for (i = 0; i < current_menu.menu->element_cnt; i++) {
        curr = current_menu.menu->elements[i];
        if (curr.text != NULL) {
            PrintXY(curr.x1 + 2, curr.y1 + 2, (unsigned char *) curr.text, 0);
        }
        if (curr.type == MENUELEMENT_BUTTON) {
            Bdisp_DrawLineVRAM(curr.x1, curr.y1, curr.x1 + curr.width, curr.y1);
            Bdisp_DrawLineVRAM(curr.x1, curr.y1, curr.x1, curr.y1 + 10);
            Bdisp_DrawLineVRAM(curr.x1 + curr.width, curr.y1, curr.x1 + curr.width, curr.y1 + 10);
            Bdisp_DrawLineVRAM(curr.x1, curr.y1 + 10, curr.x1 + curr.width, curr.y1 + 10);
            if (current_menu.selected == i) {
                Bdisp_AreaReverseVRAM(curr.x1 + 1, curr.y1 + 1, curr.x1 + curr.width - 1, curr.y1 + 10 - 1);
            }
        }
        if (curr.type == MENUELEMENT_SETUP_BOOL) {
            Bdisp_AreaReverseVRAM(117, curr.y1 + 1, 125, curr.y1 + 9);
            Bdisp_AreaReverseVRAM(118, curr.y1 + 2, 124, curr.y1 + 8);
            if (setup_getval(curr.setupkey)) {
                /*Bdisp_DrawLineVRAM(118, curr.y1 + 6, 120, curr.y1 + 8);
                Bdisp_DrawLineVRAM(120, curr.y1 + 8, 124, curr.y1 + 4);*/
                Bdisp_AreaReverseVRAM(119, curr.y1 + 3, 123, curr.y1 + 7);
            }
            if (current_menu.selected == i) {
                Bdisp_DrawLineVRAM(0, curr.y1, 0, curr.y1 + 10);
                Bdisp_DrawLineVRAM(1, curr.y1, 1, curr.y1 + 10);
            }
        }
    }
    return S_SUCCESS;
}

bool ui_is_selectable(menuelement_t e) {
    return e.type == MENUELEMENT_BUTTON || e.type == MENUELEMENT_SETUP_BOOL;
}

int ui_entermenu(menudef_t *menu) {
    current_menu.menu = menu;
    if (menu != NULL) {
        if (current_menu.menu->has_selectable) {
            current_menu.selected = 0;
            while (current_menu.selected < current_menu.menu->element_cnt) {
                if (ui_is_selectable(current_menu.menu->elements[current_menu.selected])) break;
                current_menu.selected++;
            }
        }
        else {
            current_menu.selected = -1;
        }
        inmenu = TRUE;
    }
    else {
        inmenu = FALSE;
    }
    return S_SUCCESS;
}

int ui_closemenu(void) {
    return ui_entermenu(current_menu.menu->prev_menu);
}

static bool done_next = FALSE;
static bool done_prev = FALSE;
static bool done_exec = FALSE;
static bool done_close = FALSE;

void menu_keyboard_handler(void) {
    if (!inmenu || current_menu.menu == NULL) return;

    if (IsKeyDown(KEY_CTRL_RIGHT) || IsKeyDown(KEY_CTRL_DOWN)) {
        if (!done_next) {
            ui_inc_button_index();
            done_next = TRUE;
        }
    }
    else {
        done_next = FALSE;
    }

    if (IsKeyDown(KEY_CTRL_LEFT) || IsKeyDown(KEY_CTRL_UP)) {
        if (!done_prev) {
            ui_dec_button_index();
            done_prev = TRUE;
        }
    }
    else {
        done_prev = FALSE;
    }

    if (IsKeyDown(KEY_CTRL_EXE)) {
        if (!done_exec) {
            if (current_menu.selected != -1) {
                if (current_menu.menu->elements[current_menu.selected].onclick != NULL) {
                    current_menu.menu->elements[current_menu.selected].onclick(&current_menu.menu->elements[current_menu.selected]);
                }
            }
            done_exec = TRUE;
        }
    }
    else {
        done_exec = FALSE;
    }

    if (IsKeyDown(KEY_CTRL_EXIT)) {
        if (!done_close) {
            ui_closemenu();
            done_close = TRUE;
        }
    }
    else {
        done_close = FALSE;
    }
}

int ui_inc_button_index(void) {
    if (current_menu.selected != -1) {
        do {
            current_menu.selected++;
            if (current_menu.selected >= current_menu.menu->element_cnt) {
                current_menu.selected = 0;
            }
        } while (!ui_is_selectable(current_menu.menu->elements[current_menu.selected]));
        return S_SUCCESS;
    }
    else {
        return S_NOBUTTON;
    }
}

int ui_dec_button_index(void) {
    if (current_menu.selected != -1) {
        do {
            current_menu.selected--;
            if (current_menu.selected < 0) {
                current_menu.selected = current_menu.menu->element_cnt - 1;
            }
        } while (!ui_is_selectable(current_menu.menu->elements[current_menu.selected]));
        return S_SUCCESS;
    }
    else {
        return S_NOBUTTON;
    }
}

// ----- specific object handlers -----

bool onclick_closemenu(void *unused) {
    if (ui_closemenu() == S_SUCCESS) {
        return TRUE;
    }
    return FALSE;
}

bool onclick_quit(void *unused) {
    quit();
    return TRUE; // won't run
}

bool onclick_open_settings(void *unused) {
    return ui_entermenu(&menu_settings);
}

bool onclick_setup_bool(menuelement_t *el) {
    uint8_t key = el->setupkey;
    setup_setval(key, !setup_getval(key));
    return TRUE;
}