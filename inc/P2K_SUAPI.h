#ifndef P2K_SUAPI_H
#define P2K_SUAPI_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef UINT32                                             SU_SIZE;
typedef INT32                                              SU_TASK_HANDLE;

#define SU_SELF                                            ((SU_TASK_HANDLE) (-2))

enum SU_ENUM_T {
	SU_OK = 0,              /* Successful completion. */
	SU_EDELETED,            /* Resource deleted. */
	SU_EEMPTY,              /* Resource is empty. */
	SU_EINUSE,              /* Resource in use. */
	SU_EINVFUNC,            /* Invalid function. */
	SU_EINVPARAM,           /* Invalid parameter. */
	SU_EINVTIME,            /* Invalid timeout specified. */
	SU_ENOEXIST,            /* Invalid object handle. */
	SU_ENOMEM,              /* Insufficient memory. */
	SU_ETIMEOUT,            /* Timeout occurred before service completed. */
	SU_ETOOMANY,            /* Object resource limit exceeded. */
	SU_EFULL,               /* Resource is full. */
	SU_EUNIMPLEMENTED,      /* Feature not implemented on this platform. */
	SU_EINVEVENT,           /* Event not allocated to calling task. */
	SU_EOWN,                /* Task tried to release mutex it doesn't own. */
	SU_EINVFLAG,            /* Specified flag is unallocated. */
	SU_EINVLPF=SU_EINVFLAG, /* Specified LP flag is unallocated. */
	SU_ENOTFOUND,           /* Object was not found (suFindName). */
	SU_EINVFLGGRP,          /* Specified flag group is invalid. */
	SU_ESTACKTRACE,         /* Error occurred during stack trace. */
	SU_ESTACKINFO,          /* Error occurred while generating stack report. */
	SU_ETASKINFO,           /* Error occurred while generating task report. */
	SU_ESEMFAILED,          /* Internal RTOS call by SUAPI failed. */
	SU_EINIT,               /* Initialization was not done. */
	SU_ESTACKTRACE_EXCEP,   /* Stack trace failed on exception frame. */
	SU_ESTACKTRACE_FUNC,    /* Stack trace failed on function frame. */
	SU_ESTACKTRACE_FRSZ     /* Stack trace failed on frame size calculation. */
};
typedef INT32                                              SU_RET_STATUS;

/*
 * Allocate memory for general use.
 *
 * This service allocates contiguous bytes of memory. Memory is allocated from a set of fixed buffer size
 * memory pools using a best fit algorithm. The product designer is responsible for configuring the
 * buffer pools used by this service.
 *
 * The minimum memory allocated is determined by the fixed memory configuration.
 * Memory is allocated using the smallest buffer that is equal to or larger than `buffer_size`.
 *
 * If configured, memory allocation occurs from fixed memory pools of successively larger buffer sizes when the best
 * fit fixed memory pool is exhausted.
 *
 * For SUAPI on VRTXmc, allocations of 4 bytes or larger are guaranteed to be aligned on a 4 byte boundary.
 * For everything else, allocations are guaranteed to be aligned on a 8 byte boundary.
 *
 * `buffer_size` -- The number of contiguous bytes of memory requested.
 * `error_ptr`   -- Pointer to the location to store the error status. If err is `NULL` and a fatal error
 *                  occurs, then this service calls panic and does not return.
 *
 * Return Value  -- Returns the pointer to the newly allocated memory. Returns `NULL` if an error occurred.
 *
 * If the caller wishes to handle error returns, then a valid pointer must be supplied for the `error_ptr` parameter.
 * Any error that occurs is then returned to the caller via the `error_ptr` parameter. If `error_ptr` is `NULL` and
 * a fatal error occurs, then this service calls panic to handle the error. Panic function is not expected to return.
 * Nonfatal errors that occur when `error_ptr` is `NULL` just return without successfully completing the service.
 *
 * SU_OK         -- Nonfatal  -- No errors detected.
 * SU_ENOMEM     -- Fatal     -- Insufficient memory is available to fulfill request.
 * SU_EFAULT     -- Fatal     -- Memory access failure or memory corruption detected.
 */
extern void *suAllocMem(
	SU_SIZE buffer_size,
	SU_RET_STATUS *error_ptr
);

/*
 * Free allocated memory.
 *
 * This service frees memory previously allocated with `suAllocMem()`.
 *
 * The use of freed memory, including trying to free already freed memory, is undefined and should be avoided.
 * Freeing a `NULL` pointer results in `suFreeMem()` returning immediately without error.
 *
 * `suFreeMem()` is the one service that can always be called to free memory allocated from a SUAPI-provided
 * general memory allocator. No matter which SUAPI-provided general memory allocator allocated the memory,
 * this service will free it appropriately. This service is not intended to be used for freeing nongeneral memory
 * allocations such as messages.
 *
 * Any failures detected by this service result in the service calling panic, which does not return any value.
 */
extern void suFreeMem(
	void *buffer_ptr
);

/*
 * TODO:
 * SU_PORT_HANDLE phandle -- typedef struct SU_PORT_HANDLE_STRUCT *SU_PORT_HANDLE;
 * Huge port struct, not needed here.
 */
extern void suLogData(
	UINT32 *phandle,
	UINT32 msgid,
	UINT32 num_pairs,
	...
);

extern void sc_lock(void);
extern void sc_unlock(void);

extern void suSuspendTask(SU_TASK_HANDLE thandle, SU_RET_STATUS *err);
extern void suResumeTask(SU_TASK_HANDLE thandle, SU_RET_STATUS *err);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SUAPI_H */
