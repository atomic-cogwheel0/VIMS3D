#include "settings.h"

#include "fxlib.h"

static int32_t setup_arr[SETUP_CNT];
static int setup_arr_default[SETUP_CNT] = {0, 0, 1, 0, 32, 10, 90, 0}; // DRAW_TEXTURES is on

int RecreateFile(char *fname) {
	Bfile_DeleteMainMemory((unsigned char *)fname);
	Bfile_CreateMainMemory((unsigned char *)fname);
	return Bfile_OpenMainMemory((unsigned char *)fname);
}

void setup_save() {
	int handle = 0;
	int i;

	handle = RecreateFile("SETUP");

	// write all variables serialized
	for (i = 0; i < SETUP_CNT; i++) {
		Bfile_WriteFile(handle, &setup_arr[i], sizeof(int32_t));
	}

	Bfile_CloseFile(handle);
}

void setup_load() {
	int handle = 0;
	unsigned int pos = 0;
	int i;

	handle = Bfile_OpenMainMemory((unsigned char *)"SETUP");

	// load all variables
	for (i = 0; i < SETUP_CNT; i++) {
		// set default on read error
		if (Bfile_ReadFile(handle, &setup_arr[i], sizeof(int32_t), pos) < 0) {
			setup_arr[i] = setup_arr_default[i];
		}
		pos += sizeof(int32_t);
	}
	
	Bfile_CloseFile(handle);
}

void setup_setval(enum setup_key_id key, int32_t val) {
    setup_arr[key] = val;
}

int32_t setup_getval(enum setup_key_id key) {
    return setup_arr[key];
}

void setup_getbounds(enum setup_key_id key, int32_t *min, int32_t *max) {
	if (min == NULL || max == NULL) return;

	switch(key) {
		case SETUP_INT_ROTSPEED:
			*min = 10;
			*max = 60;
			break;
		case SETUP_INT_MOVESPEED:
			*min = 1;
			*max = 20;
			break;
		case SETUP_INT_FOV:
			*min = 60;
			*max = 120;
			break;
		default:
			*min = 0;
			*max = 100;
			break;
	}
}

void setup_getstep(enum setup_key_id key, int32_t *step) {
	if (step == NULL) return;

	switch(key) {
		case SETUP_INT_FOV:
			*step = 5;
			break;
		default:
			*step = 1;
			break;
	}
}