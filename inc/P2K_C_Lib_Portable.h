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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_LIB_PORTABLE_H */
