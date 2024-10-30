#include "VIMS_defs.h"
#include "fixed.h"

int snprintf_light(char *dest, size_t len, const char *fmt, ...) {
	va_list ap;
	int d = 0; // iterator of dest
	int f = 0; // iterator of fmt
	char curr;
	int foffset; // stores offset of the first '%' of the currently processed format specifier
	// precision from fmt
	int prec;
	bool has_prec;
	// arguments from va_arg
	fixed farg, farg_frac, farg_whole; // fixed argument and its fractional and whole parts
	int iarg;
	unsigned int uarg;
	int flen = strlen(fmt); // length of formatting string

	char itoa_buf[12]; // length of INT_MIN (-2147483648) in decimal is 11 characters
	int w; // number of bytes written by itoa
	int i; // generic iterator

	int64_t pow; // stores a power of 10 for multiplying fixeds 

	va_start(ap, fmt);
	while (f < flen) {
		curr = fmt[f++];
		if (curr != '%') {
			if (d < len) dest[d] = curr;
			d++;
			continue;
		}
		// parse format
		foffset = f;
		curr = '\0';
		while (f < flen && (f-foffset) <= 2) {
			curr = fmt[f++];
			// check if formatting specifier is a precision specifier
			if ((curr >= '0' && curr <= '9')) {
				continue;
			}
			else break;
		}
		if (f <= flen) {
			if ((f - foffset) == 2 && (fmt[f-2] >= '0' && fmt[f-2] <= '9')) {
				prec = (fmt[f-2] - '0');
				has_prec = TRUE;
			} 
			else has_prec = FALSE;

			switch (curr) {
			case 'f':
				farg = va_arg(ap, fixed);
				farg_whole = f2int(farg);
				farg_frac = farg & FIXED_FRAC_MASK;
				// convert whole part first
				w = itoax(itoa_buf, (int)farg_whole, 11, 10);
				i = 0;
				while (i < w) {
					if (d < len) dest[d] = itoa_buf[i];
					d++;
					i++;
				}
				// fractional part next
				// decimal point
				if (farg_frac != 0) {
					if (d < len) dest[d] = '.';
					d++;

					if (!has_prec) {
						prec = 6; // default precision
					}

					// convert fractional part by dividing a large exponent of 10 suitable for current precision
					pow = 10;
					for (i = 0; i < prec; i++) {
						pow *= 10;
					}
					w = uitoax(itoa_buf, (unsigned int)((fixed_double_t)farg_frac * (fixed_double_t)pow / 4096), 11, 10);

					i = 0;
					while (i < prec) {
						if (i >= w) {
							if (d < len) dest[d] = '0';
						}
						else {
							if (d < len) dest[d] = itoa_buf[i];
						}
						d++;
						i++;
					}
					// cut off trailing zeros if not precise
					if (!has_prec) {
						while (dest[--d] == '0')
							;
						d++;
					}
				}
				break;
			case 'd':
				iarg = va_arg(ap, signed int);
				w = itoax(itoa_buf, iarg, 11, 10);
				// copy itoa output to dest
				i = 0;
				while (i < w) {
					// checking if it still fits within buf
					if (d < len) dest[d] = itoa_buf[i];
					d++;
					i++;
				}
				break;
			case 'u':
				uarg = va_arg(ap, unsigned int);
				w = uitoax(itoa_buf, uarg, 11, 10);
				// copy uitoa output to dest
				i = 0;
				while (i < w) {
					if (d < len) dest[d] = itoa_buf[i];
					d++;
					i++;
				}
				break;
			case 'x':
				uarg = va_arg(ap, unsigned int);
				w = uitoax(itoa_buf, uarg, 11, 16);
				// length specified
				if (has_prec) {
					// number too long
					if (w > prec) {
						i = 0;
						while (i++ < prec) {
							if (d < len) dest[d] = '*';
							d++;
						}
						break;
					}
					// write padding zeroes
					i = 0;
					while (i++ < prec - w) {
						if (d < len) dest[d] = '0';
						d++;
					}
				}
				i = 0;
				while (i < w) {
					if (d < len) dest[d] = itoa_buf[i];
					d++;
					i++;
				}
				break;
			case '%':
				if (d < len) dest[d] = '%';
				d++;
			default:
				break;
			}
		}
		else break;
	}

	// terminate string
	if (d < len) {
		dest[d] = '\0';
	}
	else {
		dest[len-1] = '\0';
	}

	va_end(ap);

	// return number of bytes that have been written if dest was large enough
	return d;
}