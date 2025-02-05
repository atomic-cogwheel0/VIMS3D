#include "raster.h"
#include "vec.h"

// texture IDs in raster.h

/*
TEXTUREMAP.C
 contains the textures for raster.c
 format:
  2-bit pixels packed into bytes (efficient storage)
  colors:
	0: white
	1: black
	2: transparent
	3: unused (currently renders as transparent)
*/

byte tx_o_white[]           = {85,85,64,1,64,1,64,1,64,1,64,1,64,1,85,85};
byte tx_o_black[]           = {64};
byte tx_o_checkerboard_8[]  = {68,68,17,17,68,68,17,17,68,68,17,17,68,68,17,17};
byte tx_o_checkerboard_4[]  = {17,68,17,68};
byte tx_o_stripes_b[]       = {68,68,68,68};
byte tx_o_stripes_w[]       = {17,17,17,17};
byte tx_o_topfilled[]       = {64,1,64,1};
byte tx_o_tanktrack[]       = {69,81,69,81,64,1,69,81,69,81,64,1,69,81,69,81};
byte tx_o_tankfront[]       = {85,85,64,1,69,81,69,81,69,81,69,81,64,1,85,85};
byte tx_o_tanktop[]         = {85,85,64,1,69,81,68,17,68,17,69,81,64,1,85,85};

byte tx_t_transparent_square[]  = {85,85,106,169,106,169,106,169,106,169,106,169,106,169,85,85};
byte tx_t_person[]              = {170,170,170,169,106,170,170,170,170,170,170,84,21,170,170,170,170,170,169,4,16,106,170,170,170,170,164,1,64,26,170,170,170,170,164,0,0,26,170,170,170,170,149,85,85,86,170,170,170,170,169,84,21,106,170,170,170,170,165,0,0,90,170,170,170,170,165,16,68,90,170,170,170,170,169,1,64,106,170,170,170,170,169,4,16,106,170,170,170,170,169,81,69,106,170,170,170,170,170,84,21,170,170,170,170,170,170,85,85,170,170,170,170,170,170,149,86,170,170,170,170,170,170,165,90,170,170,170,170,170,85,80,5,85,170,170,170,165,1,64,1,64,90,170,170,144,1,64,1,64,6,170,170,144,1,64,1,64,6,170,170,144,1,64,1,64,6,170,170,144,65,64,1,65,6,170,170,144,17,64,1,68,6,170,170,144,17,64,1,68,6,170,170,144,17,64,1,68,6,170,170,144,17,64,1,68,6,170,170,144,17,64,1,68,6,170,170,148,17,64,1,68,22,170,170,144,17,64,1,68,6,170,170,144,17,64,1,68,6,170,170,144,17,64,1,68,6,170,170,144,21,85,85,84,6,170,170,144,16,64,1,4,6,170,170,144,21,84,21,84,6,170,170,149,80,5,80,5,86,170,170,144,21,80,5,84,6,170,170,64,21,85,85,84,1,170,170,68,85,85,85,85,17,170,170,145,21,85,85,84,70,170,170,145,21,85,85,84,70,170,170,166,85,85,85,85,154,170,170,170,149,85,85,86,170,170,170,170,85,85,85,85,170,170,170,170,85,86,149,85,170,170,170,170,85,86,149,85,170,170,170,170,85,86,149,85,170,170,170,170,85,86,149,85,170,170,170,170,85,86,149,85,170,170,170,170,85,90,165,85,170,170,170,169,85,90,165,85,106,170,170,169,85,90,165,85,106,170,170,169,85,90,165,85,106,170,170,170,85,106,169,85,170,170,170,170,85,106,169,85,170,170,170,170,85,106,169,85,170,170,170,170,85,106,169,85,170,170,170,170,85,106,169,85,170,170,170,170,85,106,169,85,170,170,170,170,85,106,169,85,170,170,170,170,85,106,169,85,170,170,170,169,85,170,170,85,106,170,170,149,85,106,169,85,86,170,170,85,85,106,169,85,85,170,170,85,85,106,169,85,85,170,};
byte tx_t_tree[]                = {170,170,170,170,170,170,170,170,170,170,170,86,170,170,170,170,170,170,165,69,170,169,86,170,170,169,84,1,85,85,85,170,170,85,84,0,1,0,1,106,169,0,4,0,0,80,1,106,169,0,1,0,0,16,0,106,169,0,1,80,0,16,0,90,169,0,5,21,0,16,1,86,165,0,16,1,64,16,4,22,164,16,16,0,64,80,16,26,164,21,0,0,64,0,16,22,148,5,0,0,0,0,64,6,144,1,80,0,0,0,0,26,144,0,85,0,0,80,0,26,145,64,21,65,69,64,0,26,148,0,5,69,84,0,0,22,165,0,1,85,84,16,4,70,165,1,0,85,84,16,84,6,165,1,0,21,80,5,80,6,169,4,0,21,80,21,64,6,169,80,5,85,80,85,0,26,169,0,17,5,84,85,64,22,169,64,80,5,85,84,80,22,170,85,64,1,85,80,20,6,170,169,0,1,85,64,4,6,170,169,1,1,85,4,0,22,170,165,5,5,85,5,64,90,170,164,0,21,85,0,85,106,170,165,0,85,84,0,86,170,170,169,64,21,85,85,106,170,170,169,80,21,85,90,170,170,170,165,17,85,86,170,170,170,170,84,21,101,86,170,170,170,170,64,81,165,86,170,170,170,170,80,5,165,86,170,170,170,170,149,22,149,86,165,86,170,170,169,85,149,86,148,69,170,170,170,149,85,86,80,1,106,170,170,165,85,86,64,84,106,170,170,169,85,85,69,81,106,170,170,169,85,85,85,85,170,170,170,170,85,85,85,170,170,170,170,170,85,85,86,170,170,170,170,170,149,85,170,170,170,170,170,170,149,86,170,170,170,170,170,170,85,86,170,170,170,170,170,170,85,90,170,170,170,170,170,170,85,90,170,170,170,170,170,170,85,90,170,170,170,170,170,170,85,90,170,170,170,170,170,170,85,86,170,170,170,170,170,170,149,86,170,170,170,170,170,170,149,86,170,170,170,170,170,170,149,86,170,170,170,170,170,170,85,85,170,170,170,170,170,170,85,85,170,170,170,170,170,170,85,85,170,170,170,170,170,169,85,85,170,170,170,170,170,169,85,85,106,170,170,170,170,169,85,85,106,170,170,170,170,169,85,85,106,170,170,170,170,169,85,85,106,170,170,170,170,169,85,85,106,170,170,};

byte tx_anim_flash[]		= {20,
							   65};
byte tx_anim_arrow[]		= {170,170,170,169,170,170,150,165,85,90,85,85,101,85,90,170,150,170,169,170,170,170,170,170,170,
							   170,170,170,169,170,170,150,165,85,26,64,0,101,85,26,170,150,170,169,170,170,170,170,170,170,};

tx_data_t textures[TX_CNT] = {
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
	{32, 64, 1, 1, tx_t_person},
	{32, 64, 1, 1, tx_t_tree},
	{2, 2, 1, 1, tx_anim_flash},
	{10, 10, 1, 1, tx_anim_arrow},
};