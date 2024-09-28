#include "vec.h"

/*
  VEC.C
	functions for handling vectors (and triangles)
	they are most likely correct
*/

//initialize vec3f

vec3f ivec3f(fixed _x, fixed _y, fixed _z) {
    vec3f r;
    r.x = _x; r.y = _y; r.z = _z;
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

#ifndef MACRO_ADDSUBPV
void addpvv(vec3f *t, vec3f v) {
	(t->x)+=v.x;
    (t->y)+=v.y;
    (t->z)+=v.z;
}

void subpvv(vec3f *t, vec3f v) {
	(t->x)-=v.x;
    (t->y)-=v.y;
    (t->z)-=v.z;
}
#endif

// mulpvf: multiply vector t with lambda f in place

void mulpvf(vec3f *t, fixed f) {
	mulpff(&(t->x), f);
    mulpff(&(t->y), f);
    mulpff(&(t->z), f);
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


// py2vec3f: transform pitch and yaw into a rotated normal vector

vec3f py2vec3f(fixed pitch, fixed yaw) {
    vec3f resp;
    vec3f v;	

	fixed sy, cy, sp, cp;

	sy = sin_f(yaw); sp = sin_f(pitch); cy = cos_f(yaw); cp = cos_f(pitch);

	v = ivec3f(0, 0, int2f(1));

	// pitch (around X)		

    resp = ivec3f(v.x,
                  mulff(v.y, cp)-mulff(v.z, sp),
                  mulff(v.y, sp)+mulff(v.z, cp));

	// yaw (around Y);

	return ivec3f(mulff(resp.x, cy)+mulff(resp.z, sy),
                  resp.y,
                  mulff(resp.z, cy)-mulff(resp.x, sy));
}

// rot: rotate vector t around axes x (pitch) and y (yaw)

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

// itrianglef: initialize trianglef

trianglef itrianglef(vec3f _a, vec3f _b, vec3f _c, texture_t *_tx, tr_id_t _id, bool _flip) {
    trianglef r;
	r.a = _a;
	r.b = _b;
	r.c = _c;
	r.tx = _tx;
	r.id = _id;
	r.flip_texture = _flip;
    return r;
}

// move: transform triangle (offset with vector q then rotated around origin)

trianglef move(trianglef q, vec3f v, fixed pitch, fixed yaw) {
	return itrianglef(rot(subvv(q.a, v), -pitch, -yaw),
                  rot(subvv(q.b, v), -pitch, -yaw),
                  rot(subvv(q.c, v), -pitch, -yaw),
                  q.tx, q.id, q.flip_texture);
}

vec3f normal(trianglef q) {
	return crossp(subvv(q.b, q.a), subvv(q.c, q.a));
}