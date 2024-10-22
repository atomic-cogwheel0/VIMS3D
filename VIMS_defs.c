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

void uitoa(char *to, unsigned int val, int maxlen) {
	int i = 0, j;
	char c;
	// convert in reverse order
	do {
		to[i++] = val % 10 + '0';
		val /= 10;
	} while (val > 0 && i < maxlen);
	to[i] = '\0';
	// reverse the string
	for (j = 0, i = i-1; j < i; j++, i--) {
		c = to[j];
		to[j] = to[i];
		to[i] = c;
	}
}