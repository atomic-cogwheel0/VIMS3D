#include "uiobj.h"

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