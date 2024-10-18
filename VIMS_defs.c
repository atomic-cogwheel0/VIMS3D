#include "VIMS_defs.h"

// miscellaneous funcs

void toggle_rising(toggle_t *t, bool state) {
	t->toggle = state;
	// has state changed since last call and is it on?
	if (t->toggle != t->prev_toggle && t->toggle) {
		t->is_on = !t->is_on;
	}
	t->prev_toggle = t->toggle;
}

void toggle_falling(toggle_t *t, bool state) {
	t->toggle = state;
	// has state changed since last call and is it off?
	if (t->toggle != t->prev_toggle && !t->toggle) {
		t->is_on = !t->is_on;
	}
	t->prev_toggle = t->toggle;
}