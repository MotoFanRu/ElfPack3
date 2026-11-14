#ifndef P2K_EP3_MEMORY_H
#define P2K_EP3_MEMORY_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * EXL, 15-Nov-2026:
 *
 * Функции для аллокации и освобождения памяти, поведение которых
 * настраивается на этапе компиляции.
 */
extern void *EP3_Memory_Alloc(UINT32 size);
extern void EP3_Memory_Free(void *address);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_MEMORY_H */
