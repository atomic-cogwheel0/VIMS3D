#ifndef SETTINGS_H
#define SETTINGS_H

#include "VIMS_defs.h"
#include "fixed.h"

// the variables in order in the setup array
enum setup_key_id {
    SETUP_BOOL_WIREFRAME = 0,
    SETUP_BOOL_DRAWAREA,
    SETUP_BOOL_TEXTURES,
    SETUP_BOOL_SAVEPLAYER,

    SETUP_INT_ROTSPEED,
    SETUP_INT_MOVESPEED,
    SETUP_INT_FOV,

    SETUP_BOOL_AFFINE,

    SETUP_CNT // count of setup vars, must be at the end of the enum
};

// deletes, creates and opens file, returns handle
int RecreateFile(char *fname);

// save to main memory @VIMS3D/SETUP
void setup_save(void);
// load from main memory @VIMS3D/SETUP
void setup_load(void);

// set the value the setup variable with the given key in the global arr
void setup_setval(enum setup_key_id key, int32_t val);

// get variable from global setup arr
int32_t setup_getval(enum setup_key_id key);

// fill arguments with bounds of given key
void setup_getbounds(enum setup_key_id key, int32_t *min, int32_t *max);

// fill arguments with step size of given key
void setup_getstep(enum setup_key_id key, int32_t *step);

#endif