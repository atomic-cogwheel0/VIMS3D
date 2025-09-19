#include "ui.h"

static volatile bool inmenu = FALSE;
menu_instance_t current_menu = {NULL, 0, 0, NULL, 0};

menuelement_t ielement(bool (*onclick)(struct _menuelement_t *obj), int x1, int y1, int width, char *text, uint8_t type, int setupkey) {
    menuelement_t e;
    e.onclick = onclick;
    e.y1 = y1;
    if (width != -1) {
        e.width = width;
    }
    else {
        if (text != NULL) {
            e.width = strlen(text) * 6;
        }
        else width = 0;
    }
    e.text = text;
    e.type = type;
    e.setupkey = setupkey;
    if (x1 != -1) {
        e.x1 = x1;
    }
    else {
        // centered horizontally on the 128-pixel wide screen
        e.x1 = 63 - e.width / 2;
    }
    return e;
}

menupage_t imenupage(menuelement_t *elements, uint8_t element_cnt) {
    menupage_t p;
    int i;
    p.elements = elements;
    p.element_cnt = element_cnt;
    p.has_selectable = FALSE;
    // iterate and check whether the menu has any selectables
    if (element_cnt > 0 && elements != NULL) {
        for (i = 0; i < element_cnt; i++) {
            if (ui_is_selectable(elements[i])) {
                p.has_selectable = TRUE;
            }
        }
    }
    return p;
}

menu_t imenu(menupage_t *pages, uint8_t page_cnt, menu_t *prev) {
    menu_t m;
    m.page_cnt = page_cnt;
    m.pages = pages;
    m.prev_menu = prev;

    return m;
}

int ui_getmenustatus(void) {
    if (!inmenu || current_menu.menu == NULL) {
        return MENU_CLOSED;
    }
    else return MENU_OPEN;
}

int ui_rendermenu(void) {
    uint8_t i, l;
    static char buf[24];
    static const unsigned char bulletin[3] = "\xe6\x9b";
    menuelement_t curr;

    if (!inmenu || current_menu.menu == NULL) return S_ENOMENU;

    if (current_menu.in_slider != NULL) {
        return ui_rendermenu_slider();
        return;
    }
    
    for (i = 0; i < current_menu.menu->pages[current_menu.page].element_cnt; i++) {
        curr = current_menu.menu->pages[current_menu.page].elements[i];
        if (curr.text != NULL) {
            if (curr.type == MENUELEMENT_SETUP_SLIDER) {
                PrintXY(curr.x1 + 2, curr.y1 + 2, bulletin, 0); // draw a little '>' character to indicate the submenu available
                PrintXY(curr.x1 + 8, curr.y1 + 2, (unsigned char *)curr.text, 0);
            }
            else {
                PrintXY(curr.x1 + 2, curr.y1 + 2, (unsigned char *)curr.text, 0);
            }
        }
        if (curr.type == MENUELEMENT_BUTTON) {
            // draw a border around the button
            Bdisp_DrawLineVRAM(curr.x1, curr.y1, curr.x1 + curr.width + 4, curr.y1);
            Bdisp_DrawLineVRAM(curr.x1, curr.y1, curr.x1, curr.y1 + 10);
            Bdisp_DrawLineVRAM(curr.x1 + curr.width + 4, curr.y1, curr.x1 + curr.width + 4, curr.y1 + 10);
            Bdisp_DrawLineVRAM(curr.x1, curr.y1 + 10, curr.x1 + curr.width + 4, curr.y1 + 10);
            // invert the inside of the button
            if (current_menu.selected == i) {
                Bdisp_AreaReverseVRAM(curr.x1 + 1, curr.y1 + 1, curr.x1 + curr.width + 3, curr.y1 + 10 - 1);
            }
        }
        if (curr.type == MENUELEMENT_SETUP_BOOL) {
            // create a little box
            Bdisp_AreaReverseVRAM(117, curr.y1 + 1, 125, curr.y1 + 9);
            Bdisp_AreaReverseVRAM(118, curr.y1 + 2, 124, curr.y1 + 8);
            if (setup_getval(curr.setupkey)) {
                /*Bdisp_DrawLineVRAM(118, curr.y1 + 6, 120, curr.y1 + 8);
                Bdisp_DrawLineVRAM(120, curr.y1 + 8, 124, curr.y1 + 4);*/ // ugly tick mark

                Bdisp_AreaReverseVRAM(119, curr.y1 + 3, 123, curr.y1 + 7); // fill in the middle, with some white left
            }
            // draw small vertical line (2 wide) to indicate selection
            if (current_menu.selected == i) {
                Bdisp_AreaReverseVRAM(curr.x1 - 1, curr.y1, curr.x1, curr.y1 + 10);
            }
        }
        if (curr.type == MENUELEMENT_SETUP_SLIDER) {
            l = snprintf_light(buf, 24, "%d", setup_getval(curr.setupkey));
            PrintXY(127 - (l)*6, curr.y1 + 2, buf, 0);
            // draw small vertical line (2 wide) to indicate selection
            if (current_menu.selected == i) {
                Bdisp_AreaReverseVRAM(curr.x1 - 1, curr.y1, curr.x1, curr.y1 + 10);
            }
        }
        if (curr.type == MENUELEMENT_TITLE) {
            Bdisp_DrawLineVRAM(1, curr.y1 + 10, 126, curr.y1 + 10);
        }
    }
    // draw arrow icons in the top right to indicate available pages
    if (current_menu.menu->page_cnt > 1) {
        // right
        if (current_menu.page < current_menu.menu->page_cnt-1) {
            Bdisp_DrawLineVRAM(124, 1, 124, 5);
            Bdisp_DrawLineVRAM(124, 1, 126, 3);
            Bdisp_DrawLineVRAM(125, 4, 125, 3);
        }
        // left
        if (current_menu.page > 0) {
            Bdisp_DrawLineVRAM(122, 1, 122, 5);
            Bdisp_DrawLineVRAM(122, 1, 120, 3);
            Bdisp_DrawLineVRAM(121, 4, 121, 3);
        }
    }
    return S_SUCCESS;
}

int ui_rendermenu_slider(void) {
    int lower, upper, val;
    int sliderpx;
    menuelement_t *el = current_menu.in_slider;

    static char buf[24];

    el = current_menu.in_slider;

    setup_getbounds(el->setupkey, &lower, &upper);

    if (el->text != NULL) {
        PrintXY(el->x1 + 2, 1 + 2, (unsigned char *)el->text, 0);
    }
    Bdisp_DrawLineVRAM(1, 1 + 10, 126, 1 + 10);

    snprintf_light(buf, 24, "%d (%d\xe6\x91%d)", current_menu.sliderpos, lower, upper); // print curr (min>max)
    PrintXY(1, 13 + 2, (unsigned char *)buf, 0);

    // calculate X coordinate of slider
    sliderpx = (current_menu.sliderpos - lower) * 100 / (upper - lower);

    // draw slider line
    Bdisp_DrawLineVRAM(14, 29, 114, 29);
    Bdisp_DrawLineVRAM(14, 30, 114, 30);

    // draw slider handle
    Bdisp_DrawLineVRAM(sliderpx + 14, 26, sliderpx + 14, 33);
    Bdisp_DrawLineVRAM(sliderpx + 15, 26, sliderpx + 15, 33);

    return S_SUCCESS;
}

bool ui_is_selectable(menuelement_t e) {
    return e.type == MENUELEMENT_BUTTON || e.type == MENUELEMENT_SETUP_BOOL || e.type == MENUELEMENT_SETUP_SLIDER;
}

int ui_prevpage(void) {
    if (current_menu.page > 0) {
        current_menu.page--;
        if (ui_initpage() != S_SUCCESS) return S_EGENERIC;
    }

    return S_SUCCESS;
}

int ui_nextpage(void) {
    if (current_menu.page < current_menu.menu->page_cnt-1) {
        current_menu.page++;
        if (ui_initpage() != S_SUCCESS) return S_EGENERIC;
    }

    return S_SUCCESS;
}

int ui_initpage(void) {
    // find first selectable element in menu if it exists
    if (current_menu.menu->pages[current_menu.page].has_selectable) {
        current_menu.selected = 0;
        while (current_menu.selected < current_menu.menu->pages[current_menu.page].element_cnt) {
            if (ui_is_selectable(current_menu.menu->pages[current_menu.page].elements[current_menu.selected])) break;
            current_menu.selected++;
        }
    }
    else {
        current_menu.selected = -1;
    }
    return S_SUCCESS;
}

int ui_entermenu(menu_t *menu) {
    current_menu.menu = menu;
    if (menu != NULL) {
        current_menu.page = 0;
        ui_initpage();
        inmenu = TRUE;
    }
    else {
        // invalid menu, or exiting one with no parent
        inmenu = FALSE;
    }
    return S_SUCCESS;
}

int ui_closemenu(void) {
    return ui_entermenu(current_menu.menu->prev_menu);
}

// used to wait for key release
static bool done_next = FALSE;
static bool done_prev = FALSE;
static bool done_nextpage = FALSE;
static bool done_prevpage = FALSE;
static bool done_exec = FALSE;
static bool done_close = FALSE;

void menu_keyboard_handler(void) {
    int lower, upper;
    if (!inmenu || current_menu.menu == NULL) return;

    if (IsKeyDown(KEY_CTRL_DOWN)) {
        if (!done_next) {
            if (current_menu.in_slider == NULL) {
                ui_nextbutton();
            }
            done_next = TRUE;
        }
    }
    else done_next = FALSE;

    if (IsKeyDown(KEY_CTRL_UP)) {
        if (!done_prev) {
            if (current_menu.in_slider == NULL) {
                ui_prevbutton();
            }
            done_prev = TRUE;
        }
    }
    else done_prev = FALSE;

    if (IsKeyDown(KEY_CTRL_RIGHT)) {
        if (!done_nextpage) {
            if (current_menu.in_slider != NULL) {
                current_menu.sliderpos++;
            }
            else {
                ui_nextpage();
            }
            done_nextpage = TRUE;
        }
    }
    else done_nextpage = FALSE;

    if (IsKeyDown(KEY_CTRL_LEFT)) {
        if (!done_prevpage) {
            if (current_menu.in_slider != NULL) {
                current_menu.sliderpos--;
            }
            else {
                ui_prevpage();
            }
            done_prevpage = TRUE;
        }
    }
    else done_prevpage = FALSE;
    
    if (current_menu.in_slider != NULL) {
        setup_getbounds(current_menu.in_slider->setupkey, &lower, &upper);
        
        if (current_menu.sliderpos < lower) current_menu.sliderpos = lower;
        if (current_menu.sliderpos > upper) current_menu.sliderpos = upper;
    }

    if (IsKeyDown(KEY_CTRL_EXE)) {
        if (!done_exec) {
            if (current_menu.in_slider == NULL) {
                if (current_menu.selected != -1) {
                    if (current_menu.menu->pages[current_menu.page].elements[current_menu.selected].onclick != NULL) {
                        current_menu.menu->pages[current_menu.page].elements[current_menu.selected].onclick(&current_menu.menu->pages[current_menu.page].elements[current_menu.selected]);
                    }
                }
            }
            done_exec = TRUE;
        }
    }
    else done_exec = FALSE;

    if (IsKeyDown(KEY_CTRL_EXIT)) {
        if (!done_close) {
            if (current_menu.in_slider != NULL) {
                setup_setval(current_menu.in_slider->setupkey, current_menu.sliderpos);
                current_menu.in_slider = NULL;
            }
            else {
                ui_closemenu();
            }
            done_close = TRUE;
        }
    }
    else done_close = FALSE;
}

int ui_nextbutton(void) {
    if (current_menu.selected != -1) {
        do {
            current_menu.selected++;
            if (current_menu.selected >= current_menu.menu->pages[current_menu.page].element_cnt) {
                current_menu.selected = 0;
            }
        } while (!ui_is_selectable(current_menu.menu->pages[current_menu.page].elements[current_menu.selected]));
        return S_SUCCESS;
    }
    else {
        return S_NOBUTTON;
    }
}

int ui_prevbutton(void) {
    if (current_menu.selected != -1) {
        do {
            current_menu.selected--;
            if (current_menu.selected < 0) {
                current_menu.selected = current_menu.menu->pages[current_menu.page].element_cnt - 1;
            }
        } while (!ui_is_selectable(current_menu.menu->pages[current_menu.page].elements[current_menu.selected]));
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

extern void quit(void);

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

bool onclick_setup_slider(menuelement_t *el) {
    current_menu.in_slider = el;
    current_menu.sliderpos = (int)setup_getval(el->setupkey);
    return TRUE;
}