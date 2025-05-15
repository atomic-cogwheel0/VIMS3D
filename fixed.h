#ifndef FIXED_H
#define FIXED_H

#include <mathf.h>

#include "VIMS_defs.h"

/*
FIXED POINT MATH LIBRARY

precision: 20+12; 32bit (4byte) =>  FIXED_MAX = 524287.999755859375
									FIXED_MIN = -524288
									FIXED_EPSILON = 0.000244140625

operation names:
 add/sub/mul/div + (p = in-place e.g. +=) + first operand type + second operand type

the following operations do not need extra functions (use the built-in operators):
	add/sub ff
	mul/div fi
*/

// fixed point constants
#define FIXED_MAX ((fixed)0x7FFFFFFF)
#define FIXED_MIN ((fixed)0x80000000)
#define FIXED_EPSILON ((fixed)1)
#define FIXED_PRECISION 12
#define FIXED_CONST (1<<FIXED_PRECISION)
#define FIXED_WHOLE_MASK ((fixed)(0xFFFFFFFF>>FIXED_PRECISION)<<FIXED_PRECISION)
#define FIXED_FRAC_MASK (~FIXED_WHOLE_MASK)

// for angle unit conversion (done as floats)
#define DEG2RAD_MULT (3.1415926535898f/180.0f)
#define RAD2DEG_MULT (180.0f/3.1415926535898f)

typedef int16_t fixed_half_t;
typedef int32_t fixed_full_t;
typedef int64_t fixed_double_t;
typedef fixed_full_t fixed;

// defines certain functions as macros or real functions
// rebuild all if changed
#define MACRO_VAL2F
#define MACRO_F2VAL
#define MACRO_MULDIVF
#define MACRO_ADDSUBF
#define MACRO_FLOORF

#ifdef MACRO_VAL2F
#define int2f(i) (((fixed)(int)(i))<<FIXED_PRECISION)
#define float2f(f) ((fixed)(((float)(f))*FIXED_CONST))
#else
#pragma inline(int2f)
fixed int2f(int i);
#pragma inline(float2f)
fixed float2f(float f);
#endif 

#ifdef MACRO_F2VAL
#define f2int(t) ((t)>>FIXED_PRECISION)
#define f2float(t) (((float)(t))/FIXED_CONST)
#else
#pragma inline(f2int)
int f2int(fixed t);
#pragma inline(f2float)
float f2float(fixed t);
#endif

#ifdef MACRO_MULDIVF
#define mulff(t, f) ((fixed)(((fixed_double_t)(t)*(fixed_double_t)(f))>>FIXED_PRECISION))
#define mulfi(t, i) ((fixed)((t)*(i)))
#define divff(t, f) ((fixed)((((fixed_double_t)(t))<<FIXED_PRECISION)/(f)))
#define divfi(t, i) ((fixed)((t)/(i)))
#define shrfi(t, bits) ((fixed)((t)>>(bits)))
#define shlfi(t, bits) ((fixed)((t)<<(bits)))
#else
#pragma inline(mulff)
fixed mulff(fixed t, fixed f);
fixed mulfi(fixed t, int i);
#pragma inline(divff)
fixed divff(fixed t, fixed f);
fixed divfi(fixed t, int i);
fixed shrfi(fixed t, int bits);
fixed shlfi(fixed t, int bits);
#endif 

#define divshiftfi shrfi
#define mulshiftfi shlfi

// these work with normal + and -
#ifdef MACRO_ADDSUBF
#define addff(t, f) ((t)+(f))
#define subff(t, f) ((t)-(f))
#else
fixed addff(fixed t, fixed f);
fixed subff(fixed t, fixed f);
#endif

fixed addfi(fixed t, int i);
fixed subfi(fixed t, int i);

// these clash with <math.h> without the underscores

#ifdef MACRO_FLOORF
#define floor_f(t) ((t) & FIXED_WHOLE_MASK)
#else
#pragma inline(floor_f)
fixed floor_f(fixed t);
#endif

#pragma inline(abs_f)
fixed abs_f(fixed t);
#pragma inline(mod_f)
fixed mod_f(fixed t, fixed f);

fixed sqrt_f(fixed t);

fixed cos_f(fixed t);
fixed sin_f(fixed t);

fixed rad2deg(fixed rad);
fixed deg2rad(fixed deg);

fixed clamp_f(fixed val, fixed min, fixed max);
// as fixed_full_t is the same as int, this is possible
#define clamp_i(val, min, max) (int)clamp_f((fixed)(val), (fixed)(min), (fixed)(max))

// find log of power of 2
uint8_t fast_log2(uint8_t n);

#endif