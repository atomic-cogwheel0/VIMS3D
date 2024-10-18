#include "fxlib.h"
#include "game.h"

/*
 Very Impressive Mesh System 3D (VIMS3D): a C 3D rendering engine

 The VIMS renders very impressive meshes (wow), with even some billboarding capability :)

  TODO: ability to render EVEN MORE meshes
        maybe grayscale?
        BETTER PERFORMANCE, probably impossible
		also gint and perhaps a C++ port

  fun fact: I've tried to do this with a raytracer,
            it took 500ms to draw 4 triangles in half resolution (64x32), it's now ~80ms for 96... (600x performance yay)
*/ 

void display_error_screen(void);

int main(void) {
	unsigned int key;
	volatile int *gamestate_ptr = get_gamestate_ptr();
	jmp_buf *envptr = get_jmpbuf_ptr();

	// setup longjmp environment for later call from halt()
	if(setjmp(*envptr)) {
		display_error_screen();
	}

	init();

	while(*gamestate_ptr != GAMESTATE_QUIT_DONE) {
		GetKey(&key);
	}
}

void display_error_screen(void) {
	int ka, kb; short uu; // for GetKeyWait
	locate(1,1);
	Print((unsigned char *)"////////ERROR////////");
	Bdisp_PutDisp_DD();
	while (1) {
		if (Bkey_GetKeyWait(&ka, &kb, KEYWAIT_HALTON_TIMERON, 1, 0, &uu) == KEYREP_TIMEREVENT) {
			Bdisp_AreaReverseVRAM(0, 0, 127, 6);
			Bdisp_PutDisp_DD();
		}
	}
}

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