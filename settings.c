#include "settings.h"

#include "fxlib.h"

int setup_arr[SETUP_CNT];
static int setup_arr_default[SETUP_CNT] = {0, 0, 1, 0}; // DRAW_TEXTURES is on

int RecreateFile(char *fname) {
	Bfile_DeleteMainMemory(fname);
	Bfile_CreateMainMemory(fname);
	return Bfile_OpenMainMemory(fname);
}

void setup_save() {
	int handle = 0;
	int i;

	handle = RecreateFile("SETUP");

	// write all variables serialized
	for (i = 0; i < SETUP_CNT; i++) {
		Bfile_WriteFile(handle, &setup_arr[i], 4);
	}

	Bfile_CloseFile(handle);
}

void setup_load() {
	int handle = 0;
	unsigned int pos = 0;
	int i;

	handle = Bfile_OpenMainMemory("SETUP");

	// load all variables
	for (i = 0; i < SETUP_CNT; i++) {
		// set default on read error
		if (Bfile_ReadFile(handle, &setup_arr[i], 4, pos) < 0) {
			setup_arr[i] = setup_arr_default[i];
		}
		pos += 4;
	}
	
	Bfile_CloseFile(handle);
}

void setup_setval(uint8_t key, int val) {
    setup_arr[key] = val;
}

int setup_getval(uint8_t key) {
    return setup_arr[key];
}