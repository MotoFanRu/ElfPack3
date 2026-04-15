#ifndef P2K_J2ME_SYSTEM_H
#define P2K_J2ME_SYSTEM_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void PFprintf(const char *format, ...);

extern void *AmMemAllocPointer(int size);
extern void AmMemFreePointer(void *ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_J2ME_SYSTEM_H */
