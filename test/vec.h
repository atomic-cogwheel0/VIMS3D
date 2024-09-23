#ifndef _VEC_H
#define _VEC_H

#include <math.h>
#include <stdio.h>

#include "fixed.h"

#define DEG_UNIT (3.1415926535898f/180.0f)

#define C_WHITE 0
#define C_BLACK 1

typedef struct {
	fixed x;
	fixed y;
	fixed z;
} vec3f;

vec3f ivec3f(fixed _x, fixed _y, fixed _z);
vec3f py2vec3f(fixed pitch, fixed yaw);
vec3f rot(vec3f t, fixed pitch, fixed yaw);

fixed magnitude(vec3f t);
vec3f neg(vec3f t);
vec3f normalize(vec3f t);

fixed dotp(vec3f t, vec3f v);
vec3f crossp(vec3f t, vec3f v);

vec3f addvv(vec3f t, vec3f v);
vec3f subvv(vec3f t, vec3f v);
vec3f mulvf(vec3f t, fixed f);

void addpvv(vec3f *t, vec3f v);
void subpvv(vec3f *t, vec3f v);
void mulpvf(vec3f *t, fixed f);

typedef struct {
	vec3f a;
	vec3f b;
	vec3f c;
	unsigned char color;
	signed short zindex;
	uint32_t id;
} trianglef;

trianglef itrianglef(vec3f a, vec3f b, vec3f c, unsigned char color, uint32_t id);

trianglef move(trianglef t, vec3f v, fixed pitch, fixed yaw);

#endif
