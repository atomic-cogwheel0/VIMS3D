#ifndef _GAME_H
#define _GAME_H

#include <stdlib.h>
#include "fxlib.h"
#include "timer.h"

#include "meshrender.h"
#include "VIMS_defs.h"

int RTC_GetTicks(void);

#define TICK_TIMER ID_USER_TIMER1
#define DRAW_TIMER ID_USER_TIMER2

#define TICK_MS 25
#define DRAW_MS 25

void tick(void);
void init(void);
void quit(void);

void toggle_rising(toggle_t *t, bool state);
void toggle_falling(toggle_t *t, bool state);

#define GAMESTATE_PREINIT 0
#define GAMESTATE_RUNNING 1
#define GAMESTATE_QUIT_INPROG 2
#define GAMESTATE_QUIT_DONE 3

#endif