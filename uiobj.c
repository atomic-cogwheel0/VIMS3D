#include "uiobj.h"

int onclick_closemenu(void *unused) {
    ui_closemenu();
}

int onclick_quit(void *unused) {
    quit();
}