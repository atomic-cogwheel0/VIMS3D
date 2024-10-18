#ifndef _GAME_H
#define _GAME_H

#include <stdlib.h>
#include <setjmp.h>
#include "fxlib.h"
#include "timer.h"

#include "worldobj.h"
#include "VIMS_defs.h"

int RTC_GetTicks(void);

// timer config
#define TICK_TIMER ID_USER_TIMER1
#define TICK_MS 25

// handles input, initiates rendering and object ticking
void tick(void);
// prepares game & objects
void init(void);
// exit to main menu
void quit(void);

// exit to ERROR screen
void halt(void);
// print a message then halt the program
void halt_msg(char *msg);

// subsys interop
int *get_gamestate_ptr(void);
jmp_buf *get_jmpbuf_ptr(void);

// possible game states
#define GAMESTATE_PREINIT 0
#define GAMESTATE_RUNNING 1
#define GAMESTATE_QUIT_INPROG 2
#define GAMESTATE_QUIT_DONE 3
#define GAMESTATE_ERR 4

#endif