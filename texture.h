#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdlib.h>
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
	byte *pixels; // array of bytes; each byte is 4 pixels; serialized row by row
} tx_data_t;

// data for animation
typedef struct {
	uint8_t nframes; // number of frames in animation (1 means not animated; 0 is undefined)
	unsigned int frame_us; // number of microseconds each frame takes
	unsigned int us_elapsed; // number of microseconds since frame switch
	uint8_t frame; // currently displayed frame
	
	bool is_loop : 1; // should the animation loop?
	bool is_running : 1; // used to pause animation
} anim_data_t;

// a texture INSTANCE with unique animation data and possibly shared texture data
typedef struct {
	tx_data_t *texture;
	// if animated, texture->tx_data holds every frame, each frame comes immediately after the next; w, h and tile_sizes used normally
	anim_data_t anim;
} texture_t;

// init animated texture
texture_t *i_tx_anim(tx_data_t *tx, uint8_t nframes, unsigned int frame_us, bool loop, bool imm_start);
// init texture with no animation (single frame)
texture_t *i_tx_static(tx_data_t *tx);
// free and set *tx to NULL
void tx_free(texture_t **tx);

// create a new node for the texture (returns tx and stores status (NULLable))
texture_t *a_register_texture(texture_t *tx, int *status);
// figure out frames to display
void a_tick(uint32_t elapse_us);
// deallocate every registered texture
void a_dealloc(void);

int a_px_offset(texture_t *tx);

#endif