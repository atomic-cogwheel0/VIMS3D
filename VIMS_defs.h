#ifndef _VIMS_DEFS_H
#define _VIMS_DEFS_H

#include "fxlib.h"

/*
VIMS_defs:
 defines miscellaneous utility functions
*/

// rebuild all if changed
//#define DEBUG_BUILD
//#define BENCHMARK_RASTER

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

// stores all data needed for a toggle in a tick function
typedef struct {
	bool is_on;
	bool toggle;
	bool prev_toggle;
} toggle_t;

// toggles t->is_on if state has changed to TRUE since last call
void toggle_rising(toggle_t *t, bool state);
// toggles t->is_on if state has changed to FALSE since last call
void toggle_falling(toggle_t *t, bool state);

// status codes
#define S_SUCCESS 0			// function did everything successfully/error-free
#define S_EALLOC -1			// function could not allocate needed RAM
#define S_EEMPTY -2			// function was run with an empty buffer
#define S_EBUFFULL -3		// the subsystem's array is full
#define S_ENEXIST -4		// argument was not in the subsystem's array
#define S_ENULLPTR -5		// function was passed a NULL pointer
#define S_EALREADYINITED -6	// subsystem was already initialized [init() called twice]
#define S_EDOWN -7			// subsystem is down
#define S_EIMPLEMENT -8		// requested feature not implemented

#endif