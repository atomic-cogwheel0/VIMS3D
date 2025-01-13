#include "mesh.h"

/*
 MESH.C:
  functions for handling meshes and colliders
*/

mesh imesh(trianglef *arr, texture_t **tx_arr, uint8_t arrlen, vec3f pos, vec3f ctr) {
	mesh m;
	m.mesh_arr = arr;
	m.tx_arr = tx_arr;
	m.tr_cnt = arrlen;
	m.coll_arr = NULL;
	m.coll_cnt = 0;
	m.pos.pos = pos;
	m.pos.pitch = 0;
	m.pos.yaw = 0;
	m.ctr = ctr;
	m.is_renderable = TRUE;
	m.has_collision = FALSE;
	m.is_billboard = FALSE;
	return m;
}

int m_setcoll(mesh *m, collider *colls, uint8_t coll_cnt) {
	if (m == NULL) return S_ENULLPTR;
	m->coll_arr = colls;
	m->coll_cnt = coll_cnt;
	m->has_collision = (colls == NULL) ? FALSE : TRUE;
	return S_SUCCESS;
}

mesh ibill(trianglef *arr, texture_t **tx_arr, vec3f pos) {
	mesh m;
	m.mesh_arr = arr;
	m.tx_arr = tx_arr;
	m.tr_cnt = 2; // billboards must have precisely two triangles
	m.coll_arr = NULL;
	m.coll_cnt = 0;
	m.pos.pos = pos;
	m.pos.pitch = 0;
	m.pos.yaw = 0;
	// (0;0;0) is the center -> please make sure it's symmetric
	m.ctr = ivec3f(0, 0, 0);
	m.is_renderable = TRUE;
	m.has_collision = FALSE;
	m.is_billboard = TRUE;
	return m;
}

mesh inullmesh(void) {
	mesh m;
	m.mesh_arr = NULL;
	m.tx_arr = NULL;
	m.tr_cnt = 0;
	m.coll_arr = NULL;
	m.coll_cnt = 0;
	m.pos.pos = ivec3f(0, 0, 0);
	m.pos.pitch = 0;
	m.pos.yaw = 0;
	m.ctr = ivec3f(0, 0, 0);
	m.is_renderable = FALSE;
	m.has_collision = FALSE;
	m.is_billboard = FALSE;
	return m;
}

int m_collide(mesh *a, mesh *b) {
	collider tr_a, tr_b;
	int i, j;
	if (a == NULL || b == NULL)
		return S_ENULLPTR;

	if (!a->has_collision || !b->has_collision)
		return FALSE;

	if (a->coll_cnt == 0 && b->coll_cnt == 0)
		return FALSE;

	// check every collider in their arrays, offset by their coordinates
	for (i = 0; i < a->coll_cnt; i++) {
		tr_a = c_move_collider(a->coll_arr[i], a->pos.pos);
		for (j = 0; j < b->coll_cnt; j++) {
			tr_b = c_move_collider(b->coll_arr[j], b->pos.pos);
			if (c_do_colliders_collide(tr_a, tr_b)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

// --- collision code ---

// simplifies internal calculation
bool inner_c_collide_sphere_aabb(collider sphere, collider aabb);

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
		return inner_c_collide_sphere_aabb(a, b);
	}
	if (a.type == COLLIDER_AABB && b.type == COLLIDER_SPHERE) {
		return inner_c_collide_sphere_aabb(b, a);
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

bool inner_c_collide_sphere_aabb(collider sphere, collider aabb) {
	vec3f closest_pt;

	// is the sphere inside the AABB?
	if (c_pt_within_collider(sphere.shape.sphere.ctr, aabb)) return TRUE;

	// find closest point of the AABB to the sphere by clamping the coords of the center onto the surface of the AABB
	closest_pt.x = clamp_f(sphere.shape.sphere.ctr.x, min(aabb.shape.aabb.pt.x, aabb.shape.aabb.opp.x), max(aabb.shape.aabb.pt.x, aabb.shape.aabb.opp.x));
	closest_pt.y = clamp_f(sphere.shape.sphere.ctr.y, min(aabb.shape.aabb.pt.y, aabb.shape.aabb.opp.y), max(aabb.shape.aabb.pt.y, aabb.shape.aabb.opp.y));
	closest_pt.z = clamp_f(sphere.shape.sphere.ctr.z, min(aabb.shape.aabb.pt.z, aabb.shape.aabb.opp.z), max(aabb.shape.aabb.pt.z, aabb.shape.aabb.opp.z));

	// is closest point within radius?
	return c_pt_within_collider(closest_pt, sphere);
}