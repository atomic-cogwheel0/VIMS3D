#ifndef _MESH_H
#define _MESH_H

#include <stdlib.h>

#include "raster.h"

// collider types
#define COLLIDER_NONE   0
#define COLLIDER_SPHERE 1
#define COLLIDER_AABB   2 //Axis Aligned Bounding Box

// a single collider's geometry
typedef struct {
	uint8_t type; 
	union {
		struct {
			vec3f ctr;    // center
			fixed radius; // radius
		} sphere;
		struct {
			vec3f pt;  // any corner of the AABB
			vec3f opp; // the opposite corner
		} aabb;
	} shape; // a single collider can't be both at the same time
} collider;

// initialize spherical collider 
collider icoll_sphere(vec3f ctr, fixed r);
// initialize axis aligned collider, from two opposite corners
collider icoll_aabb(vec3f pt, vec3f opp);
// do two colliders collide?
bool c_do_colliders_collide(collider a, collider b);
// is point within collider?
bool c_pt_within_collider(vec3f pt, collider c);
// translate collider with vec v
collider c_move_collider(collider c, vec3f v);

// a single instance of a mesh, stores position, and POINTERS to hitboxes, triangles and colliders -> reusable arrays
typedef struct {
	trianglef *mesh_arr;
	texture_ptr_t *tx_arr;
	uint8_t tr_cnt; // if not billboard, tx_arr and mesh_arr both must have tr_cnt elements

	collider *coll_arr; // array of colliders relative to pos
	uint8_t coll_cnt;  // arr size

	vec3f pos; // position (offset of triangles and hitbox)
	fixed yaw; // around y axis (horizontal rot)
	vec3f ctr; // center of mesh (relative to pos)

	uuid_t id;

	bool flag_renderable : 1;
	bool flag_has_collision : 1;
	bool flag_is_billboard : 1; // if true: tx_arr is assumed to have 1 element, mesh_arr is assumed to have 2
} mesh;

// init new mesh
// tx_arr should be an array of pointers to the textures defined in "texturemap.c" (or anywhere else, really)
mesh imesh(trianglef *arr, texture_ptr_t *tx_arr, uint8_t arrlen, vec3f pos, vec3f ctr);
// init new billboard mesh (always faces player, single face = 2 tris)
// tx_pseudo_arr should be a pointer to a pointer to a texture
mesh ibill(trianglef *arr, texture_ptr_t *tx_pseudo_arr, vec3f pos);

// NEEDS CALLING (allocate bufs)
int m_init(void);
// clear buffers, reset indexes
int m_clrbuf(void);
// deallocate buffers (MAKES THE MESH SYSTEM UNUSABLE)
void m_dealloc(void);
// get status
int m_getstatus(void);

// add a mesh (returns id)
// the mesh will be rendered in the next pass
uuid_t m_addmesh(mesh *m);
// remove a mesh by id
int m_removemesh(uuid_t id);

// manipulate a mesh's pos and yaw (does not touch mesh_arr)
// IMPORTANT: these are relative, not absolute offsets
int m_movemesh(uuid_t id, vec3f offset);
int m_rotmesh(uuid_t id, fixed yaw);

int m_collide(uuid_t id1, uuid_t id2);

// run rendering pass with current meshes (returns draw time in ticks)
int m_rendermeshes(bool debug_overlay, camera *cam);

#endif