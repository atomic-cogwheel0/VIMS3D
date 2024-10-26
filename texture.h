#ifndef TEXTURE_H
#define TEXTURE_H

#include "VIMS_defs.h"

// 2-bit pixels
#define C_WHITE 0
#define C_BLACK 1
#define C_TRANSPARENT 2
#define C_UNUSED 3

// data for a single texture
typedef struct {
	uint8_t w; // width
	uint8_t h; // height
	unsigned int u_tile_size : 4; // number of repeats on the X axis
	unsigned int v_tile_size : 4; // number of repeats on the Y axis
	byte *tx_data; // array of bytes; each byte is 4 pixels; iterated horizontally
} texture_t;

typedef texture_t *texture_ptr_t;

#endif