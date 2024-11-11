#ifndef VEC_H
#define VEC_H

#include <stdio.h>

#include "fixed.h"
#include "texture.h"

/*
VEC3 MATH LIBRARY

precision: fixed-point

TRIANGLEF:
 struct of 3 vec3f (coplanar vertices [later note: duh])
 flip_texture is for billboarding (and potentially other stuff)
*/

typedef struct {
	fixed x;
	fixed y;
	fixed z;
} vec3f;

// defines certain functions as macros or real functions
// rebuild all if changed
#define MACRO_DOTP

#pragma inline(ivec3f)
// initialize vec3f
vec3f ivec3f(fixed x, fixed y, fixed z);
// ease-of-use initers
// initialize from 3 ints
vec3f ivec3i(signed int x, signed int y, signed int z);
// initialize from 3 floats
vec3f ivec3float(float x, float y, float z);

// transform pitch and yaw into a rotated normal vector
vec3f py2vec3f(fixed pitch, fixed yaw);
// rotate vector t around axes x (pitch) and y (yaw)
vec3f rot(vec3f t, fixed pitch, fixed yaw);

fixed magnitude(vec3f t);
vec3f neg(vec3f t);
vec3f normalize(vec3f t);

#ifdef MACRO_DOTP
#define dotp(t, v) (mulff((t).x, (v).x) + mulff((t).y, (v).y) + mulff((t).z, (v).z))
#define dotp2(t, v) (mulff((t).x, (v).x) + mulff((t).y, (v).y))
#else
#pragma inline(dotp)
fixed dotp(vec3f t, vec3f v);
fixed dotp2(vec3f t, vec3f v);
#endif
vec3f crossp(vec3f t, vec3f v);

vec3f addvv(vec3f t, vec3f v);
vec3f subvv(vec3f t, vec3f v);
vec3f mulvf(vec3f t, fixed f);
vec3f mulvi(vec3f t, int f);
vec3f divvf(vec3f t, fixed f);
vec3f divvi(vec3f t, int f);

// remove vertical component
vec3f horiz(vec3f v);

typedef struct {
	vec3f pos;
	fixed yaw;
	fixed pitch;
} position;
typedef position camera;

typedef struct {
	vec3f a;
	vec3f b;
	vec3f c;
	bool flip_texture;
} trianglef;

// initialize trianglef
trianglef itrianglef(vec3f a, vec3f b, vec3f c, bool flip);

// add v to every point of q
trianglef move_tri_by_vec(trianglef q, vec3f v);

// rotate vector u around (0;0;0) with given angles and offset it with v
vec3f transform_vec_from_zero(vec3f u, vec3f v, fixed pitch, fixed yaw);
// transform vector from world space to camera relative space
vec3f transform_vec_to_camera(vec3f u, camera cam);

// rotate triangle q around (0;0;0) with given angles and offset it with v
trianglef transform_tri_from_zero(trianglef q, vec3f v, fixed pitch, fixed yaw);
// transform triangle from (0;0;0) to position with rotations performed first
trianglef transform_tri_to_pos(trianglef q, position pos);
// transform triangle from world space to camera relative space
trianglef transform_tri_to_camera(trianglef q, camera cam);

// calculate surface normal
vec3f normal(trianglef q);

#endif