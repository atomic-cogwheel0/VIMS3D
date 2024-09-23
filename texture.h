#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "VIMS_defs.h"

#define C_WHITE 0
#define C_BLACK 1
#define C_TRANSPARENT 2

typedef struct {
	const uint8_t w;
	const uint8_t h;
	const unsigned int u_tile_size  :4;
	const unsigned int v_tile_size  :4;
	const byte *tx_data;
} texture_t;

typedef texture_t *texture_ptr_t;

#endif