#ifndef P2K_C_LIB_H
#define P2K_C_LIB_H

#include "P2K_SDK_Base.h"
#include "P2K_C_Arg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ASCII_NULL                                         ('\0')

extern int sprintf(char *buffer, const char *format, ...);
extern int snprintf(char *buffer, size_t maxlen, const char *format, ...);

extern int vsprintf(char *buffer, const char *format, va_list arglist);
extern int vsnprintf(char *buffer, size_t maxlen, const char *format, va_list arglist);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_LIB_H */
