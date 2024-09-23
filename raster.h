#ifndef _RASTER_H
#define _RASTER_H

#include "fxlib.h"
#include "stdlib.h"

#include "vec.h"
#include "texture.h"

int RTC_GetTicks(void);
unsigned char *GetVRAMAddress(void);
int Bdisp_SYS_FastDrawLineVRAM(int x1, int y1, int x2, int y2);

#define TX_CNT 13

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

#define MAX_TRIANGLES 100

#define G_SUCCESS 0
#define G_EALLOC -1
#define G_EEMPTY -2
#define G_EBUFFULL -3
#define G_ENEXIST -4

int g_init(void);
signed int g_addtriangle(trianglef t);
int g_removetriangle(uint32_t);
void g_coord(vec3f pos, fixed pitch, fixed yaw);
unsigned int g_draw_horizon(void);
unsigned int g_rasterize_buf(void);
void g_clrbuf(void);

#define TEXT_SMALL 0x1
#define TEXT_LARGE 0x2
#define TEXT_INVERTED 0x4

void g_text3d(unsigned char *text, vec3f pos, unsigned int params);
void g_text2d(unsigned char *text, unsigned int x, unsigned int y, unsigned int params);
void g_texture2d(texture_t *tx, unsigned int x, unsigned int y);

#endif