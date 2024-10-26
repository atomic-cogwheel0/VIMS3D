#ifndef RASTER_H
#define RASTER_H

#include <stdlib.h>
#include "fxlib.h"

#include "vec.h"
#include "texture.h"

// syscalls
unsigned int RTC_GetTicks(void);
int Bdisp_SYS_FastDrawLineVRAM(int x1, int y1, int x2, int y2);

// number of triangles the subsys can handle at once
#define MAX_TRIANGLES 72

// texture ids
#define TX_WHITE 0
#define TX_BLACK 1
#define TX_CHECKERBOARD_8 2
#define TX_CHECKERBOARD_4 3
#define TX_STRIPES_A 4
#define TX_STRIPES_B 5
#define TX_TOPFILLED 6
#define TX_TANKTRACK 7
#define TX_TANKFRONT 8
#define TX_TANKTOP 9

#define TX_TRANSPARENT_SQUARE 10
#define TX_PERSON 11
#define TX_TREE 12

// number of textures
#define TX_CNT 13

// text rendering function flags
#define TEXT_SMALL 0x1
#define TEXT_LARGE 0x2
#define TEXT_INVERTED 0x4

// subsystem states
#define SUBSYS_ERR 0
#define SUBSYS_UP 1
#define SUBSYS_DOWN 2

// NEEDS CALLING (allocate bufs)
int g_init(void);
// deallocate all buffers (MAKES THE RASTER SYSTEM UNUSABLE)
void g_dealloc(void);
// get status
int g_getstatus(void);

// returns pointer to the depth buffer (128*64 arr of int16_t)
int16_t **g_getdepthbuf(void);

// draw the horizon (returns number of pixels drawn OR error code)
int g_draw_horizon(camera *cam);
// rasterize all triangles currently in the buffer (returns number of triangles drawn OR error code)
// zero_offset is the offset of tris[] from the world center, pos is the position they are transformed to
int g_rasterize_triangles(trianglef *tris, texture_ptr_t *textures, int len, camera cam, position pos, vec3f zero_offset);

// draw text at a position in 3D space (pos is the top left corner) (overwrite!) (NEEDS g_init())
int g_text3d(camera *cam, unsigned char *text, vec3f pos, unsigned int params);
// draw text onto the screen (overwrite!) (does not need g_init())
int g_text2d(unsigned char *text, unsigned int x, unsigned int y, unsigned int params);
// draw a texture (with tiling), pixel-by-pixel, (x & y are the screenspace coords of the top left corner) (overwrite!) (does not need g_init())
int g_texture2d(texture_ptr_t tx, unsigned int x, unsigned int y);

#endif