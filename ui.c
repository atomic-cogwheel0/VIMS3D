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

menudef_t imenu(menuelement_t *elements, uint8_t element_cnt, menudef_t *prev) {
    menudef_t d;
    int i;
    d.elements = elements;
    d.element_cnt = element_cnt;
    d.prev_menu = prev;
    d.has_button = FALSE;
    if (element_cnt > 0 && elements != NULL) {
        for (i = 0; i < element_cnt; i++) {
            if (elements[i].type == MENUELEMENT_BUTTON) {
                d.has_button = TRUE;
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
            if (current_menu.selected_button == i) {
                Bdisp_AreaReverseVRAM(curr.x1 + 1, curr.y1 + 1, curr.x1 + curr.width - 1, curr.y1 + 10 - 1);
            }
        }
    }
    return S_SUCCESS;
}

int ui_entermenu(menudef_t *menu) {
    current_menu.menu = menu;
    if (menu != NULL) {
        if (current_menu.menu->has_button) {
            current_menu.selected_button = 0;
            while (current_menu.selected_button < current_menu.menu->element_cnt) {
                if (current_menu.menu->elements[current_menu.selected_button].type == MENUELEMENT_BUTTON) break;
                current_menu.selected_button++;
            }
        }
        else {
            current_menu.selected_button = -1;
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
        if (current_menu.selected_button != -1) {
            if (current_menu.menu->elements[current_menu.selected_button].onclick != NULL) {
                current_menu.menu->elements[current_menu.selected_button].onclick(&current_menu.menu->elements[current_menu.selected_button]);
            }
        }
    }

    if (IsKeyDown(KEY_CTRL_EXIT)) {
        ui_closemenu();
    }
}

int ui_inc_button_index(void) {
    if (current_menu.selected_button != -1) {
        do {
            current_menu.selected_button++;
            if (current_menu.selected_button >= current_menu.menu->element_cnt) {
                current_menu.selected_button = 0;
            }
        } while (current_menu.menu->elements[current_menu.selected_button].type != MENUELEMENT_BUTTON);
        return S_SUCCESS;
    }
    else {
        return S_NOBUTTON;
    }
}

int ui_dec_button_index(void) {
    if (current_menu.selected_button != -1) {
        do {
            current_menu.selected_button--;
            if (current_menu.selected_button < 0) {
                current_menu.selected_button = current_menu.menu->element_cnt - 1;
            }
        } while (current_menu.menu->elements[current_menu.selected_button].type != MENUELEMENT_BUTTON);
        return S_SUCCESS;
    }
    else {
        return S_NOBUTTON;
    }
}