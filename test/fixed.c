#include "fixed.h"

fixed sqrt_f(fixed f) {
    uint32_t t, q, b, r;
	fixed res = 0;
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
    return q;
}

fixed int2f(int i) {
    return ((fixed)i)*FIXED_CONST;
}

fixed float2f(float f) {
    return (fixed)(f*FIXED_CONST);
}

int f2int(fixed t) {
    return t>>FIXED_PRECISION;
}

float f2float(fixed t) {
    return ((float)t)/FIXED_CONST;
}

fixed abs_f(fixed t) {
	return (t<0)?-t:t;
}

fixed addff(fixed t, fixed f) {
    return t + f;
}
fixed addfi(fixed t, int i) {
    return t + (i<<FIXED_PRECISION);
}

fixed subff(fixed t, fixed f) {
    return t - f;
}
fixed subfi(fixed t, int i) {
    return t - (i<<FIXED_PRECISION);
}

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

void addpff(fixed *t, fixed f) {
    (*t) += f;
}
void addpfi(fixed *t, int i) {
    (*t) += (i<<FIXED_PRECISION);
}

void subpff(fixed *t, fixed f) {
    (*t) -= f;
}
void subpfi(fixed *t, int i) {
    (*t) -= (i<<FIXED_PRECISION);
}

void mulpff(fixed *t, fixed f) {
    (*t) = (fixed)(((fixed_double_t)t * (fixed_double_t)f) >> FIXED_PRECISION);
}
void mulpfi(fixed *t, int i) {
    (*t) *= i;
}

void divpff(fixed *t, fixed f) {
    (*t) = (fixed)((((fixed_double_t)t)<<FIXED_PRECISION) / f);
}   
void divpfi(fixed *t, int i) {
    (*t) /= i;
}

fixed sin_f(fixed t) {
	fixed  fouropi = float2f(1.273240f);
	fixed fouropi2 = float2f(0.405285f);
	fixed       pi = float2f(3.141593f);
	fixed    twopi = float2f(6.283185f);

	fixed mod, ret;

    mod = mod_f(t, pi);

    ret = mulff(fouropi, mod) - mulff(fouropi2, mulff(mod, mod));

    if (mod_f(t, twopi) > pi) {
        ret = -ret;    
    }

    return ret;
}

/*fixed fixed::sin_d(void) {
	return(fixed(sin(tofloat())));
}*/

fixed cos_f(fixed t) {
	fixed      hpi = float2f(1.570796f);

	return sin_f(t + hpi);
}

fixed floor_f(fixed t) {
	return int2f(f2int(t));
}

fixed mod_f(fixed t, fixed f) {
    return t - mulff(floor_f(divff(t, f)), f);
}
