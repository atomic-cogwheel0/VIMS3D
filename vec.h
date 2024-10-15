#ifndef _VEC_H
#define _VEC_H

#include <math.h>
#include <stdio.h>
#include "fxlib.h"

#include "fixed.h"
#include "texture.h"

/*
VEC3 MATH LIBRARY

precision: fixed-point

TRIANGLEF:
 struct of 3 vec3f (coplanar vertices [later note: duh])
 id is considered "private"
 tx is the texture (see texturemap.c and graphic.h)
 flip_texture is for billboarding
*/

typedef struct {
	fixed x;
	fixed y;
	fixed z;
} vec3f;

#define MACRO_DOTP
#define MACRO_ADDSUBPV

#pragma inline(ivec3f)
// initialize vec3f
vec3f ivec3f(fixed _x, fixed _y, fixed _z);

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

// for some reason inlining subvv makes rendering fail
//#pragma inline(subvv)

vec3f addvv(vec3f t, vec3f v);
vec3f subvv(vec3f t, vec3f v);
vec3f mulvf(vec3f t, fixed f);
vec3f mulvi(vec3f t, int f);
vec3f divvf(vec3f t, fixed f);
vec3f divvi(vec3f t, int f);

#ifdef MACRO_ADDSUBPV
#define addpvv(t, v) (t)->x+=(v).x,(t)->y+=(v).y,(t)->z+=(v).z
#define subpvv(t, v) (t)->x-=(v).x,(t)->y-=(v).y,(t)->z-=(v).z
#else
void addpvv(vec3f *t, vec3f v);
void subpvv(vec3f *t, vec3f v);
#endif

void mulpvf(vec3f *t, fixed f);

typedef struct {
	vec3f pos;
	fixed yaw;
	fixed pitch;
} camera;

typedef struct {
	vec3f a;
	vec3f b;
	vec3f c;
	texture_t *tx;
	uuid_t id;
	bool flip_texture;
} trianglef;

// initialize trianglef
trianglef itrianglef(vec3f a, vec3f b, vec3f c, texture_t *tx, uuid_t id, bool flip);

trianglef transform_tri_from_zero(trianglef q, vec3f v, fixed pitch, fixed yaw);
trianglef transform_tri_to_camera(trianglef q, camera cam);

vec3f normal(trianglef q);

#endif