#ifndef _FIXED_HPP
#define _FIXED_HPP

#include <math.h>
#include <stdint.h>

/*typedef signed long long int64_t;
typedef signed long int32_t;
typedef signed short int16_t;
typedef signed char int8_t;
typedef unsigned long long uint64_t;
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;*/

typedef int16_t fixed_half_t;
typedef int32_t fixed_full_t;
typedef int64_t fixed_double_t;

#define FIXED_PRECISION 8
#define FIXED_CONST (1<<FIXED_PRECISION)

extern uint8_t textures[][8];

typedef fixed_full_t fixed;

fixed int2f(int i);
fixed float2f(float f);
//fixed fft2f(fixed_full_t n);
	
int f2int(fixed t);
float f2float(fixed t);

fixed floor_f(fixed t);
fixed abs_f(fixed t);
fixed mod_f(fixed t, fixed f);

fixed sqrt_f(fixed t);

fixed cos_f(fixed t);
fixed sin_f(fixed t);
fixed tan_f(fixed t);

fixed subff(fixed t, fixed f);
fixed subfi(fixed t, int i);

fixed addff(fixed t, fixed f);
fixed addfi(fixed t, int i);

fixed mulff(fixed t, fixed f);
fixed mulfi(fixed t, int i);

fixed divff(fixed t, fixed f);
fixed divfi(fixed t, int i);

void addpff(fixed *t, fixed f);
void addpfi(fixed *t, int i);

void subpff(fixed *t, fixed f);
void subpfi(fixed *t, int i);

void mulpff(fixed *t, fixed f);
void mulpfi(fixed *t, int i);

void divpff(fixed *t, fixed f);
void divpfi(fixed *t, int i);

int eqff(fixed t, fixed f);
int ltff(fixed t, fixed f);
int leff(fixed t, fixed f);
int gtff(fixed t, fixed f);
int geff(fixed t, fixed f);

#endif
