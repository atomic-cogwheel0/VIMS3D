#include "raster.h"

/*
 RASTER.C
  functions for adding, removing and rendering triangles and additional drawing functions
  provides the core of the rendering process

  using the higher level mesh system is preferred to raw triangles
*/

// depth buffer: an array 128*64=8192 depth values, every pixel has a 11+5 bit fixed-point depth value
int16_t *depthbuf;

// macro to access the depth buffer
#define DEPTHBUF_AT(x, y) depthbuf[x*64 + y]

static int g_status = SUBSYS_DOWN;

static void *vram;

// initialize all buffers (static alloc for buffers isn't possible)
int g_init(void) {
	if (g_status != SUBSYS_DOWN) return S_EALREADYINITED;

	// allocate the depth buffer
	if (!isSH4()) {
		depthbuf = (int16_t *) malloc(128*64*sizeof(int16_t));
		if (depthbuf == NULL) {
			// cancel all previous allocations and return
			g_dealloc();
			g_status = SUBSYS_ERR;
			return S_EALLOC;
		}
	}
	else {
		// use 16 kbytes of XYRAM as the depth buffer
		depthbuf = (int16_t *) 0xe500e000;
	}

	vram = GetVRAMAddress();

	g_status = SUBSYS_UP;
	return S_SUCCESS;
}

void g_dealloc(void) {
	// free all buffers
	if (!isSH4() && depthbuf != NULL) {
		free(depthbuf);
		depthbuf = NULL;
	}
	g_status = SUBSYS_DOWN;
}

int g_getstatus(void) {
	return g_status;
}

int16_t *g_getdepthbuf(void) {
	return (int16_t *)depthbuf;
}

int g_clr_depthbuf(void) {
	int dx, dy;
	if (g_status != SUBSYS_UP) return S_EDOWN;

	for (dx = 0; dx < 128; dx++) {
		for (dy = 0; dy < 64; dy++) {
			DEPTHBUF_AT(dx, dy) = 0x7FFF; // FIXED16_MAX
		}
	}

	return S_SUCCESS;
}

int g_draw_horizon(camera *cam) {
	int sspace_horiz_y;
	if (g_status != SUBSYS_UP) return S_EDOWN;
	// calculate onscreen y coordinate of the horizon line
	sspace_horiz_y = f2int(mulff(sin_f(-cam->pitch), int2f(64))) + 32;
	// is on screen?
	if (sspace_horiz_y < 0 || sspace_horiz_y >= 64) {
		return 0;
	}
	// draw line FAST
	Bdisp_SYS_FastDrawLineVRAM(0, sspace_horiz_y, 127, sspace_horiz_y);
	return 128;
}

// define fallback texture (2*2 checkerboard)
byte tx_o_fallback[] = {65};
texture_t fallback_texture = {2, 2, 1, 1, tx_o_fallback};

// the One and Only Rendering(TM) function
// have fun :)

int g_rasterize_triangles(trianglef *tris, texture_ptr_t *textures, int len, camera cam, position pos, vec3f zero_offset) {
	int curr_tidx, xiterl, xiterr, xiter, yiter;
	int bbox_left, bbox_right, bbox_top, bbox_bottom; // bounding box (on screen)
	int tri_cnt = 0;
	fixed diff;
	texture_t tx;

	trianglef t;
	vec3f v0, v1, v2, a, b, c, nrm, ctot; // ctot is Camera TO Triangle
	vec3f v2_increment;
	fixed denom, dot00, dot01, dot02l, dot02r, dot11, dot12l, dot12r, u, v, ui, vi; // for barycentric
	fixed ozstep, uzstep, vzstep; // perspective correctness iterators
	fixed ozp, uzp, vzp, ozq, uzq, vzq, oza, ozb, vzb, ozc, uzc, ozi, uzi, vzi, zci; // practical variable naming, UV data calculation
	fixed v0x_sc, v1x_sc;

	fixed dot11_dot02, dot01_dot12, dot00_dot12, dot01_dot02; // incremented every loop
	fixed dot11_dot02l, dot01_dot12l, dot00_dot12l, dot01_dot02l; // values on the leftmost pixel of current row
	fixed dot11_dot02r, dot01_dot12r, dot00_dot12r, dot01_dot02r; // values on the rightmost pixel of current row
	fixed dot11_dot02_odelta, dot01_dot12_odelta, dot00_dot12_odelta, dot01_dot02_odelta; // outer (y-)loop deltas
	fixed dot11_dot02_idelta, dot01_dot12_idelta, dot00_dot12_idelta, dot01_dot02_idelta; // inner (x-)loop deltas

	fixed dot02_increment, dot12_increment;
 
	byte px;
	int px_offset;
	uint32_t u_mult, v_mult;

	int16_t depthval;

	if (g_status != SUBSYS_UP) return S_EDOWN;

// some stuff can get stupidly large for the ~tiny~ 20-bit fixeds
// downscale them, who needs precision anyways :)
#define OF_SC 4	//OverFlow downSCale constant

// we need precision for the texture calc :(
// 12 bits aren't really enough for precise calculations with inverses, so let's make them 16-bit!
#define ZREC_EXP 4 // exponent (spares some cycles with rshift instead of division)
#define ZREC_MULT (1<<ZREC_EXP)

	// iterate on every triangle in the buffer
	for (curr_tidx = 0; curr_tidx < len; curr_tidx++) {
		t = tris[curr_tidx];
		t = move_tri_by_vec(t, neg(zero_offset)); // shift to center of rotation
		t = transform_tri_to_pos(t, pos); // rotate and move to pos
		t = move_tri_by_vec(t, zero_offset); // shift back
		t = transform_tri_to_camera(t, cam); // transform to camera

#ifdef BENCHMARK_RASTER
		{
		int i;
		for (i = 0; i < BENCHMARK_RASTER; i++) {
#endif

		nrm = normal(t);
		ctot = t.a;

		// cull (first backface, then near-plane)
		if (dotp(ctot, nrm) >= 0 || (t.a.z < int2f(1) || t.b.z < int2f(1) || t.c.z < int2f(1)))
			continue;

		// never divide by 0, check even when near-plane culling is on, don't want stuff to go wrong
		if (t.a.z == 0) {	
			t.a.z = FIXED_EPSILON;
		}
		if (t.b.z == 0) {	
			t.b.z = FIXED_EPSILON;
		}
		if (t.c.z == 0) {	
			t.c.z = FIXED_EPSILON;
		}
	
		// map to on-screen '2d' coordinates
		a = ivec3f(divff(mulff(t.a.x, int2f(64)), -t.a.z) + int2f(64),
                   divff(mulff(t.a.y, int2f(64)), -t.a.z) + int2f(32),
                   0);
		b = ivec3f(divff(mulff(t.b.x, int2f(64)), -t.b.z) + int2f(64),
                   divff(mulff(t.b.y, int2f(64)), -t.b.z) + int2f(32),
                   0);
		c = ivec3f(divff(mulff(t.c.x, int2f(64)), -t.c.z) + int2f(64),
                   divff(mulff(t.c.y, int2f(64)), -t.c.z) + int2f(32),
                   0);

		// get bounding box (extremes)
		bbox_left   = f2int(min(min(a.x, b.x), c.x));
		bbox_top    = f2int(min(min(a.y, b.y), c.y));
		bbox_right  = f2int(max(max(a.x, b.x), c.x));
		bbox_bottom = f2int(max(max(a.y, b.y), c.y));

		// is it entirely offscreen? (sanity check)
		if (bbox_right < 0 || bbox_left > 127 || bbox_bottom < 0 || bbox_top > 63) continue;

		bbox_left = clamp_i(bbox_left, 0, 127);
		bbox_top = clamp_i(bbox_top, 0, 63);
		bbox_right = clamp_i(bbox_right, 0, 127);
		bbox_bottom = clamp_i(bbox_bottom, 0, 63);

		tri_cnt++;

		// show missing texture instead of SYSTEM ERROR
		if (textures[curr_tidx] == NULL)
			tx = fallback_texture;
		else 
			tx = *textures[curr_tidx];

		// U and V are the barycentric coordinates of a pixel within the current triangle in screen space
		// their range is from 0 to 1, calculate with both tiling and texture size

		u_mult = tx.w * tx.u_tile_size;
		v_mult = tx.h * tx.v_tile_size;

		// the following code does barycentric coord calculation (u and v)
		// src: https://web.archive.org/web/20240910155457/https://blackpawn.com/texts/pointinpoly/
		
		// some (or most) of it has been ~mutilated~ for ~optimization~
		// this is NOT some especially maintainable nor good code. who cares? it's fast
		// especially the parts related to v2, the only changing vector here
		// its dot products are incremented in the for loop iterators!

		// u is on side CA and v is on side BA

		v0 = divvi(subvv(c, a), OF_SC);
		v1 = divvi(subvv(b, a), OF_SC);
		v0x_sc = divfi(v0.x, OF_SC); v1x_sc = divfi(v1.x, OF_SC);
		
		dot00 = dotp2(v0, v0);
		dot01 = dotp2(v0, v1);
		dot11 = dotp2(v1, v1);
		denom = mulff(dot00, dot11) - mulff(dot01, dot01);

		// 1/z and U/z V/z coordinates of vertices a, b and c (for perspective correction)
		// https://en.wikipedia.org/wiki/Texture_mapping#Perspective_correctness
		
		// ozX, uzX and vzX are One over Z, U/Z and V/Z calculated at point (X) of current triangle
		// U and V are used to interpolate them across the surface

		oza = divff(int2f(ZREC_MULT), t.a.z);
		ozb = divff(int2f(ZREC_MULT), t.b.z);
		ozc = divff(int2f(ZREC_MULT), t.c.z);
		
		vzb = mulff(int2f(u_mult), ozb); // V is max in point B, 0 in point C
		uzc = mulff(int2f(v_mult), ozc); // U is max in point C, 0 in point B

		// break the dot products down to simple additions in the main loop
		// first, calculate v2's values on the left and right sides
		v2.y = divfi(int2f(bbox_top) - a.y, OF_SC);

		v2.x = divfi(int2f(bbox_left) - a.x, OF_SC);
		dot02l = dotp2(v0, v2);
		dot12l = dotp2(v1, v2);

		v2.x = divfi(int2f(bbox_right) - a.x, OF_SC);
		dot02r = dotp2(v0, v2);
		dot12r = dotp2(v1, v2);

		// get increments on every y-loop
		v2_increment.x = 0;
		v2_increment.y = divfi(int2f(1), OF_SC);
		dot02_increment = dotp2(v0, v2_increment);
		dot12_increment = dotp2(v1, v2_increment);

		// deltas in the y-lopp
		dot11_dot02_odelta = mulff(dot11, dot02_increment);
		dot01_dot12_odelta = mulff(dot01, dot12_increment);
		dot00_dot12_odelta = mulff(dot00, dot12_increment);
		dot01_dot02_odelta = mulff(dot01, dot02_increment);

		// deltas in the x-loops
		dot11_dot02_idelta = mulff(dot11, v0x_sc);
		dot01_dot12_idelta = mulff(dot01, v1x_sc);
		dot00_dot12_idelta = mulff(dot00, v1x_sc);
		dot01_dot02_idelta = mulff(dot01, v0x_sc);

		// leftmost value of the dotprods in the current y-loop
		dot11_dot02l = mulff(dot11, dot02l);
		dot01_dot12l = mulff(dot01, dot12l);
		dot00_dot12l = mulff(dot00, dot12l);
		dot01_dot02l = mulff(dot01, dot02l);

		// rightmost value of the dotprods in the current y-loop
		dot11_dot02r = mulff(dot11, dot02r);
		dot01_dot12r = mulff(dot01, dot12r);
		dot00_dot12r = mulff(dot00, dot12r);
		dot01_dot02r = mulff(dot01, dot02r);

		// 1. PRECALCULATE everything that can be precalculated!
		// 2. If needed, split vectors into coordinates and use them separately!
		// 3. Remember what dot products are, avoid recalculating them if only a single coordinate has changed!
		// 4. INTERPOLATE everything that can be interpolated!
		//		- Sun Tzu, The Art of War

		// iterate by row, find leftmost and rightmost pixels and calculate 1/Z, U/Z and V/Z at these points (_zP is left, _zQ is right)
		//  then interpolate between by column
		//  Ui and Vi are the real texture coordinates
		//  1/Z, 1/Ui and 1/Vi are interpolated because they are linear across the surface in screen space

		for (yiter = bbox_top; yiter <= bbox_bottom; yiter++) {
			// find leftmost
			// increment leftmost dotp's and set up the loop iterators
			dot11_dot02 = (dot11_dot02l += dot11_dot02_odelta);
			dot01_dot12 = (dot01_dot12l += dot01_dot12_odelta);
			dot00_dot12 = (dot00_dot12l += dot00_dot12_odelta);
			dot01_dot02 = (dot01_dot02l += dot01_dot02_odelta);

			for (xiterl = bbox_left; xiterl <= bbox_right; xiterl++) {
				dot11_dot02 += dot11_dot02_idelta;
				dot01_dot12 += dot01_dot12_idelta;
				dot00_dot12 += dot00_dot12_idelta;
				dot01_dot02 += dot01_dot02_idelta;
				u = dot11_dot02 - dot01_dot12;
				if (u < 0) continue;
				v = dot00_dot12 - dot01_dot02;
				if (v < 0) continue;

				if (u + v < denom) goto found_left;
			}
			continue; // hasn't found anything

			found_left:
				u = divff(u, denom);
				v = divff(v, denom);
				// interpolate
				ozp = mulff(ozb, v) + mulff(ozc, u) + mulff(oza, int2f(1) - u - v);
				uzp = mulff(uzc, u);
				vzp = mulff(vzb, v);

			// find rightmost, same stuff but different
			// increment rightmost dotp's and set up the loop iterators
			dot11_dot02 = (dot11_dot02r += dot11_dot02_odelta);
			dot01_dot12 = (dot01_dot12r += dot01_dot12_odelta);
			dot00_dot12 = (dot00_dot12r += dot00_dot12_odelta);
			dot01_dot02 = (dot01_dot02r += dot01_dot02_odelta);

			for (xiterr = bbox_right; xiterr >= xiterl; xiterr--) {
				dot11_dot02 -= dot11_dot02_idelta;
				dot01_dot12 -= dot01_dot12_idelta;
				dot00_dot12 -= dot00_dot12_idelta;
				dot01_dot02 -= dot01_dot02_idelta;
				u = dot11_dot02 - dot01_dot12;
				if (u < 0) continue;
				v = dot00_dot12 - dot01_dot02;
				if (v < 0) continue;

				if (u + v < denom) goto found_right;
			}
			continue; // hasn't found anything

			found_right:
				u = divff(u, denom);
				v = divff(v, denom);
				ozq = mulff(ozb, v) + mulff(ozc, u) + mulff(oza, int2f(1) - u - v);
				uzq = mulff(uzc, u);
				vzq = mulff(vzb, v);

			if ((xiterr-xiterl) <= 0) continue; // probably a good idea to not render

			// number of pixels to interpolate over
			diff = int2f(xiterr-xiterl);

			ozstep = divff(ozq-ozp, diff);
			uzstep = divff(uzq-uzp, diff);
			vzstep = divff(vzq-vzp, diff);

			// interpolate between values in current row
			for (ozi = ozp, uzi = uzp, vzi = vzp, xiter = xiterl; xiter <= xiterr; xiter++, ozi+=ozstep, uzi+=uzstep, vzi+=vzstep) {
				zci = divff(int2f(ZREC_MULT*ZREC_MULT), ozi);

				// zci is halved to handle greater distances
				depthval = (f2int(zci/2) << 5) | (((zci/2) & FIXED_FRAC_MASK) >> (FIXED_PRECISION-5)); // transform 22+10bit zci to 11+5bit depth

				// is current pixel closer than the last depth written there?
				if (DEPTHBUF_AT(xiter, yiter) > depthval) {
					ui = divshiftfi(mulff(uzi, zci), ZREC_EXP+ZREC_EXP); // only use of divshiftfi is here
					vi = divshiftfi(mulff(vzi, zci), ZREC_EXP+ZREC_EXP);

					// calculate pixel offset into the array (row by row)
					if (t.flip_texture) {
						// index coordinates from the bottom right instead of top left
						px_offset = (tx.h-1 - f2int(ui)%tx.h)*tx.w + (tx.w-1 - f2int(vi)%tx.w);
					}
					else {
						px_offset = (f2int(ui)%tx.h)*tx.w + (f2int(vi)%tx.w);
					}
					// ensure array access does not cause an invalid dereference
					if (px_offset >= tx.h*tx.w) continue;
					// extract pixel at given offset (2 bit pixel extracted from byte arr)
					px = (tx.tx_data[px_offset/4] & (3 << ((3 - (px_offset%4)) * 2))) >> ((3 - (px_offset%4)) * 2);
					// is transparency bit set?
					if (!(px & 2)) {
						SetPoint_VRAM(xiter, yiter, px & 1, vram);
						DEPTHBUF_AT(xiter, yiter) = depthval; // write new depthval
					}
				}
			}
		}
#ifdef BENCHMARK_RASTER
		}}
#endif
	}
	return tri_cnt;
}

int g_text3d(camera *cam, unsigned char *text, vec3f pos, unsigned int params) {
	int sspace_x, sspace_y;
	vec3f viewport_pos;

	if (text == NULL) return S_ENULLPTR;
	if (g_status != SUBSYS_UP) return S_EDOWN;

	viewport_pos = transform_vec_to_camera(pos, *cam);

	// find screenspace coordinates of top left corner
	sspace_x = f2int(divff(mulff(viewport_pos.x, int2f(64)), -viewport_pos.z)) + 64;
	sspace_y = f2int(divff(mulff(viewport_pos.y, int2f(64)), -viewport_pos.z)) + 32;

	// is in front of player (also near plane culling)
	if (!(viewport_pos.z < float2f(0.25f))) {	
		// is on screen?
		if (sspace_x <= 127 && sspace_x >= 0 && sspace_y <= 63 && sspace_y >= 0) {
			// pass drawing params over
			return g_text2d(text, sspace_x, sspace_y, params);
		}
	}
	return S_SUCCESS;
}

int g_text2d(unsigned char *text, unsigned int x, unsigned int y, unsigned int params) {
	if (text == NULL) return S_ENULLPTR;

	if (params & TEXT_SMALL) {
		PrintMini(x, y, text, (params & TEXT_INVERTED) ? MINI_REV : MINI_OVER);
	}
	else if (params & TEXT_LARGE) {
		PrintXY(x, y, text, (params & TEXT_INVERTED) ? MINI_REV : MINI_OVER);
	}
	return S_SUCCESS;
}

int g_texture2d(texture_ptr_t tx, unsigned int x, unsigned int y) {
	unsigned int xiter, yiter;
	byte px;
	int px_offset;
	uint32_t tiled_width, tiled_height;

	if (tx == NULL) return S_ENULLPTR;

	// if tx is tiled, draw it multiple times
	tiled_width = tx->w * tx->u_tile_size;
	tiled_height = tx->h * tx->v_tile_size;

	for (yiter = 0; yiter < tiled_height; yiter++) {
		for (xiter = 0; xiter < tiled_width; xiter++) {
			// add top left coords; is still onscreen?
			if (xiter+x >= 0 && xiter+x < 128 && yiter+y >= 0 && yiter+y < 64) {
				// calculate pixel offset into the array (row by row)
				px_offset = (yiter % tx->h) * tx->w + xiter % tx->w;
				// extract pixel at given offset (2 bit pixel extracted from byte arr)
				px = (tx->tx_data[px_offset/4] & (3 << ((3 - (px_offset%4)) * 2))) >> ((3 - (px_offset%4)) * 2);
				// is transparency bit set?
				if (!(px & 2)) {
					Bdisp_SetPoint_VRAM(xiter+x, yiter+y, px & 1);
				}
			}
		}
	}
	return S_SUCCESS;
}