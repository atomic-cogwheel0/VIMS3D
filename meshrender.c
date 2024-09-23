#include "meshrender.h"

/*
 MESHRENDER.C:
  functions for handling meshes
  supported:
	mesh addition, deletion, rotation, translation
  the render process starts here
    fills triangle buffers and calls g_rasterize_buf for each mesh
    this adds some overhead but drastically reduces memory usage
*/

// max number of meshes
#define MBUF_SIZ 12

mesh *mbuf;
unsigned short mbuf_idx = 0;
signed short zindex_max = 0;
uint32_t m_uuid = 1;				// nothing gets uuid 0

// shouldn't run two draws at the same time
volatile int lock = 0;

// global coordination variables
fixed p, y; // pitch, yaw
vec3f o; // origin point (camera)

// the pixel depth buffer, reset before meshes are rendered
extern int16_t *depthbuf[64];

// initialize all buffers (static alloc for buffers is not possible)
int m_init(void) {
	mbuf = (mesh *)malloc(MBUF_SIZ*sizeof(mesh));

	if (mbuf == NULL) return G_EALLOC;	

	m_clrbuf();

	return G_SUCCESS;
}

// clear buffers, initialize 
int m_clrbuf(void) {
	memset((char *)mbuf, 0, MBUF_SIZ*sizeof(mesh));
	mbuf_idx = 0;
	zindex_max = 0;
	m_uuid = 1;
}

mesh_id_t m_addmesh(mesh m) {
	if (mbuf_idx < MBUF_SIZ-1) {
		mbuf[mbuf_idx] = m;
		mbuf[mbuf_idx++].id = m_uuid;
		return m_uuid++;
	}
	return G_EBUFFULL;
}

int m_removemesh(mesh_id_t id) {
	int i, j;
	for(i = 0; i < mbuf_idx; i++) {
		if (mbuf[i].id == id) {
			for (j = i; j < (mbuf_idx-1); j++) {
				mbuf[j] = mbuf[j+1];
			}
			mbuf_idx--;
			return G_SUCCESS;
		}
	}
	return G_ENEXIST;
}

int m_coord(vec3f _pos, fixed _pitch, fixed _yaw) {
	o = _pos;
	p = _pitch;
	y = _yaw;
}	

mesh imesh(trianglef *arr, texture_ptr_t *tx_arr, uint8_t arrlen, vec3f pos, vec3f ctr) {
	mesh m;
	m.mesh_arr = arr;
	m.tx_arr = tx_arr;
	m.arrlen = arrlen;
	m.pos = pos;
	m.id = 0;
	m.ctr = ctr;
	m.yaw = 0;
	m.is_billboard = FALSE;
	return m;
}

/*
 tx_pseudo_arr should be a ptr a ptr to a texture defined in "texturemap.h"
*/

mesh ibill(trianglef *arr, texture_ptr_t *tx_pseudo_arr, vec3f pos) {
	mesh m;
	m.mesh_arr = arr;
	m.tx_arr = tx_pseudo_arr;
	m.arrlen = 2;
	m.pos = pos;
	m.id = 0;
	m.ctr = ivec3f(0, 0, 0);
	m.yaw = 0;
	m.is_billboard = TRUE;
	return m;
}

int m_movemesh(mesh_id_t id, vec3f v) {
	int i, j;
	for(i = 0; i < mbuf_idx; i++) {
		if (mbuf[i].id == id) {
			mbuf[i].pos = addvv(mbuf[i].pos, v);
			return G_SUCCESS;
		}
	}
	return G_ENEXIST;
}

int m_rotmesh(mesh_id_t id, fixed yaw) {
	int i, j;
	for(i = 0; i < mbuf_idx; i++) {
		if (mbuf[i].id == id) {
			mbuf[i].yaw += yaw;
			if (mbuf[i].yaw > float2f(180*DEG2RAD_MULT)) {
				mbuf[i].yaw -= float2f(360*DEG2RAD_MULT);
			}
			if (mbuf[i].yaw < float2f(-180*DEG2RAD_MULT)) {
				mbuf[i].yaw += float2f(360*DEG2RAD_MULT);
			}
			return G_SUCCESS;
		}
	}
	return G_ENEXIST;
}

char buf[64]; // for sprintf

int m_rendermeshes(int debug_overlay) {
	int m_iter, t_iter, dx, dy;
	unsigned int time_s, time_e2, deltaticks, tr_cnt = 0;
	trianglef curr;	
	
	time_s = RTC_GetTicks();

	Bdisp_AllClr_VRAM();

	for (dx = 0; dx < 128; dx++) {
		for (dy = 0; dy < 64; dy++) {
			depthbuf[dy][dx] = 0x7FFF;
		}
	}
	
	g_draw_horizon();

	for (m_iter = 0; m_iter < mbuf_idx; m_iter++) {
		g_clrbuf();		

		for (t_iter = 0; t_iter < mbuf[m_iter].arrlen; t_iter++) {
			if (mbuf[m_iter].is_billboard) {
				mbuf[m_iter].yaw = -y;
			}
			curr = move(mbuf[m_iter].mesh_arr[t_iter], mbuf[m_iter].ctr, 0, mbuf[m_iter].yaw);
			curr = move(curr, subvv(ivec3f(0,0,0), mbuf[m_iter].ctr), 0, 0);
			curr = move(curr, mbuf[m_iter].pos, 0, 0);
			curr.tx = mbuf[m_iter].tx_arr[mbuf[m_iter].is_billboard?0:t_iter];
			if (mbuf[m_iter].is_billboard) {
				curr.flip_texture = t_iter == 0 ? FALSE : TRUE;
			}
			g_addtriangle(curr);
		}

		tr_cnt += g_rasterize_buf();
		
		/* if (debug_overlay) {
			sprintf((char *)buf, "%d", m_iter);
			g_text3d(buf, subvv(ivec3f(0,int2f(16),0), mbuf[m_iter].pos), TEXT_SMALL | TEXT_INVERTED);
		} */
	}
	
	time_e2 = RTC_GetTicks();

	deltaticks = time_e2-time_s;
	if (deltaticks < 1) deltaticks = 1; 

	//if (deltaticks > 128) deltaticks -= 128; // the emulator sometimes skips a whole second :)

	if (debug_overlay) {
		sprintf(buf, "%4.1fms (%2.1ffps) %dt/%dm", (deltaticks)*(1000.0/128.0), 1000.0/((deltaticks)*(1000.0/128.0)), tr_cnt, m_iter);
		PrintMini(0, 0, (unsigned char *)buf, 0);
	
		sprintf(buf, "%4.1f %4.1f %4.1f %4.1fp %4.1fy",	f2float(o.x),
														f2float(o.y),
														f2float(o.z), f2float(p)*RAD2DEG_MULT, f2float(y)*RAD2DEG_MULT);
		PrintMini(0, 6, (unsigned char *)buf, 0);
	}

#ifdef CROSSHAIR
	Bdisp_AreaReverseVRAM(61, 32, 67, 32);
	Bdisp_AreaReverseVRAM(64, 29, 64, 35);
	Bdisp_AreaReverseVRAM(64, 32, 64, 32); //crosshair
#endif

	return deltaticks;
}


