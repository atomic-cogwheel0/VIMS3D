#ifndef _GAME_H
#define _GAME_H

#include <stdlib.h>
#include <setjmp.h>
#include "fxlib.h"
#include "timer.h"

#include "worldobj.h"
#include "VIMS_defs.h"

int RTC_GetTicks(void);

#define TICK_TIMER ID_USER_TIMER1
#define DRAW_TIMER ID_USER_TIMER2

#define TICK_MS 25
#define DRAW_MS 25

void tick(void);
void init(void);
void quit(void);

void halt(void);
// print a message then halt the program
void halt_msg(char *msg);

int *get_gamestate_ptr(void);
jmp_buf *get_jmpbuf_ptr(void);

#define GAMESTATE_PREINIT 0
#define GAMESTATE_RUNNING 1
#define GAMESTATE_QUIT_INPROG 2
#define GAMESTATE_QUIT_DONE 3
#define GAMESTATE_ERR 4

#endif