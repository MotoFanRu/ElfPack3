#ifndef P2K_C_LIB_H
#define P2K_C_LIB_H

#include "P2K_SDK_Base.h"
#include "P2K_C_Arg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ASCII_NULL                     ('\0')

#if defined(FTR_DONT_DEFINE_STDLIB_FUNCTIONS)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#else
/* TODO: RECHECK THIS!!! */

extern int sprintf(char *buffer, const char *format, ...);
extern int snprintf(char *buffer, size_t maxlen, const char *format, ...);

extern int vsprintf(char *buffer, const char *format, va_list arglist);
extern int vsnprintf(char *buffer, size_t maxlen, const char *format, va_list arglist);

extern void *memset(void *dstpp, int c, size_t len);

#endif /* FTR_DONT_DEFINE_STDLIB_FUNCTIONS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_LIB_H */
