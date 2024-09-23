#include "vec.h"

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

fixed dotp(vec3f t, vec3f v) {
	return mulff(t.x, v.x) + mulff(t.y, v.y) + mulff(t.z, v.z);
}

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

void mulpvv(vec3f *t, fixed f) {
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

vec3f py2vec3f(fixed pitch, fixed yaw) {
    vec3f resp;
    vec3f v;	

	fixed sy, cy, sp, cp;

	sy = sin_f(yaw); sp = sin_f(pitch); cy = cos_f(yaw); cp = cos_f(pitch);

	v = ivec3f(0, 0, int2f(1));

	//pitch (around X)		

    resp = ivec3f(v.x,
                  mulff(v.y, cp)-mulff(v.z, sp),
                  mulff(v.y, sp)+mulff(v.z, cp));

	//yaw (around Y);

	return ivec3f(mulff(resp.x, cy)+mulff(resp.z, sy),
                  resp.y,
                  mulff(resp.z, cy)-mulff(resp.x, sy));
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


trianglef itrianglef(vec3f _a, vec3f _b, vec3f _c, unsigned char _color, uint32_t _id) {
    trianglef r;
	r.a = _a;
	r.b = _b;
	r.c = _c;
	r.color = _color;
	r.id = _id;
	r.zindex = -1;
    return r;
}

/*vec3f trianglef::normalf(void) {
	vec3f n = (b-a).crossp(c-a);
	return n;
}*/

trianglef move(trianglef t, vec3f v, fixed pitch, fixed yaw) {
	return itrianglef(rot(subvv(t.a, v), -pitch, -yaw),
                      rot(subvv(t.b, v), -pitch, -yaw),
                      rot(subvv(t.c, v), -pitch, -yaw), 
                      t.color, t.id);
}

/*signed short trianglef::getclr(int x, int y) {
	fixed denom, dot00, dot01, dot02, dot11, dot12, u, v;
	vec3f v0, v1, v2;
	vec3f p = vec3f(fixed(x), fixed(y), 0);	

	v0 = c - a;
	v1 = b - a;
	v2 = p - a;
	
	dot00 = v0.dotp(v0);
	dot01 = v0.dotp(v1);
	dot02 = v0.dotp(v2);
	dot11 = v1.dotp(v1);
	dot12 = v1.dotp(v2);
	
	denom = (dot00 * dot11 - dot01 * dot01);
	u = (dot11 * dot02 - dot01 * dot12) / denom;
	v = (dot00 * dot12 - dot01 * dot02) / denom;

	if ((u < 0) || (v < 0) || (u + v >= 1)) {
		return -1;
	}

	return textures[color][(u*8).toint()] & (1<<((v*8).toint()));
}*/

/*fixed max(fixed a, fixed b) {
	return a>b ? fixed(a) : fixed(b);
}

fixed min(fixed a, fixed b) {
	return a<b ? fixed(a) : fixed(b);
}

void trianglef::getbox2i(int *arr) {
	arr[0] = min(min(a.x, b.x), c.x).toint();
	arr[1] = min(min(a.y, b.y), c.y).toint();	
	arr[2] = max(max(a.x, b.x), c.x).toint();
	arr[3] = max(max(a.y, b.y), c.y).toint();
}*/

