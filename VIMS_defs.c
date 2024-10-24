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
	uitoax(to, val, maxlen, 10);
}

void uitoax(char *to, unsigned int val, int maxlen, int base) {
	int i = 0, j;
	char c;
	int modulo;
	// convert in reverse order
	do {
		modulo = val % base;
		if (modulo < 10) {
			to[i++] = modulo + '0';
		}
		else {
#ifdef UITOAX_UPPERCASE
			to[i++] = modulo - 10 + 'A';
#else
			to[i++] = modulo - 10 + 'a';
#endif
		}
		val /= base;
	} while (val > 0 && i < maxlen);
	to[i] = '\0';
	// reverse the string
	for (j = 0, i = i-1; j < i; j++, i--) {
		c = to[j];
		to[j] = to[i];
		to[i] = c;
	}
}
