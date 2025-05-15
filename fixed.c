#include "fixed.h"

/*
FIXED.C
 functions for fixed point arithmetic
 these are tested and correct (trigonometry is a bit inaccurate)
*/

// zero idea how this works
fixed sqrt_f(fixed f) {
	uint32_t t, q, b, r;
	r = f;
	b = 0x40000000;
	q = 0;
	while(b > 0x40) {
		t = q + b;
		if(r >= t) {
			r -= t;
			q = t + b; // equivalent to q += 2*b
		}
		r <<= 1;
		b >>= 1;
	}
	q >>= 8;
	q >>= 2;
	return (fixed)q;
}

#ifndef MACRO_VAL2F
fixed int2f(int i) {
	return ((fixed)i)*FIXED_CONST;
}

fixed float2f(float f) {
	return (fixed)(f*FIXED_CONST);
}
#endif

#ifndef MACRO_F2VAL
int f2int(fixed t) {
	return t>>FIXED_PRECISION;
}

float f2float(fixed t) {
    return ((float)t)/FIXED_CONST;
}
#endif

fixed abs_f(fixed t) {
	return (t<0)?-t:t;
}

#ifndef MACRO_ADDSUBF
fixed addff(fixed t, fixed f) {
	return t + f;
}
#endif
fixed addfi(fixed t, int i) {
	return t + (i<<FIXED_PRECISION);
}

#ifndef MACRO_ADDSUBF
fixed subff(fixed t, fixed f) {
	return t - f;
}
#endif
fixed subfi(fixed t, int i) {
	return t - (i<<FIXED_PRECISION);
}

#ifndef MACRO_MULDIVF
fixed mulff(fixed t, fixed f) {
	return (fixed)(((fixed_double_t)t * (fixed_double_t)f) >> FIXED_PRECISION);
}
fixed mulfi(fixed t, int i) {
	return t * i;
}

fixed divff(fixed t, fixed f) {
	return (fixed)((((fixed_double_t)t)<<FIXED_PRECISION) / f);
}   
fixed divfi(fixed t, int i) {
	return t / i;
}

fixed shrfi(fixed t, int bits) {
	return t >> bits;
}
fixed shlfi(fixed t, int bits) {
	return t << bits;
}
#endif

fixed sin_f(fixed t) {
	static const fixed hpi = float2f(1.570796f);

	return cos_f(t - hpi);
}

// Bhaskara I-based cosine approximation
fixed cos_f(fixed t) {
	static const fixed hpi = float2f(1.570796f);
	static const fixed pi = float2f(3.141593f);
	static const fixed twopi = float2f(6.283185f);
	static const fixed pi2 = float2f(9.869604f);

	fixed mod, x2, ret;

	mod = mod_f(t + hpi, pi) - hpi;
	x2 = mulff(mod, mod);
	
	ret = divff(pi2 - 4*x2, pi2 + x2);

	if (mod_f(t + hpi, twopi) > pi) {
		ret = -ret; 
	}

	return ret;
}

#ifndef MACRO_FLOORF
fixed floor_f(fixed t) {
	return t & FIXED_WHOLE_MASK;
}
#endif

fixed mod_f(fixed t, fixed f) {
	return t - mulff(floor_f(divff(t, f)), f);
}

fixed clamp_f(fixed val, fixed min, fixed max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

static const fixed pi = float2f(3.141593);
static const fixed one_eighty = int2f(180);

fixed rad2deg(fixed rad) {
	return divff(mulff(rad, one_eighty), pi);
}

fixed deg2rad(fixed deg) {
	return divff(mulff(deg, pi), one_eighty);
}

uint8_t fast_log2(uint8_t n) {
	uint8_t i;
	uint8_t mask = 0x80;
	for (i = 0; i < 8; i++) {
		if (mask & n) {
			break;
		}
		mask >>= 1;
	}
	return 7-i;
}