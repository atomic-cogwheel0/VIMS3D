#include "VIMS_defs.h"
#include "fixed.h"

int snprintf_light(char *dest, size_t len, const char *fmt, ...) {
	va_list ap;
	int d = 0; // iterator of dest
	int f = 0; // iterator of fmt
	char curr;
	int foffset; // stores offset of the first '%' of the currently processed format specifier
	int prec; // precision extracted from fmt
	bool has_prec;
	// arguments from va_arg
	fixed farg, farg_frac, farg_whole; // fixed argument and its fractional and whole parts
	int iarg;
	unsigned int uarg;
	char *sarg;
	int flen = strlen(fmt); // length of formatting string

	char itoa_buf[12]; // length of INT_MIN (-2147483648) in decimal is 11 characters
	int w; // number of bytes written by itoa
	int i; // generic iterator

	va_start(ap, fmt);
	while (f < flen) {
		curr = fmt[f++];
		if (curr != '%') {
			if (d < len) dest[d] = curr;
			d++;
			continue;
		}
		// parse format
		foffset = f-1;
		curr = '\0';
		// loop for two characters (1-digit prec and 1-character fmt specifier)
		while (f < flen && ((f-1) - foffset) <= 2) {
			curr = fmt[f++];
			// check if formatting specifier is a precision specifier
			if ((curr >= '0' && curr <= '9')) {
				continue;
			}
			else break;
		}
		if (f <= flen) {
			// check precision specifier digit
			if (((f-1) - foffset) == 2 && (fmt[f-2] >= '0' && fmt[f-2] <= '9')) {
				prec = (fmt[f-2] - '0');
				has_prec = TRUE;
			}
			// no or invalid prec specifier
			else has_prec = FALSE;

			switch (curr) {
			case 'f':
				farg = va_arg(ap, fixed);
				// split farg
				farg_whole = f2int(farg);
				farg_frac = farg & FIXED_FRAC_MASK;
				// convert whole part first
				w = itoax(itoa_buf, (int)farg_whole, 12, 10);
				for (i = 0; i < w; i++) {
					if (d < len) dest[d] = itoa_buf[i];
					d++;
				}
				// fractional part next
				// write fractional part if there is one, or force writing zeroes when a precision more than 0 was specified
				if (farg_frac != 0 || (has_prec && prec > 0)) {
					// decimal point
					if (d < len) dest[d] = '.';
					d++;

					if (!has_prec) {
						prec = 6; // default precision
					}

					// 1E9 is the largest power of 10 that is less than UINT_MAX
					w = uitoax(itoa_buf, (unsigned int)((uint64_t)farg_frac * 1000000000ULL / FIXED_CONST), 11, 10);

					for (i = 0; i < prec; i++) {
						if (i >= w) {
							if (d < len) dest[d] = '0';
						}
						else {
							if (d < len) dest[d] = itoa_buf[i];
						}
						d++;
					}
					// cut off trailing zeros if not precise by decrementing `d` so the next write will overwrite the first zero
					if (!has_prec) {
						while (dest[--d] == '0')
							;
						d++;
					}
				}
				break;
			case 'd':
				iarg = va_arg(ap, signed int);
				w = itoax(itoa_buf, iarg, 12, 10);
				// copy itoa output to dest
				for (i = 0; i < w; i++) {
					// checking if it still fits within buf
					if (d < len) dest[d] = itoa_buf[i];
					d++;
				}
				break;
			case 'u':
				uarg = va_arg(ap, unsigned int);
				w = uitoax(itoa_buf, uarg, 12, 10);
				// copy uitoa output to dest
				for (i = 0; i < w; i++) {
					if (d < len) dest[d] = itoa_buf[i];
					d++;
				}
				break;
			case 'x':
				uarg = va_arg(ap, unsigned int);
				w = uitoax(itoa_buf, uarg, 12, 16);
				// length specified
				if (has_prec) {
					// number too long
					if (w > prec) {
						for (i = 0; i < prec; i++) {
							if (d < len) dest[d] = '*';
							d++;
						}
						break;
					}
					// write padding zeroes
					for (i = 0; i < prec - w; i++) {
						if (d < len) dest[d] = '0';
						d++;
					}
				}
				for (i = 0; i < w; i++) {
					if (d < len) dest[d] = itoa_buf[i];
					d++;
				}
				break;
			case 's':
				sarg = va_arg(ap, char *);
				// copy nonnull string
				if (sarg != NULL) {
					for (i = 0; sarg[i] != '\0'; i++) {
						if (d < len) dest[d] = sarg[i];
						d++;
					}
				}
				// mark NULL with a '*'
				else {
					if (d < len) dest[d] = '*';
					d++;
				}
				break;
			case '%':
				if (d < len) dest[d] = '%';
				d++;
				break;
			default:
				// write the entire invalid specifier
				for (i = foffset; i < f; i++) {
					if (d < len) dest[d] = fmt[i];
					d++;
				}
				break;
			}
		}
	}

	// terminate string if it has length (practically always)
	if (d < len) {
		dest[d] = '\0';
	}
	else if (len != 0) {
		dest[len-1] = '\0';
	}

	va_end(ap);

	// return number of bytes that have been written if dest was large enough
	return d;
}