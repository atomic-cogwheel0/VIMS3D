#ifndef HIRES_TIMER_H
#define HIRES_TIMER_H

#include "VIMS_defs.h"

// the hires timer module uses the CPU's TMU1 to create a 1/32768 second precision timer

typedef uint8_t bits8;
typedef uint16_t bits16;

typedef volatile struct {
	uint32_t TCOR; // timer constant register
	uint32_t TCNT; // timer counter
	union {
		struct {
			bits16      : 7;
			bits16 UNF  : 1;  // underflow
			bits16      : 2;
			bits16 UNIE : 1; // underflow interrupt
			bits16 CKEG : 2; // clock edge
			bits16 TPSC : 3; // timer prescaler
		} TCR;
		bits16 TCRv;
	} TCR;  // timer control register
} sh_tmu_t;

typedef volatile union {
	struct {
		bits8      : 5;
		bits8 STR2 : 1;
		bits8 STR1 : 1;
		bits8 STR0 : 1;
	} TSTR;
	bits8 TSTRv;
} sh_tstr_t;

void cpu_get_timer_addrs(sh_tmu_t **tmu, sh_tstr_t **tstr);

uint32_t calc_pphi(void);

#define TIMER_TICK_MAX 0xFFFFFFFFUL

int setup_hires_timer(void);
uint32_t timer_us(void);
uint32_t timer_us_max(void);
int reset_timer_state(void);

void copy_tmu(sh_tmu_t *to, sh_tmu_t *from);

#endif