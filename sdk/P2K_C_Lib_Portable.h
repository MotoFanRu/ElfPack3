#ifndef P2K_C_LIB_PORTABLE_H
#define P2K_C_LIB_PORTABLE_H

#include "P2K_SDK_Base.h"
#include "P2K_C_Lib.h"

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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_LIB_PORTABLE_H */
