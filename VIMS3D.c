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

int main(void) {
	unsigned int key;
    volatile unsigned int *gamestate_ptr = get_gamestate_ptr();

    init();

    while(*gamestate_ptr != GAMESTATE_QUIT_DONE) {
		GetKey(&key);
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