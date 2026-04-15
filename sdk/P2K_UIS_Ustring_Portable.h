#ifndef P2K_UIS_USTRING_PORTABLE_H
#define P2K_UIS_USTRING_PORTABLE_H

#include "P2K_SDK_Base.h"
#include "P2K_UIS_Ustring.h"
#include "P2K_C_Lib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline INT32 PORTABLE_u_strlen(const WCHAR *src) {
	INT32 cnt = 0;
	if (src != NULL) {
		while (*src != UNICODE_NULL) {
			cnt++;
			src++;
		}
	}
	return cnt;
}

static inline WCHAR *PORTABLE_u_strncat(WCHAR *dst, const WCHAR *src, INT32 len) {
	if (len <= 0 || dst == NULL || src == NULL) {
		return dst;
	}

	WCHAR *anchor = dst;

	/* Rewind to the end of `dst` string. */
	while (*dst != UNICODE_NULL) {
		dst++;
	}

	/* Copy up to `len` characters or until `src` will be end. */
	while (len-- > 0) {
		*dst = *src;
		if (*src == UNICODE_NULL) {
			break;
		}
		dst++;
		src++;
	}
	*dst = UNICODE_NULL;

	return anchor;
}

static inline WCHAR *PORTABLE_u_atou(const char *src, WCHAR *dst) {
	if (src == NULL || dst == NULL) {
		return dst;
	}

	INT32 cnt = 0;
	while (src[cnt] != ASCII_NULL) {
		dst[cnt] = (WCHAR) ((UINT8) src[cnt]);
		cnt++;
	}
	dst[cnt] = UNICODE_NULL;

	return dst;
}

static inline char *PORTABLE_u_utoa(const WCHAR *src, char *dst) {
	if (src == NULL || dst == NULL) {
		return dst;
	}

	INT32 cnt = 0;
	while (src[cnt] != UNICODE_NULL) {
		dst[cnt] = (char) src[cnt];
		cnt++;
	}
	dst[cnt] = ASCII_NULL;

	return dst;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_UIS_USTRING_PORTABLE_H */
