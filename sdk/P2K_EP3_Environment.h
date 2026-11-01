#ifndef P2K_EP3_ENVIRONMENT_H
#define P2K_EP3_ENVIRONMENT_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 01-Nov-2026: Shared environment variables for all loaded images (ELF/BIN/.so).
//   One key-value table inside the RAM-resident loader, initialized from ep3.cfg
//   (env_max / env_name_max / env_value_max). Every image sees the same variables;
//   names and values are heap copies, so callers may pass stack buffers.
//
//   setenv(getenv) semantics:
//     - setenv with value == NULL removes the variable (same as unsetenv).
//     - setenv always overwrites an existing value.
//     - getenv returns a pointer INTO the store: valid until the next setenv/unsetenv
//       of that name, read-only for the caller. NULL when the name is unknown.
//
//   Registered into the DEF library table at boot, so images call them like any
//   firmware function.

#define EP3_ENV_MAX                    (32)     // Compile-time caps, cfg clamps to these.
#define EP3_ENV_NAME_MAX               (64)
#define EP3_ENV_VALUE_MAX              (128)

// EXL, 01-Nov-2026: Init the table (drops any previous one). Called from the config
//   stage; without Init every call is a logged no-op (safe for NOR contexts).
extern void EP3_Env_Init(UINT32 max_vars, UINT32 name_max, UINT32 value_max);

extern STATUS EP3_API_setenv(const char *name, const char *value);
extern const char *EP3_API_getenv(const char *name);
extern STATUS EP3_API_unsetenv(const char *name);
extern UINT32 EP3_Env_Count(void);

// EXL, 02-Nov-2026: Enumerate USED variables by position (0..EP3_Env_Count()-1).
extern STATUS EP3_Env_Get_At(UINT32 index, const char **name, const char **value);

// EXL, 02-Nov-2026: ElfPack API dispatch table, exposed to images through
//   EP3_ImageArgs_T.api — an alternative way to call loader functions without
//   going through the DEF library. Extensible: append new pointers at the end.
typedef struct tagEP3_Loader_Api {
	STATUS (*setenv)(const char *name, const char *value);
	const char *(*getenv)(const char *name);
	STATUS (*unsetenv)(const char *name);
} EP3_Loader_Api_T;

extern const EP3_Loader_Api_T EP3_Loader_Api;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_ENVIRONMENT_H */
