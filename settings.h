#ifndef SETTINGS_H
#define SETTINGS_H

#include "VIMS_defs.h"

// the variables in order in the setup array
#define SETUP_BOOL_WIREFRAME 0
#define SETUP_BOOL_DRAWAREA 1
#define SETUP_BOOL_TEXTURES 2
#define SETUP_BOOL_SAVEPLAYER 3

// count of setup vars
#define SETUP_CNT 4

// deletes, creates and opens file, returns handle
int RecreateFile(char *fname);

// save to main memory @VIMS3D/SETUP
void setup_save(void);
// load from main memory @VIMS3D/SETUP
void setup_load(void);

// set the value the setup variable with the given key in the global arr
void setup_setval(uint8_t key, int val);
// get variable from global setup arr
int setup_getval(uint8_t key);

#endif