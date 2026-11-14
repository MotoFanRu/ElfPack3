#ifndef P2K_C_LIB_PORTABLE_H
#define P2K_C_LIB_PORTABLE_H

#include "P2K_SDK_Base.h"
#include "P2K_C_Lib.h"
#include "P2K_C_Arg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline INT32 PORTABLE_strlen(const char *src) {
	INT32 cnt = 0;
	if (src != NULL) {
		while (*src != ASCII_NULL) {
			cnt++;
			src++;
		}
	}
	return cnt;
}

static inline INT32 PORTABLE_strlen_line(const char *src) {
	INT32 cnt = 0;
	if (src != NULL) {
		while (*src != '\r' && *src != '\n') {
			cnt++;
			src++;
		}
	}
	return cnt;
}

static inline char *PORTABLE_strncpy(char *dest, const char *src, UINT32 n) {
	if (n == 0) {
		return dest;
	}

	/* Keep original dest pointer for return value. */
	char *d = dest;

	/* Copy characters until we hit '\0' or run out of n. */
	while (n) {
		*d = *src;
		if (*src == ASCII_NULL) {
			break;
		}
		d++;
		src++;
		n--;
	}

	/* Pad remaining bytes with zeros (if we stopped early because of '\0'). */
	while (n) {
		*d++ = ASCII_NULL;
		n--;
	}

	return dest;
}

static inline const char *PORTABLE_strchr(const char *src, int c) {
	if (src == NULL) {
		return NULL;
	}

	const unsigned char uc = (unsigned char) c;

	while (*src != ASCII_NULL) {
		if ((unsigned char) *src == uc) {
			return src;
		}
		src++;
	}

	if (uc == ASCII_NULL) {
		return src;
	}

	return NULL;
}

static inline INT32 PORTABLE_strncmp(const char *str_a, const char *str_b, UINT32 n) {
	while (n--) {
		if (*str_a != *str_b) {
			return (INT32) (unsigned char) *str_a - (INT32) (unsigned char) *str_b;
		}
		if (*str_a == ASCII_NULL) {
			return 0;
		}
		str_a++;
		str_b++;
	}
	return 0;
}

static inline void INNER_TINY_vsnprintf_putc(char *buffer, UINT32 maxlen, UINT32 *pos, char c) {
	if (buffer != NULL && maxlen > 0 && *pos < maxlen - 1) {
		buffer[*pos] = c;
	}
	(*pos)++;
}

static inline void INNER_TINY_vsnprintf_repeat(char *buffer, UINT32 maxlen, UINT32 *pos, char c, UINT32 count) {
	while (count > 0) {
		INNER_TINY_vsnprintf_putc(buffer, maxlen, pos, c);
		count--;
	}
}

/* Internal helper: Format an integer and apply a minimum field width. */
static inline void INNER_TINY_vsnprintf_putint(
	char *buffer, UINT32 maxlen, UINT32 *pos, unsigned long val,
	int negative, int base, int uppercase, UINT32 width, int zero
) {
	char digits[sizeof(unsigned long) * 8];
	UINT32 digit_count = 0;
	UINT32 padding;

	do {
		UINT32 digit;
		if (base == 16) {
			digit = (UINT32) (val & 0xFUL);
			val >>= 4;
		} else {
			digit = (UINT32) (val % 10);
			val /= 10;
		}
		digits[digit_count++] = (char) (digit < 10
			? ('0' + digit)
			: ((uppercase ? 'A' : 'a') + digit - 10));
	} while (val != 0);

	padding = width > digit_count + (negative ? 1 : 0)
		? width - digit_count - (negative ? 1 : 0)
		: 0;

	if (!zero) {
		INNER_TINY_vsnprintf_repeat(buffer, maxlen, pos, ' ', padding);
	}
	if (negative) {
		INNER_TINY_vsnprintf_putc(buffer, maxlen, pos, '-');
	}
	if (zero) {
		INNER_TINY_vsnprintf_repeat(buffer, maxlen, pos, '0', padding);
	}
	while (digit_count > 0) {
		digit_count--;
		INNER_TINY_vsnprintf_putc(buffer, maxlen, pos, digits[digit_count]);
	}
}

static inline void INNER_TINY_vsnprintf_putint64(
	char *buffer, UINT32 maxlen, UINT32 *pos, unsigned long long val,
	int negative, int base, int uppercase, UINT32 width, int zero
) {
	char digits[20];
	UINT32 digit_count = 0;
	UINT32 padding;

	do {
		UINT32 digit;
		if (base == 16) {
			digit = (UINT32) (val & 0xFULL);
			val >>= 4;
		} else {
			digit = (UINT32) (val % 10ULL);
			val /= 10ULL;
		}
		digits[digit_count++] = (char) (digit < 10
			? ('0' + digit)
			: ((uppercase ? 'A' : 'a') + digit - 10));
	} while (val != 0);

	padding = width > digit_count + (negative ? 1 : 0)
		? width - digit_count - (negative ? 1 : 0)
		: 0;

	if (!zero) {
		INNER_TINY_vsnprintf_repeat(buffer, maxlen, pos, ' ', padding);
	}
	if (negative) {
		INNER_TINY_vsnprintf_putc(buffer, maxlen, pos, '-');
	}
	if (zero) {
		INNER_TINY_vsnprintf_repeat(buffer, maxlen, pos, '0', padding);
	}
	while (digit_count > 0) {
		digit_count--;
		INNER_TINY_vsnprintf_putc(buffer, maxlen, pos, digits[digit_count]);
	}
}

/*
 * EXL, 15-Nov-2026:
 *
 * Поскольку ранние телефоны на платформах Patriot и Rainbow не имели
 * функции `vsnprintf()` и её аналогов, мы обязаны в их случае использовать её
 * портативный и упрощённый вариант и для удобства он будет использоваться
 * в большинстве случаев внутреннего использования.
 */
/**
 * Portable vsnprintf implementation optimized for ARM7TDMI-S.
 * Supports: %% %s %c %d %i %u %x %X %p, l/ll integer modifiers, and numeric width.
 */
static inline INT32 PORTABLE_TINY_vsnprintf(char *buffer, UINT32 maxlen, const char *format, va_list arglist) {
	UINT32 pos = 0;

	if (format == NULL) {
		if (buffer != NULL && maxlen > 0) {
			buffer[0] = ASCII_NULL;
		}
		return 0;
	}

	while (*format != ASCII_NULL) {
		if (*format != '%') {
			INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, *format);
			format++;
		} else {
			const char *spec_start = format++;
			int zero = 0;
			UINT32 width = 0;

			if (*format == ASCII_NULL) {
				INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, '%');
				break;
			}

			if (*format == '0') {
				zero = 1;
				format++;
			}

			while (*format >= '0' && *format <= '9') {
				UINT32 digit = (UINT32) (*format - '0');
				width = width > (0x7FFFFFFFUL - digit) / 10
					? 0x7FFFFFFFUL
					: width * 10 + digit;
				format++;
			}

			int long_count = 0;
			if (*format == 'l') {
				long_count = 1;
				format++;
				if (*format == 'l') {
					long_count = 2;
					format++;
				}
			}

			if (*format == ASCII_NULL) {
				while (spec_start < format) {
					INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, *spec_start++);
				}
				break;
			}

			switch (*format) {
				case 's': {
					const char *s = va_arg(arglist, const char *);
					if (s == NULL) {
						s = "(null)";
					}
					UINT32 string_len = 0;
					while (s[string_len] != ASCII_NULL) {
						string_len++;
					}
					for (UINT32 i = 0; i < string_len; ++i) {
						INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, s[i]);
					}
					break;
				}
				case 'c': {
					char c = (char) va_arg(arglist, int);
					INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, c);
					break;
				}
				case 'd':
				case 'i': {
					if (long_count == 2) {
						long long val = va_arg(arglist, long long);
						int negative = val < 0;
						unsigned long long magnitude = negative
							? 0ULL - (unsigned long long) val
							: (unsigned long long) val;
						INNER_TINY_vsnprintf_putint64(
							buffer, maxlen, &pos, magnitude, negative, 10, 0, width, zero
						);
					} else if (long_count == 1) {
						long val = va_arg(arglist, long);
						int negative = val < 0;
						unsigned long magnitude = negative
							? 0UL - (unsigned long) val
							: (unsigned long) val;
						INNER_TINY_vsnprintf_putint(
							buffer, maxlen, &pos, magnitude, negative, 10, 0, width, zero
						);
					} else {
						int val = va_arg(arglist, int);
						int negative = val < 0;
						unsigned long magnitude = negative
							? 0UL - (unsigned long) val
							: (unsigned long) val;
						INNER_TINY_vsnprintf_putint(
							buffer, maxlen, &pos, magnitude, negative, 10, 0, width, zero
						);
					}
					break;
				}
				case 'u':
				case 'x':
				case 'X': {
					int base = *format == 'u' ? 10 : 16;
					if (long_count == 2) {
						unsigned long long val = va_arg(arglist, unsigned long long);
						INNER_TINY_vsnprintf_putint64(
							buffer, maxlen, &pos, val, 0, base, *format == 'X', width, zero
						);
					} else if (long_count == 1) {
						unsigned long val = va_arg(arglist, unsigned long);
						INNER_TINY_vsnprintf_putint(
							buffer, maxlen, &pos, val, 0, base, *format == 'X', width, zero
						);
					} else {
						unsigned long val = (unsigned long) va_arg(arglist, unsigned int);
						INNER_TINY_vsnprintf_putint(
							buffer, maxlen, &pos, val, 0, base, *format == 'X', width, zero
						);
					}
					break;
				}
				case 'p': {
					void *ptr = va_arg(arglist, void *);
					INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, '0');
					INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, 'x');
					INNER_TINY_vsnprintf_putint(
						buffer, maxlen, &pos, (unsigned long) (UINTPTR) ptr, 0, 16, 0,
						width > 2 ? width - 2 : 0, zero
					);
					break;
				}
				case '%': {
					INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, '%');
					break;
				}
				default: {
					while (spec_start <= format) {
						INNER_TINY_vsnprintf_putc(buffer, maxlen, &pos, *spec_start++);
					}
					break;
				}
			}
			format++;
		}
	}

	/* Ensure null termination if buffer is provided and maxlen > 0 */
	if (buffer != NULL && maxlen > 0) {
		if (pos < maxlen) {
			buffer[pos] = ASCII_NULL;
		} else {
			buffer[maxlen - 1] = ASCII_NULL;
		}
	}

	return (INT32) pos;
}

/**
 * Portable snprintf implementation optimized for ARM7TDMI-S.
 * Supports: %% %s %c %d %i %u %x %X %p, l/ll integer modifiers, and numeric width.
 */
static inline INT32 PORTABLE_TINY_snprintf(char *buffer, UINT32 maxlen, const char *format, ...) {
	va_list arglist;
	INT32 result;

	va_start(arglist, format);
	result = PORTABLE_TINY_vsnprintf(buffer, maxlen, format, arglist);
	va_end(arglist);

	return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_LIB_PORTABLE_H */
