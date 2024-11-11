#include "game.h"

/*
 Very Impressive Mesh System 3D (VIMS3D): a C 3D rendering engine

 The VIMS renders very impressive meshes (wow), with even some billboarding capability :)

  TODO: ability to render EVEN MORE meshes
        maybe grayscale?
        BETTER PERFORMANCE, probably impossible
		also gint and perhaps a C++ port
*/ 

void display_error_screen(void);
void reset_before_exit(void);

int Bkey_GetKeyWait(int *code1, int *code2, int wait_type, int time, int menu, short *unused);

int main(void) {
	// get subsystem interop pointers
	volatile int *gamestate_ptr = get_gamestate_ptr();
	jmp_buf *envptr = get_jmpbuf_ptr();

#ifdef BENCHMARK_RASTER
	unsigned int key;
#endif

	// setup longjmp environment for later call from halt()
	if (setjmp(*envptr)) {
		// longjmp() was called in halt()
		display_error_screen();
		// display_error_screen() does not return (for now at least), but make sure we don't accidentally call init() again anyway
		return 1;
	}

	setup_hires_timer();
	init();

#ifndef BENCHMARK_RASTER
	// wait for game to quit in tick()
	while (*gamestate_ptr != GAMESTATE_QUIT_DONE) {
		tick(); // run a tick
	}
#else
	// do a single tick and show stats
	tick();
	w_print_bench_result();
	GetKey(&key);
#endif
	reset_before_exit();
	return 0;
}

// draw an ERROR message, does not clear content
void display_error_screen(void) {
	int ka, kb; short uu; // for GetKeyWait
	// print top row
	locate(1,1);
	Print((unsigned char *)"////////ERROR////////");
	Bdisp_PutDisp_DD();
	reset_before_exit(); // clear to make sure nothing interferes with operation
	while (1) {
		// wait for a keypress for 1 second (MENU returns to main menu); if no key was pressed, invert 1st row of screen
		if (Bkey_GetKeyWait(&ka, &kb, KEYWAIT_HALTON_TIMERON, 1, 0, &uu) == KEYREP_TIMEREVENT) {
			Bdisp_AreaReverseVRAM(0, 0, 127, 6);
			Bdisp_PutDisp_DD();
		}
	}
}

void reset_before_exit(void) {
	reset_timer_state();
	Keyboard_ClrBuffer(); // clear before returning, because the buffer was not handled properly (with GetKey()) during runtime
}

// ----- SDK funcs -----

int AddIn_main(int isAppli, unsigned short OptionNum)
{
    main();
    return 1;
}

//****************************************************************************
//**************                                              ****************
//**************                 Notice!                      ****************
//**************                                              ****************
//**************  Please do not change the following source.  ****************
//**************                                              ****************
//****************************************************************************

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section

#pragma section _TOP

//****************************************************************************
//  InitializeSystem
//
//  param   :   isAppli   : 1 = Application / 0 = eActivity
//              OptionNum : Option Number (only eActivity)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}

#pragma section