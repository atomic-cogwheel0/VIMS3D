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

int uitoax(char *to, unsigned int val, size_t len, int base) {
	int i = 0, j, l;
	char c;
	int modulo;
	if (len == 0 || to == NULL) {
		return 0;
	}
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
	} while (val > 0 && i < len);
	// return empty string if number didn't fit in `to`
	if (i >= len) {
		i = 0;
	}
	to[i] = '\0';
	l = i;
	// reverse the string
	for (j = 0, i = i-1; j < i; j++, i--) {
		c = to[j];
		to[j] = to[i];
		to[i] = c;
	}
	return l;
}

int uitoa(char *to, unsigned int val, size_t len) {
	return uitoax(to, val, len, 10);
}

int itoax(char *to, signed int val, size_t len, int base) {
	unsigned int uval; // unsigned value to print
	if (len == 0 || to == NULL) {
		return 0;
	}
	if (val < 0) {
		// if len is less than 2, it can't fit a minus sign and the number, so return
		if (len == 1) {
			to[0] = '\0';
			return 0;
		}
		to[0] = '-';
		// handle INT_MIN safely (-INT_MIN does not fit into signed int)
		if (val == INT_MIN) {
			uval = ((unsigned int)INT_MAX)+1;
		}
		else {
			uval = (unsigned int) -val;
		}
		// convert uval to str with a character already filled
		return uitoax(to+1, uval, len-1, base) + 1;
	}
	else {
		// no '-' sign, just do an uitoax; val is safe to cast to unsigned
		return uitoax(to, (unsigned int) val, len, base);
	}
}

int itoa(char *to, signed int val, size_t len) {
	return itoax(to, val, len, 10);
}