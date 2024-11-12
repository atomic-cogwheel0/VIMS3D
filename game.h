#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <setjmp.h>

#include "worldobj.h"
#include "VIMS_defs.h"

// prepares game & objects
void init(void);
// handles input, initiates rendering and object ticking
void tick(void);

// exit to main menu after freeing all memory
void quit(void);
// immediately stop everything and exit to ERROR screen
// unsafe quit: does not call destructors and does not free (but stops timer)
void halt(void);
// print a message then halt() the program
void halt_msg(char *msg);

// subsys interop without externs
volatile int *get_gamestate_ptr(void);
jmp_buf *get_jmpbuf_ptr(void);

// possible game states
#define GAMESTATE_PREINIT 0
#define GAMESTATE_RUNNING 1
#define GAMESTATE_QUIT_INPROG 2
#define GAMESTATE_QUIT_DONE 3
#define GAMESTATE_ERR 4

#endif