#include "mesh.h"

/*
 MESH.C:
  functions for handling meshes and colliders
  supported:
	mesh addition, deletion, rotation, translation
	collision boxes & spheres
  the render process starts here
    fills triangle buffers and calls g_rasterize_buf for each mesh
    this adds some overhead but drastically reduces memory usage
*/

// max number of meshes
#define MBUF_SIZ 12

mesh *mbuf;
unsigned short mbuf_idx = 0;
signed short zindex_max = 0;
uuid_t m_uuid = 1;				// nothing gets uuid 0

// global coordination variables
fixed p, y; // pitch, yaw
vec3f o; // origin point (camera)

int m_status = G_SUBSYS_DOWN;

// simplifies internal calculation
bool _c_collide_sphere_aabb(collider sphere, collider aabb);

collider icoll_sphere(vec3f ctr, fixed r) {
	collider coll;
	coll.type = COLLIDER_SPHERE;
	coll.shape.sphere.ctr = ctr;
	coll.shape.sphere.radius = r;
	return coll;
}

collider icoll_aabb(vec3f pt, vec3f opp) {
	collider coll;
	coll.type = COLLIDER_AABB;
	coll.shape.aabb.pt = pt;
	coll.shape.aabb.opp = opp;
	return coll;
}

bool c_do_colliders_collide(collider a, collider b) {
	if (a.type == COLLIDER_SPHERE && b.type == COLLIDER_SPHERE) {
		return (a.shape.sphere.radius + b.shape.sphere.radius >= magnitude(subvv(b.shape.sphere.ctr, a.shape.sphere.ctr)));
	}
	if (a.type == COLLIDER_AABB && b.type == COLLIDER_AABB) {
		if ((max(a.shape.aabb.pt.x, a.shape.aabb.opp.x) >= min(b.shape.aabb.pt.x, b.shape.aabb.opp.x)) && (min(a.shape.aabb.pt.x, a.shape.aabb.opp.x) <= max(b.shape.aabb.pt.x, b.shape.aabb.opp.x)) && 
		    (max(a.shape.aabb.pt.y, a.shape.aabb.opp.y) >= min(b.shape.aabb.pt.y, b.shape.aabb.opp.y)) && (min(a.shape.aabb.pt.y, a.shape.aabb.opp.y) <= max(b.shape.aabb.pt.y, b.shape.aabb.opp.y)) &&
		    (max(a.shape.aabb.pt.z, a.shape.aabb.opp.z) >= min(b.shape.aabb.pt.z, b.shape.aabb.opp.z)) && (min(a.shape.aabb.pt.z, a.shape.aabb.opp.z) <= max(b.shape.aabb.pt.z, b.shape.aabb.opp.z))) return TRUE;
	}
	if (a.type == COLLIDER_SPHERE && b.type == COLLIDER_AABB) {
		return _c_collide_sphere_aabb(a, b);
	}
	if (a.type == COLLIDER_AABB && b.type == COLLIDER_SPHERE) {
		return _c_collide_sphere_aabb(b, a);
	}
	return FALSE;
}

bool c_pt_within_collider(vec3f pt, collider c) {
	if (c.type == COLLIDER_SPHERE) {
		return (c.shape.sphere.radius >= magnitude(subvv(c.shape.sphere.ctr, pt)));
	}
	if (c.type == COLLIDER_AABB) {
		if ((min(c.shape.aabb.pt.x, c.shape.aabb.opp.x) <= pt.x && max(c.shape.aabb.pt.x, c.shape.aabb.opp.x) >= pt.x) &&
			(min(c.shape.aabb.pt.y, c.shape.aabb.opp.y) <= pt.y && max(c.shape.aabb.pt.x, c.shape.aabb.opp.y) >= pt.y) &&
			(min(c.shape.aabb.pt.z, c.shape.aabb.opp.z) <= pt.z && max(c.shape.aabb.pt.z, c.shape.aabb.opp.z) >= pt.z)) return TRUE;
	}
	return FALSE;
}

collider c_move_collider(collider c, vec3f v) {
	if (c.type == COLLIDER_SPHERE) {
		c.shape.sphere.ctr = addvv(c.shape.sphere.ctr, v);
	}
	if (c.type == COLLIDER_AABB) {
		c.shape.aabb.pt = addvv(c.shape.aabb.pt, v);
		c.shape.aabb.opp = addvv(c.shape.aabb.opp, v);
	}
	return c;
}

bool _c_collide_sphere_aabb(collider sphere, collider aabb) {
	vec3f closest_pt;

	// is the sphere inside the AABB?
	if (c_pt_within_collider(sphere.shape.sphere.ctr, aabb)) return TRUE;
	
	// find closest point of the AABB to the sphere by clamping coords of the center onto the surface of the AABB
	closest_pt.x = clamp_f(sphere.shape.sphere.ctr.x, min(aabb.shape.aabb.pt.x, aabb.shape.aabb.opp.x), max(aabb.shape.aabb.pt.x, aabb.shape.aabb.opp.x));
	closest_pt.y = clamp_f(sphere.shape.sphere.ctr.y, min(aabb.shape.aabb.pt.y, aabb.shape.aabb.opp.y), max(aabb.shape.aabb.pt.y, aabb.shape.aabb.opp.y));
	closest_pt.z = clamp_f(sphere.shape.sphere.ctr.z, min(aabb.shape.aabb.pt.z, aabb.shape.aabb.opp.z), max(aabb.shape.aabb.pt.z, aabb.shape.aabb.opp.z));

	// is closest point within radius?
	return c_pt_within_collider(closest_pt, sphere);
}

mesh imesh(trianglef *arr, texture_ptr_t *tx_arr, uint8_t arrlen, vec3f pos, vec3f ctr) {
	mesh m;
	m.mesh_arr = arr;
	m.tx_arr = tx_arr;
	m.tr_cnt = arrlen;
	m.coll_arr = NULL;
	m.coll_cnt = 0;
	m.pos = pos;
	m.id = 0;
	m.ctr = ctr;
	m.yaw = 0;
	m.flag_renderable = TRUE;
	m.flag_has_collision = FALSE;
	m.flag_is_billboard = FALSE;
	return m;
}

mesh ibill(trianglef *arr, texture_ptr_t *tx_pseudo_arr, vec3f pos) {
	mesh m;
	m.mesh_arr = arr;
	m.tx_arr = tx_pseudo_arr;
	m.tr_cnt = 2;
	m.coll_arr = NULL;
	m.coll_cnt = 0;
	m.pos = pos;
	m.id = 0;
	m.ctr = ivec3f(0, divfi(arr[0].a.x + arr[0].b.x, 2), 0);
	m.yaw = 0;
	m.flag_renderable = TRUE;
	m.flag_has_collision = FALSE;
	m.flag_is_billboard = TRUE;
	return m;
}

// initialize all buffers (static alloc for buffers is not possible)
int m_init(void) {
	if (m_status != G_SUBSYS_DOWN) return G_EALREADYINITED;

	mbuf = (mesh *)malloc(MBUF_SIZ*sizeof(mesh));

	if (mbuf == NULL) {
		m_status = G_SUBSYS_ERR;
		return G_EALLOC;
	}
	m_status = G_SUBSYS_UP;

	m_clrbuf();

	return G_SUCCESS;
}

int m_clrbuf(void) {
	if (m_status != G_SUBSYS_UP) return G_EDOWN;
	memset((char *)mbuf, 0, MBUF_SIZ*sizeof(mesh));
	mbuf_idx = 0;
	zindex_max = 0;
	m_uuid = 1;
}

void m_dealloc(void) {
	if (mbuf != NULL) {
		free(mbuf);
		mbuf = NULL;
	}
	m_status = G_SUBSYS_DOWN;
}

int m_getstatus(void) {
	return m_status;
}

void m_coord(vec3f pos, fixed pitch, fixed yaw) {
	o = pos;
	p = pitch;
	y = yaw;
}

uuid_t m_addmesh(mesh m) {
	if (m_status != G_SUBSYS_UP) return G_EDOWN;
	if (m_uuid < UUID_MAX) {
		if (mbuf_idx < MBUF_SIZ-1) {
			mbuf[mbuf_idx] = m;
			mbuf[mbuf_idx++].id = m_uuid;
			return m_uuid++;
		}
	}
	return G_EBUFFULL;
}

int m_removemesh(uuid_t id) {
	int i, j;
	if (m_status != G_SUBSYS_UP) return G_EDOWN;
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

int m_movemesh(uuid_t id, vec3f v) {
	int i, j;
	if (m_status != G_SUBSYS_UP) return G_EDOWN;
	for(i = 0; i < mbuf_idx; i++) {
		if (mbuf[i].id == id) {
			mbuf[i].pos = addvv(mbuf[i].pos, v);
			return G_SUCCESS;
		}
	}
	return G_ENEXIST;
}

int m_rotmesh(uuid_t id, fixed yaw) {
	int i, j;
	if (m_status != G_SUBSYS_UP) return G_EDOWN;
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

int m_collide(uuid_t id1, uuid_t id2) {
	mesh *a = NULL, *b = NULL;
	collider tr_a, tr_b;
	int i, j;
	if (m_status != G_SUBSYS_UP) return G_EDOWN;
	for (i = 0; i < mbuf_idx; i++) {
		if (mbuf[i].id == id1) {
			a = &mbuf[i];
			break;
		}
	}
	for (i = 0; i < mbuf_idx; i++) {
		if (mbuf[i].id == id2) {
			b = &mbuf[i];
			break;
		}
	}
	if (a == NULL && b == NULL)
		return G_ENEXIST;

	if (!a->flag_has_collision || !b->flag_has_collision)
		return FALSE;

	if (a->coll_cnt == 0 && b->coll_cnt == 0)
		return FALSE;

	for (i = 0; i < a->coll_cnt; i++) {
		tr_a = c_move_collider(a->coll_arr[i], a->pos);
		for (j = 0; j < b->coll_cnt; j++) {
			tr_b = c_move_collider(b->coll_arr[i], b->pos);
			if (c_do_colliders_collide(tr_a, tr_b))
				return TRUE;
		}
	}
	return FALSE;
}

char buf[64]; // for sprintf
unsigned int frame;

int m_rendermeshes(bool debug_overlay, bool interlace) {
	int m_iter, t_iter, dx, dy, i;
	unsigned int time_s, time_e2, deltaticks, tr_cnt = 0;
	trianglef curr;
	int16_t **depthbuf; // the pixel depth buffer, reset before meshes are rendered
	
	if (m_status != G_SUBSYS_UP) return G_EDOWN;
	if (g_getstatus() != G_SUBSYS_UP) return G_EDOWN;

	depthbuf = g_getdepthbuf();

	time_s = RTC_GetTicks();

	Bdisp_AllClr_VRAM();

	for (dx = 0; dx < 128; dx++) {
		for (dy = 0; dy < 64; dy++) {
			depthbuf[dy][dx] = 0x7FFF;
		}
	}
	
	g_draw_horizon();

	for (m_iter = 0; m_iter < mbuf_idx; m_iter++) {
		if (mbuf[m_iter].flag_is_billboard) {
			mbuf[m_iter].yaw = -y;
		}
		if (!mbuf[m_iter].flag_renderable)
			continue;
		g_clrbuf();
		for (t_iter = 0; t_iter < mbuf[m_iter].tr_cnt; t_iter++) {
			// TODO: separate this into physics ticks
			curr = move(mbuf[m_iter].mesh_arr[t_iter], mbuf[m_iter].ctr, 0, mbuf[m_iter].yaw);
			curr = move(curr, subvv(ivec3f(0,0,0), mbuf[m_iter].ctr), 0, 0);
			curr = move(curr, mbuf[m_iter].pos, 0, 0);
			curr.tx = mbuf[m_iter].tx_arr[mbuf[m_iter].flag_is_billboard?0:t_iter];
			if (mbuf[m_iter].flag_is_billboard) {
				curr.flip_texture = t_iter == 0 ? FALSE : TRUE;
			}
			g_addtriangle(curr);
		}

#ifndef BENCHMARK_RASTER
		tr_cnt += g_rasterize_buf(interlace, frame % 2);
#else
		for (i = 0; i < 40; i++) {
			tr_cnt += g_rasterize_buf(interlace, frame % 2);
		}
#endif
		
#ifdef DEBUG_BUILD
		if (debug_overlay) {
			sprintf((char *)buf, "%d", m_iter);
			g_text3d(buf, subvv(ivec3f(0,int2f(16),0), mbuf[m_iter].pos), TEXT_SMALL | TEXT_INVERTED);
		}
#endif
	}
	
	time_e2 = RTC_GetTicks();

	frame++;

	deltaticks = time_e2-time_s;
	if (deltaticks < 1) deltaticks = 1; 

	//if (deltaticks > 128) deltaticks -= 128; // the emulator sometimes skips a whole second :)

#ifndef BENCHMARK_RASTER
	if (debug_overlay) {
#else
	if (TRUE) {
#endif
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

