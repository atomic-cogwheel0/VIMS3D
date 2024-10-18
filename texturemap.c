#include "raster.h"
#include "vec.h"

// texture IDs in raster.h

/*
TEXTUREMAP.C
 contains the textures for raster.c
 format:
  2-bit pixels packed into bytes (memory limitations)
  colors:
	0: white
	1: black
	2: transparent
	3: unused(currently renders as transparent)
*/

byte tx_o_white[]			= {85,85,64,1,64,1,64,1,64,1,64,1,64,1,85,85};
byte tx_o_black[]			= {64};
byte tx_o_checkerboard_8[]	= {68,68,17,17,68,68,17,17,68,68,17,17,68,68,17,17};
byte tx_o_checkerboard_4[]	= {17,68,17,68};
byte tx_o_stripes_b[]		= {68,68,68,68};
byte tx_o_stripes_w[]		= {17,17,17,17};
byte tx_o_topfilled[]		= {64,1,64,1};
byte tx_o_tanktrack[]		= {69,81,69,81,64,1,69,81,69,81,64,1,69,81,69,81};
byte tx_o_tankfront[]		= {85,85,64,1,69,81,69,81,69,81,69,81,64,1,85,85};
byte tx_o_tanktop[]			= {85,85,64,1,69,81,68,17,68,17,69,81,64,1,85,85};

byte tx_t_transparent_square[]	= {85,85,106,169,106,169,106,169,106,169,106,169,106,169,85,85};
byte tx_t_person[]				= {165,90,145,70,148,22,165,90,169,106,149,86,101,89,101,89,101,89,101,89,166,154,166,154,166,154,166,154,166,154,150,150};
byte tx_t_tree[]				= {165,86,144,1,68,81,65,1,80,65,64,69,145,65,165,86,169,90,169,106,169,106,169,106,169,106,169,106,165,90,149,86};

texture_t textures[TX_CNT] = {
	{8, 8, 1, 1, tx_o_white},
	{1, 1, 1, 1, tx_o_black},
	{8, 8, 1, 1, tx_o_checkerboard_8},
	{4, 4, 1, 1, tx_o_checkerboard_4},
	{8, 2, 1, 4, tx_o_stripes_b},
	{8, 2, 1, 4, tx_o_stripes_w},
	{8, 2, 1, 4, tx_o_topfilled},
	{8, 8, 1, 1, tx_o_tanktrack},
	{8, 8, 1, 1, tx_o_tankfront},
	{8, 8, 1, 1, tx_o_tanktop},
	{8, 8, 1, 1, tx_t_transparent_square},
	{8, 16, 1, 1, tx_t_person},
	{8, 16, 1, 1, tx_t_tree}
};