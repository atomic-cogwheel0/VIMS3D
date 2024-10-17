#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "VIMS_defs.h"

// 2-bit pixels
#define C_WHITE 0
#define C_BLACK 1
#define C_TRANSPARENT 2
#define C_UNUSED 3

// data for a single texture
typedef struct {
	const uint8_t w; // width
	const uint8_t h; // height
	const unsigned int u_tile_size : 4; // number of repeats on the X axis
	const unsigned int v_tile_size : 4; // number of repeats on the Y axis
	const byte *tx_data; // array of bytes; each byte is 4 pixels; iterated horizontally
} texture_t;

typedef texture_t *texture_ptr_t;

#endif