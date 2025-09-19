#ifndef SETTINGS_H
#define SETTINGS_H

#include "VIMS_defs.h"
#include "fixed.h"

// the variables in order in the setup array
#define SETUP_BOOL_WIREFRAME 0
#define SETUP_BOOL_DRAWAREA 1
#define SETUP_BOOL_TEXTURES 2
#define SETUP_BOOL_SAVEPLAYER 3

#define SETUP_INT_ROTSPEED 4
#define SETUP_INT_MOVESPEED 5

// count of setup vars
#define SETUP_CNT 6

// deletes, creates and opens file, returns handle
int RecreateFile(char *fname);

// save to main memory @VIMS3D/SETUP
void setup_save(void);
// load from main memory @VIMS3D/SETUP
void setup_load(void);

// set the value the setup variable with the given key in the global arr
void setup_setval(uint8_t key, uint32_t val);

// get variable from global setup arr
uint32_t setup_getval(uint8_t key);

// fill arguments with bounds of given key
void setup_getbounds(uint8_t key, uint32_t *min, uint32_t *max);

#endif