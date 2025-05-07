#ifndef SETTINGS_H
#define SETTINGS_H

#include "VIMS_defs.h"

#define SETUP_BOOL_WIREFRAME 0
#define SETUP_BOOL_DRAWAREA 1
#define SETUP_BOOL_TEXTURES 2
#define SETUP_BOOL_SAVEPLAYER 3

#define SETUP_CNT 4

// returns handle
int RecreateFile(char *fname);

void setup_save();
void setup_load();

void setup_setval(uint8_t key, int val);
int setup_getval(uint8_t key);

#endif