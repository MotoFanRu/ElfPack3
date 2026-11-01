#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_Logger.h>

#include <P2K_EP3_Memory.h>
#include <P2K_EP3_Config.h>
#include <P2K_EP3_Environment.h>

// EXL, 01-Nov-2026: Shared environment variables. One flat table inside the RAM-resident
//   loader, sized by the ep3.cfg limits; every loaded image (ELF/BIN/.so) reads and
//   writes the same variables through the DEF library registered calls.

typedef struct tagEP3_Env_Var {
	BOOL used;
	char *name;                           // Heap copies, sized by the cfg limits.
	char *value;
} EP3_Env_Var_T;

static EP3_Env_Var_T *EP3_Env_Table = NULL;
static UINT32 EP3_Env_Table_Size = 0;
static UINT32 EP3_Env_Name_Max = 0;       // Limits including the NUL terminator space.
static UINT32 EP3_Env_Value_Max = 0;

void EP3_Env_Init(UINT32 max_vars, UINT32 name_max, UINT32 value_max) {
	if (max_vars == 0) {
		max_vars = EP3_CFG_DEFAULT_ENV_MAX;
	}
	if (name_max == 0) {
		name_max = EP3_CFG_DEFAULT_ENV_NAME_MAX;
	}
	if (value_max == 0) {
		value_max = EP3_CFG_DEFAULT_ENV_VALUE_MAX;
	}
	if (max_vars > EP3_ENV_MAX) {
		L("[EP3 ENV]: Variable count clamped to %d (requested %d).\n", EP3_ENV_MAX, max_vars);
		max_vars = EP3_ENV_MAX;
	}
	if (name_max > EP3_ENV_NAME_MAX) {
		L("[EP3 ENV]: Name length clamped to %d (requested %d).\n", EP3_ENV_NAME_MAX, name_max);
		name_max = EP3_ENV_NAME_MAX;
	}
	if (value_max > EP3_ENV_VALUE_MAX) {
		L("[EP3 ENV]: Value length clamped to %d (requested %d).\n", EP3_ENV_VALUE_MAX, value_max);
		value_max = EP3_ENV_VALUE_MAX;
	}

	// EXL, 01-Nov-2026: Re-init drops the previous table with all its strings.
	if (EP3_Env_Table != NULL) {
		for (UINT32 i = 0; i < EP3_Env_Table_Size; ++i) {
			if (EP3_Env_Table[i].used) {
				EP3_Memory_Free(EP3_Env_Table[i].name);
				EP3_Memory_Free(EP3_Env_Table[i].value);
			}
		}
		EP3_Memory_Free(EP3_Env_Table);
		EP3_Env_Table = NULL;
	}

	EP3_Env_Table = (EP3_Env_Var_T *) EP3_Memory_Alloc(sizeof(EP3_Env_Var_T) * max_vars);
	if (EP3_Env_Table == NULL) {
		L("[EP3 ENV]: Failed to allocate %d bytes for %d variables.\n", sizeof(EP3_Env_Var_T) * max_vars, max_vars);
		EP3_Env_Table_Size = 0;
		return;
	}
	memset(EP3_Env_Table, 0, sizeof(EP3_Env_Var_T) * max_vars);
	EP3_Env_Table_Size = max_vars;
	EP3_Env_Name_Max = name_max;
	EP3_Env_Value_Max = value_max;

	L("[EP3 ENV]: Initialized with %d variables (name<%d, value<%d) at 0x%08X.\n",
		EP3_Env_Table_Size, EP3_Env_Name_Max, EP3_Env_Value_Max, EP3_Env_Table
	);
}

static INT32 EP3_Env_Find(const char *name) {
	for (UINT32 i = 0; i < EP3_Env_Table_Size; ++i) {
		EP3_Env_Var_T *var = &EP3_Env_Table[i];
		if (var->used && (PORTABLE_strncmp(var->name, name, EP3_Env_Name_Max) == 0)) {
			return (INT32) i;
		}
	}

	return -1;
}

STATUS EP3_API_setenv(const char *name, const char *value) {
	if (
		(EP3_Env_Table == NULL) || (name == NULL) || (name[0] == ASCII_NULL) ||
		((value != NULL) && (value[0] == ASCII_NULL))
	) {
		L("[EP3 ENV]: %s\n", "setenv: invalid arguments.");
		return RESULT_FAIL;
	}

	UINT32 name_len = (UINT32) PORTABLE_strlen(name);
	if (name_len >= EP3_Env_Name_Max) {
		L("[EP3 ENV]: setenv: name '%s' is too long, max %d.\n", name, EP3_Env_Name_Max - 1);
		return RESULT_FAIL;
	}

	// EXL, 01-Nov-2026: setenv(name, NULL) removes the variable, same as unsetenv.
	if (value == NULL) {
		return EP3_API_unsetenv(name);
	}

	UINT32 value_len = (UINT32) PORTABLE_strlen(value);
	if (value_len >= EP3_Env_Value_Max) {
		L("[EP3 ENV]: setenv: value of '%s' is too long, max %d.\n", name, EP3_Env_Value_Max - 1);
		return RESULT_FAIL;
	}

	INT32 idx = EP3_Env_Find(name);

	if (idx >= 0) {
		// EXL, 01-Nov-2026: Overwrite: shrink or grow the value copy.
		EP3_Env_Var_T *var = &EP3_Env_Table[idx];
		char *new_value = (char *) EP3_Memory_Alloc(value_len + 1);
		if (new_value == NULL) {
			L("[EP3 ENV]: setenv: failed to allocate %d bytes for the value.\n", value_len + 1);
			return RESULT_FAIL;
		}
		PORTABLE_strncpy(new_value, value, value_len);
		new_value[value_len] = ASCII_NULL;

		EP3_Memory_Free(var->value);
		var->value = new_value;

		D("[EP3 ENV]: Set '%s' = '%s' (overwritten).\n", var->name, var->value);
		return RESULT_OK;
	}

	// EXL, 01-Nov-2026: Create: find a free slot.
	for (UINT32 i = 0; i < EP3_Env_Table_Size; ++i) {
		EP3_Env_Var_T *var = &EP3_Env_Table[i];
		if (var->used) {
			continue;
		}

		var->name = (char *) EP3_Memory_Alloc(name_len + 1);
		var->value = (char *) EP3_Memory_Alloc(value_len + 1);
		if ((var->name == NULL) || (var->value == NULL)) {
			L("[EP3 ENV]: setenv: %s\n", "failed to allocate the variable strings.");
			EP3_Memory_Free(var->name);
			EP3_Memory_Free(var->value);
			var->name = NULL;
			var->value = NULL;
			return RESULT_FAIL;
		}
		PORTABLE_strncpy(var->name, name, name_len);
		var->name[name_len] = ASCII_NULL;
		PORTABLE_strncpy(var->value, value, value_len);
		var->value[value_len] = ASCII_NULL;
		var->used = TRUE;

		D("[EP3 ENV]: Set '%s' = '%s'.\n", var->name, var->value);
		return RESULT_OK;
	}

	L("[EP3 ENV]: Too many environment variables, max %d.\n", EP3_Env_Table_Size);

	return RESULT_FAIL;
}

const char *EP3_API_getenv(const char *name) {
	if ((EP3_Env_Table == NULL) || (name == NULL) || (name[0] == ASCII_NULL)) {
		return NULL;
	}

	INT32 idx = EP3_Env_Find(name);
	if (idx < 0) {
		D("[EP3 ENV]: getenv: '%s' not found.\n", name);
		return NULL;
	}

	return EP3_Env_Table[idx].value;
}

STATUS EP3_API_unsetenv(const char *name) {
	if ((EP3_Env_Table == NULL) || (name == NULL) || (name[0] == ASCII_NULL)) {
		L("[EP3 ENV]: %s\n", "unsetenv: invalid arguments.");
		return RESULT_FAIL;
	}

	INT32 idx = EP3_Env_Find(name);
	if (idx < 0) {
		D("[EP3 ENV]: unsetenv: '%s' not found.\n", name);
		return RESULT_FAIL;
	}

	EP3_Env_Var_T *var = &EP3_Env_Table[idx];
	L("[EP3 ENV]: Unset '%s' = '%s'.\n", var->name, var->value);

	EP3_Memory_Free(var->name);
	EP3_Memory_Free(var->value);
	var->name = NULL;
	var->value = NULL;
	var->used = FALSE;

	return RESULT_OK;
}

UINT32 EP3_Env_Count(void) {
	if (EP3_Env_Table == NULL) {
		return 0;
	}

	UINT32 count = 0;
	for (UINT32 i = 0; i < EP3_Env_Table_Size; ++i) {
		if (EP3_Env_Table[i].used) {
			count++;
		}
	}

	return count;
}

STATUS EP3_Env_Get_At(UINT32 index, const char **name, const char **value) {
	if (EP3_Env_Table == NULL) {
		return RESULT_FAIL;
	}

	UINT32 used = 0;
	for (UINT32 i = 0; i < EP3_Env_Table_Size; ++i) {
		if (!EP3_Env_Table[i].used) {
			continue;
		}
		if (used == index) {
			if (name != NULL) {
				*name = EP3_Env_Table[i].name;
			}
			if (value != NULL) {
				*value = EP3_Env_Table[i].value;
			}
			return RESULT_OK;
		}
		used++;
	}

	return RESULT_FAIL;
}

// EXL, 02-Nov-2026: The dispatch table exposed through EP3_ImageArgs_T.api.
//   Extend with new loader API pointers at the END only (ABI stability for images).
const EP3_Loader_Api_T EP3_Loader_Api = {
	EP3_API_setenv,
	EP3_API_getenv,
	EP3_API_unsetenv
};
