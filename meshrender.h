#ifndef _MESHRENDER_H
#define _MESHRENDER_H

#include <stdlib.h>

#include "raster.h"

typedef signed short mesh_id_t;

typedef struct {
	trianglef *mesh_arr;
	texture_ptr_t *tx_arr;
	uint8_t arrlen;

	vec3f pos;
	fixed yaw; // around y axis
	vec3f ctr; // center of mesh (relative to pos)

	bool is_billboard; // if true: tx_arr is assumed to have 1 element, mesh_arr is assumed to have 2
	mesh_id_t id;
} mesh;

mesh imesh(trianglef *arr, texture_ptr_t *tx_arr, uint8_t arrlen, vec3f pos, vec3f ctr);
mesh ibill(trianglef *arr, texture_ptr_t *tx_pseudo_arr, vec3f pos);

mesh_id_t m_addmesh(mesh m);
int m_removemesh(mesh_id_t id);

int m_movemesh(mesh_id_t id, vec3f offset);
int m_rotmesh(mesh_id_t id, fixed yaw);

int m_coord(vec3f _pos, fixed _pitch, fixed _yaw);
int m_init(void);

int m_clrbuf(void);
int m_rendermeshes(int);

#endif