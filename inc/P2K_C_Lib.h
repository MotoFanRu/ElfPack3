#ifndef P2K_C_LIB_H
#define P2K_C_LIB_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ASCII_NULL                                         ('\0')

extern int snprintf(char *str, size_t maxlen, const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_LIB_H */
