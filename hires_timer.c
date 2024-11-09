#include "hires_timer.h"

static sh_tmu_t tmu1_backup;
static sh_tstr_t tstr_backup;
static bool is_inited = FALSE; 

static volatile sh_tmu_t *tmu1;
static volatile sh_tstr_t *tstr;

static uint32_t tick_per_sec;

#define UNSET 2
int cpuSH4 = UNSET;

bool isSH4(void) {
	if (cpuSH4 == UNSET) {
		cpuSH4 = (((*(volatile uint32_t *)0xFF000030) & 0xffffff00) == 0x10300b00);
	}
	// we can assume that the processor model does not change in a single run
	return cpuSH4 ? TRUE : FALSE;
}

void cpu_get_timer_addrs(sh_tmu_t **tmu, sh_tstr_t **tstr) {
	// test Processor Version Register for SH4 cpu
	if (isSH4()) {
		*tstr = (sh_tstr_t *)0xA4490004;
		*tmu = (sh_tmu_t *)0xA4490014;
	}
	else {
		*tstr = (sh_tstr_t *)0xFFFFFE92;
		*tmu = (sh_tmu_t *)0xFFFFFEA0;
	}
}

void copy_tmu(sh_tmu_t *to, sh_tmu_t *from) {
	to->TCOR = from->TCOR;
	to->TCR.TCRv = from->TCR.TCRv;
}

// register addresses
#define FRQCR_7305 (*(uint32_t *)0xa4150000)
#define FLLFRQ_7305 (*(uint32_t *)0xa4150050)
#define PLLCR_7305 (*(uint32_t *)0xa4150024)

#define FRQCR_7705 (*(uint16_t *)0xFFFFFF80)

uint32_t calc_pphi(void) {
	uint32_t pll, fll;
	uint32_t base;
	// calculate peripheral clock speed based on processor model
	if (isSH4()) {
		pll = ((FRQCR_7305 & (63 << 24)) >> 24) + 1; // FRQCR.STC
		fll = (FLLFRQ_7305 & 2047) >> ((FLLFRQ_7305 & (3 << 14)) ? 1 : 0);

		base = 32768U;

		if (PLLCR_7305 & (1 << 12)) base *= fll; // PLLCR.FLLE
		if (PLLCR_7305 & (1 << 14)) base *= pll; // PLLCR.PLLE
		
		return base >> ((FRQCR_7305 & 15) + 1);
	}
	else {
		pll = 14745600*2 * (((FRQCR_7705 & (3 << 8)) >> 8) + 1);

		return pll / ((FRQCR_7705 & 3) + 1);
	}
}

int setup_hires_timer(void) {
	if (is_inited) return 1;

	cpu_get_timer_addrs(&tmu1, &tstr);

	// backup regs
	copy_tmu(&tmu1_backup, tmu1);

	tstr_backup = *tstr;

	tstr->TSTR.STR1 = 0; // disable TMU1

	tmu1->TCOR = TIMER_TICK_MAX;
	tmu1->TCNT = TIMER_TICK_MAX;
	tmu1->TCR.TCR.UNIE = 1; // enable underflow interrupt 
	tmu1->TCR.TCR.CKEG = 0; // count on rising edge
	tmu1->TCR.TCR.TPSC = 1; // Pphi/16

	tstr->TSTR.STR1 = 1; // enable TMU1

	tick_per_sec = calc_pphi() / 16;

	is_inited = TRUE;

	return 0;
}

uint32_t timer_us(void) {
	if (!is_inited) return 0;

	// convert raw tick value to microseconds
	return (uint32_t) (((uint64_t)TIMER_TICK_MAX - tmu1->TCNT) * 1000000ULL / tick_per_sec);
}

uint32_t timer_us_max(void) {
	if (!is_inited) return 0;

	return (uint32_t) ((uint64_t)TIMER_TICK_MAX * 1000000ULL / tick_per_sec);
}

int reset_timer_state(void) {
	if (!is_inited) return 1;

	tstr->TSTR.STR1 = 0; // disable TMU1

	// restore regs
	copy_tmu(tmu1, &tmu1_backup);
	*tstr = tstr_backup;

	is_inited = FALSE;

	return 0;
}