#ifndef _MESHRENDER_H
#define _MESHRENDER_H

#include <stdlib.h>

#include "raster.h"

typedef signed short mesh_id_t;

typedef struct {
	trianglef * mesh_arr;
	texture_ptr_t *tx_arr;
	uint8_t arrlen;

	vec3f pos;
	fixed yaw; // around y axis
	vec3f ctr; // center of mesh (relative to pos)

	bool is_billboard; // if true: tx_arr is assumed to have 1 element, mesh_arr is assumed to have 2
	mesh_id_t id;
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

// set global variables (keeping them separated from the raster engine might prove useful)
void m_coord(vec3f pos, fixed pitch, fixed yaw);

// add a mesh (returns id)
// the mesh will be rendered in the next pass
mesh_id_t m_addmesh(mesh m);
// remove a mesh by id
int m_removemesh(mesh_id_t id);

// manipulate a mesh's pos and yaw (does not touch mesh_arr)
// IMPORTANT: these are relative, not absolute offsets
int m_movemesh(mesh_id_t id, vec3f offset);
int m_rotmesh(mesh_id_t id, fixed yaw);

// run rendering pass with current meshes (returns draw time in ticks)
int m_rendermeshes(bool debug_overlay, bool interlace);

#endif