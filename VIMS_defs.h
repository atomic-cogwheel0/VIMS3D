#ifndef _VIMS_DEFS_H
#define _VIMS_DEFS_H

//#define DEBUG_BUILD

// typedefs (no <stddef.h>)

typedef signed long long int64_t;
typedef signed long int32_t;
typedef signed short int16_t;
typedef signed char int8_t;
typedef unsigned long long uint64_t;
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned char byte;

typedef unsigned char bool;
#define TRUE 1
#define FALSE 0

#define max(a, b) (((a)>(b))?(a):(b))
#define min(a, b) (((a)<(b))?(a):(b))

// defined and used in game.c

typedef struct {
	bool is_on;
	bool toggle;
	bool prev_toggle;
} toggle_t;

#endif