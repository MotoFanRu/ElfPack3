#ifndef P2K_UIS_MEMORY_H
#define P2K_UIS_MEMORY_H

#include "P2K_SDK_Base.h"
#include "P2K_SUAPI.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef UINT32                                             UIS_PARTITION_BLOCK_SIZE_T;

enum UIS_ALLOCATION_STATUS_ENUM_T {
	UIS_MEMORY_ALLOCATION_SUCCESS                            = 0,
	UIS_MEMORY_ALLOCATION_FAILURE_NO_MEMORY_AVAILABLE        = SU_EINVFLGGRP + 1,
	UIS_MEMORY_ALLOCATION_FAILURE_NO_CONTROL_BLOCK_AVAILABLE = SU_EINVFLGGRP + 2,
	UIS_MEMORY_ALLOCATION_FAILURE_SIZE_TOO_BIG               = SU_EINVFLGGRP + 3,
	UIS_MEMORY_ALLOCATION_FAILURE_INVALID_SIZE               = SU_EINVFLGGRP + 4,
	UIS_MEMORY_ALLOCATION_FAILURE_SEMAPHORE                  = SU_EINVFLGGRP + 5,
	UIS_MEMORY_ALLOCATION_FAILURE_NODE_TO_BE_FREED_NOT_FOUND = SU_EINVFLGGRP + 6,
	UIS_MEMORY_ALLOCATION_FAILURE_INVALID_ADDRESS_GIVEN      = SU_EINVFLGGRP + 7,
	UIS_MEMORY_ALLOCATION_FAILURE_CONTROL_ARRAY_CORRUPT      = SU_EINVFLGGRP + 8
};
typedef INT32                                              UIS_ALLOCATION_ERROR_T;

extern void *uisAllocateMemory(UIS_PARTITION_BLOCK_SIZE_T num_bytes, UIS_ALLOCATION_ERROR_T *status);
extern void *uisReAllocMemory(void *address, UIS_PARTITION_BLOCK_SIZE_T new_size, UIS_ALLOCATION_ERROR_T *status);
extern void uisFreeMemory(void *address);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_UIS_MEMORY_H */
