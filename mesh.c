#include "mesh.h"

/*
 MESH.C:
  functions for handling meshes and colliders
*/

mesh imesh(trianglef *arr, texture_ptr_t *tx_arr, uint8_t arrlen, vec3f pos, vec3f ctr) {
	mesh m;
	m.mesh_arr = arr;
	m.tx_arr = tx_arr;
	m.tr_cnt = arrlen;
	m.coll_arr = NULL;
	m.coll_cnt = 0;
	m.pos = pos;
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
	m.tr_cnt = 2; // billboards must have precisely two triangles
	m.coll_arr = NULL;
	m.coll_cnt = 0;
	m.pos = pos;
	// center point is in the bottom middle of the mesh (find it by averaging the coords)
	m.ctr = ivec3f((min(arr[0].a.x, min(arr[0].b.x, arr[0].c.x)) + max(arr[0].a.x, max(arr[0].b.x, arr[0].c.x)))/2, 0, (min(arr[0].a.z, min(arr[0].b.z, arr[0].c.z)) + max(arr[0].a.z, max(arr[0].b.z, arr[0].c.z)))/2);
	m.yaw = 0;
	m.flag_renderable = TRUE;
	m.flag_has_collision = FALSE;
	m.flag_is_billboard = TRUE;
	return m;
}

int m_collide(mesh *a, mesh *b) {
	collider tr_a, tr_b;
	int i, j;
	if (a == NULL || b == NULL)
		return S_ENULLPTR;

	if (!a->flag_has_collision || !b->flag_has_collision)
		return FALSE;

	if (a->coll_cnt == 0 && b->coll_cnt == 0)
		return FALSE;

	// check every collider in their arrays, offset by their coordinates
	for (i = 0; i < a->coll_cnt; i++) {
		tr_a = c_move_collider(a->coll_arr[i], subvv(a->pos, a->ctr));
		for (j = 0; j < b->coll_cnt; j++) {
			tr_b = c_move_collider(b->coll_arr[i], subvv(b->pos, b->ctr));
			if (c_do_colliders_collide(tr_a, tr_b))
				return TRUE;
		}
	}
	return FALSE;
}

// --- collision code ---

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