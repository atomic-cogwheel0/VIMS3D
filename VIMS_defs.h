#ifndef VIMS_DEFS_H
#define VIMS_DEFS_H

#include <string.h>
#include <stdarg.h>
#include "fxlib.h"

#include "assembly.h"

/*
VIMS_defs:
 defines miscellaneous utility functions
*/

// CONFIG: rebuild all if flags are changed

// show the debug screen by default
//#define DEBUG_BUILD
// if defined: renders single frame BENCHMARK_RASTER times, shows debuginfo then stops
//#define BENCHMARK_RASTER 160
// this build is on the official sdk (not on gint)
#define OFFICIAL_SDK
// should assertions be checked
#define DO_ASSERT

// typedefs (no <stdint.h> or <stdbool.h> on official SDK)

#ifdef OFFICIAL_SDK
typedef signed long long int64_t;
typedef signed long int32_t;
typedef signed short int16_t;
typedef signed char int8_t;
typedef unsigned long long uint64_t;
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned char bool;
#endif

typedef unsigned char byte;

#define TRUE 1
#define FALSE 0

#define max(a, b) (((a)>(b))?(a):(b))
#define min(a, b) (((a)<(b))?(a):(b))

#define INT_MIN 0x80000000
#define INT_MAX 0x7FFFFFFF
#define UINT_MAX 0xFFFFFFFFU

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
#define S_SUCCESS 0         // function did everything successfully/error-free
#define S_EALLOC -1         // function could not allocate needed RAM
#define S_EEMPTY -2         // function was run with an empty buffer
#define S_EBUFFULL -3       // the subsystem's array is full
#define S_ENEXIST -4        // argument was not in the subsystem's array
#define S_ENULLPTR -5       // function was passed a NULL pointer
#define S_EALREADYINITED -6 // subsystem was already initialized [init() called twice]
#define S_EDOWN -7          // subsystem is down
#define S_EIMPLEMENT -8     // requested feature not implemented
#define S_EUNSPECIFIED -9   // multiple errors have occurred and their nature is unimportant
#define S_EGENERIC -10      // an error has occurred

// if defined, uitoax and itoax will write alphabetical digits (base 11 or more) uppercase, else lowercase
//#define UITOAX_UPPERCASE

// number to str conversion functions
// these return number of bytes written, excluding '\0'
// `to` must hold at least len bytes
// if `to` was too small, bytes written will be 0 and `to` will become "\0"

// unsigned int to str in given base
int uitoax(char *to, unsigned int val, size_t len, int base);
// uitoax() wrapper in base 10
int uitoa(char *to, unsigned int val, size_t len);
// like uitoax, prepends a '-' if negative, handles INT_MIN correctly; len should be at least 2
int itoax(char *to, signed int val, size_t len, int base);
// itoax() wrapper in base 10; len should be at least 2
int itoa(char *to, signed int val, size_t len);

// snprintf misc issues:
// fixed point rounding: TRUNCATION!
// these fixeds aren't precise enough to make rounding worth it
// can't really represent even two decimal places without issues, so just cut the end off

// own snprintf() implementation, supports following formats:
// ------ X is a single digit number (optional)
// %Xf	: fixed according to fixed.h (X digits after decimal point if specified, otherwise trailing zeroes are cut off)
// %d	: int32_t
// %u	: uint32_t
// %Xx	: uint32_t as hex (writes X digits if specified, uses '0' as padding if too short; writes '*' X times if too long)
// %s	: string ('*' if NULL)
// %%	: the character %
// returns the number of bytes excluding the closing '\0', that were written or would have been written to dest if dest was large enough
int snprintf_light(char *dest, size_t len, const char *fmt, ...);

#define STRINGIZE(n) STRINGIZE_INNER(n)
#define STRINGIZE_INNER(x) #x

#ifdef DO_ASSERT
// if given expression is true, continues exec; if it is false, prints error and expression and halt()s program
#define assert(a)	do { \
						if (!(a)) { \
							void halt(void); \
							Bdisp_AllClr_DDVRAM(); \
							PrintXY(0, 8, (unsigned char *)"assertion failed!", 0); \
							PrintMini(0, 17, (unsigned char *)" " #a, MINI_OVER); \
							PrintXY(0, 24, (unsigned char *)"l:" STRINGIZE(__LINE__), 0); \
							PrintXY(0, 32, (unsigned char *)"f:", 0); PrintXY(12, 32, (unsigned char *)strrchr("\\" __FILE__ "\0", '\\') + 1, 0); \
							halt(); \
						} \
					} while(0)
#else
#define assert(a)	do { \
						if (!(a)) { \
							break; \
						} \
					} while(0)
#endif

// generate random integer value within the bounds (min included, max excluded)
#define RANDINT(min, max) (rand()%((max)-(min)) + (min))

#endif