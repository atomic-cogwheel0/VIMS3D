#include "vec.h"

/*
  VEC.C
	functions for handling vectors (and triangles)
	they are most likely correct
*/

vec3f ivec3f(fixed x, fixed y, fixed z) {
	vec3f r;
	r.x = x; r.y = y; r.z = z;
	return r;
}
vec3f ivec3i(signed int x, signed int y, signed int z) {
	vec3f r;
	r.x = int2f(x); r.y = int2f(y); r.z = int2f(z);
	return r;
}
vec3f ivec3float(float x, float y, float z) {
	vec3f r;
	r.x = float2f(x); r.y = float2f(y); r.z = float2f(z);
	return r;
}

fixed magnitude(vec3f t) {
	return sqrt_f(dotp(t, t));
}

vec3f neg(vec3f t) {
	return ivec3f(-t.x, -t.y, -t.z);
}

vec3f normalize(vec3f t) {
	fixed m = magnitude(t);
	return ivec3f(divff(t.x, m), divff(t.y, m), divff(t.z, m));
}

#ifndef MACRO_DOTP
fixed dotp(vec3f t, vec3f v) {
	return mulff(t.x, v.x) + mulff(t.y, v.y) + mulff(t.z, v.z);
}
fixed dotp2(vec3f t, vec3f v) {
	return mulff(t.x, v.x) + mulff(t.y, v.y);
}
#endif

vec3f crossp(vec3f t, vec3f v) {
	return ivec3f(mulff(t.y, v.z)-mulff(t.z, v.y), mulff(t.z, v.x)-mulff(t.x, v.z), mulff(t.x, v.y)-mulff(t.y, v.x));
}

vec3f addvv(vec3f t, vec3f v) {
	return ivec3f(t.x+v.x, t.y+v.y, t.z+v.z);
}

vec3f subvv(vec3f t, vec3f v) {
	return ivec3f(t.x-v.x, t.y-v.y, t.z-v.z);
}

vec3f mulvf(vec3f t, fixed f) {
	return ivec3f(mulff(t.x, f), mulff(t.y, f), mulff(t.z, f));
}

vec3f mulvi(vec3f t, int f) {
	return ivec3f(mulfi(t.x, f), mulfi(t.y, f), mulfi(t.z, f));
}

vec3f divvf(vec3f t, fixed f) {
	return ivec3f(divff(t.x, f), divff(t.y, f), divff(t.z, f));
}

vec3f divvi(vec3f t, int f) {
	return ivec3f(divfi(t.x, f), divfi(t.y, f), divfi(t.z, f));
}

/*  |cos ?   -sin ?   0| |x|   |x cos ? - y sin ?|   |x'|
    |sin ?    cos ?   0| |y| = |x sin ? + y cos ?| = |y'|
    |  0       0      1| |z|   |        z        |   |z'|


    | cos ?    0   sin ?| |x|   | x cos ? + z sin ?|   |x'|
    |   0      1       0| |y| = |         y        | = |y'|
    |-sin ?    0   cos ?| |z|   |-x sin ? + z cos ?|   |z'|


    |1     0           0| |x|   |        x        |   |x'|
    |0   cos ?    -sin ?| |y| = |y cos ? - z sin ?| = |y'|
    |0   sin ?     cos ?| |z|   |y sin ? + z cos ?|   |z'| */

vec3f py2vec3f(fixed pitch, fixed yaw) {
	vec3f v = ivec3f(0, 0, int2f(1));
	
	return rot(v, pitch, yaw);
}

vec3f rot(vec3f t, fixed pitch, fixed yaw) {
	vec3f resp, resy;	

	fixed sy, cy, sp, cp;

	sy = sin_f(yaw); sp = sin_f(pitch); cy = cos_f(yaw); cp = cos_f(pitch);

	resy.x = mulff(t.x, cy) + mulff(t.z, sy);
	resy.y = t.y;
	resy.z = mulff(t.z, cy) - mulff(t.x, sy);

	resp.x = resy.x;
	resp.y = mulff(resy.y, cp) - mulff(resy.z, sp);
	resp.z = mulff(resy.y, sp) + mulff(resy.z, cp);

	return resp;
}

vec3f horiz(vec3f v) {
	return ivec3f(v.x, 0, v.z);
}

trianglef itrianglef(vec3f _a, vec3f _b, vec3f _c, bool _flip) {
	trianglef r;
	r.a = _a;
	r.b = _b;
	r.c = _c;
	r.flip_texture = _flip;
	return r;
}

vec3f transform_vec_from_zero(vec3f u, vec3f v, fixed pitch, fixed yaw) {
	return addvv(rot(u, pitch, yaw), v);
}

vec3f transform_vec_to_camera(vec3f u, camera cam) {
	return rot(subvv(u, cam.pos), -cam.pitch, -cam.yaw);
}

trianglef transform_tri_from_zero(trianglef q, vec3f v, fixed pitch, fixed yaw) {
	return itrianglef(addvv(rot(q.a, pitch, yaw), v), addvv(rot(q.b, pitch, yaw), v), addvv(rot(q.c, pitch, yaw), v), q.flip_texture);
}

trianglef transform_tri_to_pos(trianglef q, position pos) {
	return transform_tri_from_zero(q, pos.pos, pos.pitch, pos.yaw);
}

trianglef transform_tri_to_camera(trianglef q, camera cam) {
	return itrianglef(rot(subvv(q.a, cam.pos), -cam.pitch, -cam.yaw), rot(subvv(q.b, cam.pos), -cam.pitch, -cam.yaw), rot(subvv(q.c, cam.pos), -cam.pitch, -cam.yaw), q.flip_texture);
}

vec3f normal(trianglef q) {
	return crossp(subvv(q.b, q.a), subvv(q.c, q.a));
}