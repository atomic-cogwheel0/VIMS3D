#ifndef _RASTER_H
#define _RASTER_H

#include <stdlib.h>
#include "fxlib.h"

#include "vec.h"
#include "texture.h"

// syscalls
int RTC_GetTicks(void);
unsigned char *GetVRAMAddress(void);
int Bdisp_SYS_FastDrawLineVRAM(int x1, int y1, int x2, int y2);

#define TX_CNT 13

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

// text params
#define TEXT_SMALL 0x1
#define TEXT_LARGE 0x2
#define TEXT_INVERTED 0x4

// return codes (for raster.c and mesh.c too)
#define G_SUCCESS 0
#define G_EALLOC -1
#define G_EEMPTY -2
#define G_EBUFFULL -3
#define G_ENEXIST -4
#define G_ENULLPTR -5
#define G_EALREADYINITED -6
#define G_EDOWN -7

#define G_SUBSYS_ERR 0
#define G_SUBSYS_UP 1
#define G_SUBSYS_DOWN 2

typedef uint8_t interlace_param_t;

#define INTERLACE_MASK_ISON 1
#define INTERLACE_MASK_ROW  2

// NEEDS CALLING (allocate bufs)
int g_init(void);
// clear buffers, reset indexes
int g_clrbuf(void);
// deallocate buffers (MAKES THE RASTER SYSTEM UNUSABLE)
void g_dealloc(void);
// get status
int g_getstatus(void);

// returns pointer to depth buffer (128*64 arr of int16_t)
int16_t **g_getdepthbuf(void);

// add a triangle (returns id of added triangle)
tr_id_t g_addtriangle(trianglef t);
// remove a triangle by id
int g_removetriangle(tr_id_t id);

// set global variables
void g_coord(vec3f pos, fixed pitch, fixed yaw);

// draw the horizon (returns number of pixels drawn)
unsigned int g_draw_horizon(void);
// rasterize all triangles currently in buffer (returns number of triangles drawn)
unsigned int g_rasterize_buf(interlace_param_t interlace);

// draw text at a position in 3D space (overwrite!) (NEEDS g_init())
int g_text3d(unsigned char *text, vec3f pos, unsigned int params);
// draw text onto the screen (overwrite!) (does not need g_init())
int g_text2d(unsigned char *text, unsigned int x, unsigned int y, unsigned int params);
// draw a texture (with tiling), pixel-by-pixel, screenspace coords (overwrite!) (does not need g_init())
int g_texture2d(texture_ptr_t tx, unsigned int x, unsigned int y);

#endif