#include "raster.h"

/*
 RASTER.C
  functions for adding, removing and rendering triangles and additional drawing functions
  provides the core of the rendering process

  using the higher level mesh system is preferred to raw triangles
*/

#define TBUF_SIZ MAX_TRIANGLES

// triangle buffer, triangle indexing
trianglef *tbuf;
unsigned short tbuf_idx = 0;
uuid_t g_uuid = 1;				// nothing gets g_uuid 0

// depth buffer: a 128*64 array, every pixel has a 11+5 bit fixed-point depth value
int16_t *depthbuf[64];

unsigned char *vram;

unsigned int g_status = SUBSYS_DOWN;

// initialize all buffers (static alloc for buffers isn't possible)
int g_init(void) {
	int i;

	if (g_status != SUBSYS_DOWN) return S_EALREADYINITED;

	tbuf = (trianglef *)malloc(TBUF_SIZ*sizeof(trianglef));

	if (tbuf == NULL) {
		g_status = SUBSYS_ERR;
		return S_EALLOC;
	}

	for (i = 0; i < 64; i++) {
		depthbuf[i] = (int16_t *) malloc(128*sizeof(int16_t));
		if (depthbuf[i] == NULL) {
			g_dealloc();
			g_status = SUBSYS_ERR;
			return S_EALLOC;
		}
	}
	g_status = SUBSYS_UP;

	g_clrbuf();

	vram = GetVRAMAddress();

	return S_SUCCESS;
}

// clear buffers, initialize 
int g_clrbuf(void) {
	if (g_status != SUBSYS_UP) return S_EDOWN;
	memset((char *)tbuf, 0, TBUF_SIZ*sizeof(trianglef));
	tbuf_idx = 0;
	g_uuid = 1;
	return S_SUCCESS;
}

void g_dealloc(void) {
	int i;
	if (tbuf != NULL) {
		free(tbuf);
		tbuf = NULL;
	}
	for (i = 0; i < 64; i++) {
		if (depthbuf[i] != NULL) {
			free(depthbuf[i]);
			depthbuf[i] = NULL;
		}
	}
	g_status = SUBSYS_DOWN;
}

int g_getstatus(void) {
	return g_status;
}

int16_t **g_getdepthbuf(void) {
	return (int16_t **)depthbuf;
}

uuid_t g_addtriangle(trianglef t) {
	if (g_status != SUBSYS_UP) return S_EDOWN;
	if (g_uuid < UUID_MAX) {
		if (tbuf_idx < TBUF_SIZ-1) {
			tbuf[tbuf_idx++] = itrianglef(t.a, t.b, t.c, t.tx, g_uuid, t.flip_texture);
			return g_uuid++;
		}
	}
	return S_EBUFFULL;
}

int g_removetriangle(uuid_t id) {
	int i, j;
	if (g_status != SUBSYS_UP) return S_EDOWN;
	for(i = 0; i < tbuf_idx; i++) {
		if (tbuf[i].id == id) {
			for (j = i; j < (tbuf_idx-1); j++) {
				tbuf[j] = tbuf[j+1];
			}
			tbuf_idx--;
			return S_SUCCESS;
		}
	}
	return S_ENEXIST;
}

int g_draw_horizon(camera *cam) {
	int sspace_horiz_y;
	if (g_status != SUBSYS_UP) return S_EDOWN;
	sspace_horiz_y = f2int(mulff(sin_f(-cam->pitch), int2f(64))) + 32;
	if (sspace_horiz_y < 0 || sspace_horiz_y >= 64) {
		return 0;
	}
	Bdisp_SYS_FastDrawLineVRAM(0, sspace_horiz_y, 127, sspace_horiz_y);
	return 128;
}

// define fallback texture (2*2 checkerboard)
byte tx_o_fallback[] = {65};
texture_t fallback_texture = {2, 2, 1, 1, tx_o_fallback};

// the One and Only Rendering(TM) function
// have fun :)

int g_rasterize_buf(camera *cam) {
	int curr_tidx, xiterl, xiterr, xiter, yiter;
	int bbox_left, bbox_right, bbox_top, bbox_bottom; // bounding box (on screen)
	int q_cnt = 0;
	fixed diff;

	trianglef t;
	vec3f v0, v1, v2, a, b, c, nrm, ctot; // ctot is Camera TO Triangle
	fixed denom, dot00, dot01, dot02, dot11, dot12, u, v, ui, vi; // for barycentric
	fixed ozstep, uzstep, vzstep; // perspective correctness iterators
	fixed ozp, uzp, vzp, ozq, uzq, vzq, oza, ozb, vzb, ozc, uzc, ozi, uzi, vzi, zci; // practical variable naming, UV data calculation
	fixed v0x_sc, v1x_sc;

	fixed dot11_dot02, dot01_dot12, dot00_dot12, dot01_dot02;
	fixed dot11_dot02_increment, dot01_dot12_increment, dot00_dot12_increment, dot01_dot02_increment;

	uint8_t px, pxl;
	uint16_t u_mult, v_mult;

	signed short depthval;

	if (g_status != SUBSYS_UP) return S_EDOWN;

// some stuff can get stupidly large for the ~tiny~ 20-bit fixeds
// downscale them, who needs precision anyways :)
#define OF_SC 4	//OverFlow downSCale constant

// we need precision for the texture calc :(
// 12 bits aren't really enough for precise calculations with inverses, so let's make them 16-bit!
#define ZREC_EXP 4 // exponent (spares some cycles with rshift instead of division)
#define ZREC_MULT (1<<ZREC_EXP)

	// iterate on every triangle in the buffer
	for (curr_tidx = 0; curr_tidx < tbuf_idx; curr_tidx++) {
		t = transform_tri_to_camera(tbuf[curr_tidx], *cam); // transform to camera

		nrm = normal(tbuf[curr_tidx]);
		ctot = subvv(tbuf[curr_tidx].a, cam->pos);

		// cull
		if (t.a.z < float2f(.5f) || t.b.z < float2f(.5f) || t.c.z < float2f(.5f) ||							// near-plane
		    (t.a.z > int2f(64) && t.b.z > int2f(64) && t.c.z > int2f(64)) ||								// far-plane
		    dotp(ctot, nrm) >= 0) {													// backface
			continue;
		}

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

		if (bbox_right < 0 || bbox_left > 127 || bbox_bottom < 0 || bbox_top > 63) continue;

		bbox_left = clamp_i(bbox_left, 0, 127);
		bbox_top = clamp_i(bbox_top, 0, 63);
		bbox_right = clamp_i(bbox_right, 0, 127);
		bbox_bottom = clamp_i(bbox_bottom, 0, 63);
		
		// is it entirely offscreen? (sanity check)

		q_cnt++;

		if (t.tx == NULL) t.tx = &fallback_texture;

		// U and V are the barycentric coordinates of a pixel within the current triangle in screen space
		// their range is from 0 to 1, calculate with both tiling and texture size

		u_mult = t.tx->w * t.tx->u_tile_size;
		v_mult = t.tx->h * t.tx->v_tile_size;

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

		// 1. PRECALCULATE everything that can be precalculated!
		// 2. If needed, split vectors into coordinates and use them separately!
		// 3. Remember what dot products are, avoid recalculating 'em if only a single coordinate has changed!
		// 4. INTERPOLATE everything that can be interpolated!
		//		- Sun Tzu, The Art of War

		// iterate by row, find leftmost and rightmost pixels and calculate 1/Z, U/Z and V/Z at these points (_zP is left, _zQ is right)
		//  then interpolate between by column
		//  Ui and Vi are the real texture coordinates
		//  1/Z, 1/Ui and 1/Vi are interpolated because they are linear across the surface in screen space

		for (yiter = bbox_top; yiter <= bbox_bottom; yiter += 1) {
			if (yiter < 0 || yiter >= 64) continue;

			// find leftmost
			v2 = divvi(subvv(ivec3f(int2f(bbox_left), int2f(yiter), 0), a), OF_SC);
			dot02 = dotp2(v0, v2);
			dot12 = dotp2(v1, v2);

			dot11_dot02 = mulff(dot11, dot02);
			dot01_dot12 = mulff(dot01, dot12);
			dot00_dot12 = mulff(dot00, dot12);
			dot01_dot02 = mulff(dot01, dot02);

			dot11_dot02_increment = mulff(dot11, v0x_sc);
			dot01_dot12_increment = mulff(dot01, v1x_sc);
			dot00_dot12_increment = mulff(dot00, v1x_sc);
			dot01_dot02_increment = mulff(dot01, v0x_sc);

			for (xiterl = bbox_left; xiterl <= bbox_right; xiterl++) {
				dot11_dot02 += dot11_dot02_increment;
				dot01_dot12 += dot01_dot12_increment;
				dot00_dot12 += dot00_dot12_increment;
				dot01_dot02 += dot01_dot02_increment;
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
			v2 = divvi(subvv(ivec3f(int2f(bbox_right), int2f(yiter), 0), a), OF_SC);
			dot02 = dotp2(v0, v2);
			dot12 = dotp2(v1, v2);

			dot11_dot02 = mulff(dot11, dot02);
			dot01_dot12 = mulff(dot01, dot12);
			dot00_dot12 = mulff(dot00, dot12);
			dot01_dot02 = mulff(dot01, dot02);

			dot11_dot02_increment = mulff(dot11, v0x_sc);
			dot01_dot12_increment = mulff(dot01, v1x_sc);
			dot00_dot12_increment = mulff(dot00, v1x_sc);
			dot01_dot02_increment = mulff(dot01, v0x_sc);

			for (xiterr = bbox_right; xiterr >= xiterl; xiterr--) {
				dot11_dot02 -= dot11_dot02_increment;
				dot01_dot12 -= dot01_dot12_increment;
				dot00_dot12 -= dot00_dot12_increment;
				dot01_dot02 -= dot01_dot02_increment;
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

			diff = int2f(xiterr-xiterl);

			ozstep = divff(ozq-ozp, diff);
			uzstep = divff(uzq-uzp, diff);
			vzstep = divff(vzq-vzp, diff);

			// interpolate between values in current row
			for (ozi = ozp, uzi = uzp, vzi = vzp, xiter = xiterl; xiter <= xiterr; xiter++, ozi+=ozstep, uzi+=uzstep, vzi+=vzstep) {
				zci = divff(int2f(ZREC_MULT*ZREC_MULT), ozi);

				depthval = (f2int(zci) << 5) | ((zci & FIXED_FRAC_MASK) >> (FIXED_PRECISION-5)); // transform 22+10bit zci to 11+5bit depth

				if (depthbuf[yiter][xiter] > depthval) {			
					ui = divshiftfi(mulff(uzi, zci), ZREC_EXP+ZREC_EXP); // only use of divshiftfi is here
					vi = divshiftfi(mulff(vzi, zci), ZREC_EXP+ZREC_EXP);

					// index coordinates from the bottom right instead of top left
					if (t.flip_texture) {
						pxl = (t.tx->h-1 - f2int(ui)%t.tx->h)*t.tx->w + (t.tx->w-1 - f2int(vi)%t.tx->w);
					}
					else {
						pxl = (f2int(ui)%t.tx->h)*t.tx->w + (f2int(vi)%t.tx->w);
					}
					px = (t.tx->tx_data[pxl>>2] & (3<<(2*(3-(pxl&3))))) >> (2*(3-(pxl&3))); // pxl>>2 = pxl/4; pxl&3 = pxl%4
					if (!(px & 2)) {
						Bdisp_SetPoint_VRAM(xiter, yiter, px & 1);
						depthbuf[yiter][xiter] = depthval;
					}
				}
			}
		}	
	}
	return q_cnt;
}

int g_text3d(camera *cam, unsigned char *text, vec3f pos, unsigned int params) {
	int sspace_x, sspace_y;
	vec3f viewport_pos;
	vec3f ctot;

	if (text == NULL) return S_ENULLPTR;
	if (g_status != SUBSYS_UP) return S_EDOWN;

	ctot = subvv(pos, cam->pos);
	viewport_pos = rot(subvv(pos, cam->pos), -cam->pitch, -cam->yaw);

	sspace_x = f2int(divff(mulff(viewport_pos.x, int2f(64)), -viewport_pos.z)) + 64;
	sspace_y = f2int(divff(mulff(viewport_pos.y, int2f(64)), -viewport_pos.z)) + 32;

	// is in view?
	if (!(viewport_pos.z < float2f(0.25f))) {	
		if (sspace_x <= 127 && sspace_x >= 0 && sspace_y <= 63 && sspace_y >= 0) {
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
	uint8_t px;
	unsigned int px_offset;
	unsigned int tiled_width, tiled_height;

	if (tx == NULL) return S_ENULLPTR;

	tiled_width = tx->w * tx->u_tile_size;
	tiled_height = tx->h * tx->v_tile_size;

	for (yiter = 0; yiter < tiled_height; yiter++) {
		for (xiter = 0; xiter < tiled_width; xiter++) {
			if (xiter+x >= 0 && xiter+x < 128 && yiter+y >= 0 && yiter+y < 64) {
				px_offset = ((yiter % tx->h) * tx->w) + xiter % tx->w;
				px = (tx->tx_data[px_offset>>2] & (3<<(2*(3-px_offset&3)))) >> (2*(3-(px_offset&3)));
				if (!(px & 2)) {
					Bdisp_SetPoint_VRAM(xiter+x, yiter+y, px & 1);
				}
			}
		}
	}
	return S_SUCCESS;
}