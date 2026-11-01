#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_UIS_Ustring.h>
#include <P2K_UIS_Ustring_Portable.h>

#include <P2K_EP3_Memory.h>
#include <P2K_EP3_Image_Manager.h>
#include <P2K_EP3_Environment.h>
#include <P2K_EP3_Image_Args.h>

// EXL, 02-Nov-2026: EP3_ImageArgs_T block builder. One heap allocation contains the
//   struct, the argv/envp pointer arrays and every string (WCHAR), so the Image
//   Manager frees it with a single EP3_Memory_Free when the image is unregistered.
//
//   Block layout (sequential bump allocation, 4-byte aligned):
//     [EP3_ImageArgs_T][argv[] argc+1][envp[] env_cnt+1][path][dir]
//     [argv[1..] tokens][envp "NAME=value" strings]

typedef struct tagBump {
	UINT8 *base;
	UINT8 *ptr;
	UINT32 left;
} Bump_T;

static void *Bump_Alloc(Bump_T *bump, UINT32 size) {
	size = (size + 3) & ~3UL;
	if (bump->left < size) {
		return NULL;
	}

	void *ptr = bump->ptr;
	bump->ptr += size;
	bump->left -= size;

	return ptr;
}

static void Bump_Put_Wstr(Bump_T *bump, const WCHAR **slot, const WCHAR *src) {
	UINT32 len = (UINT32) PORTABLE_u_strlen(src) + 1;
	WCHAR *dst = (WCHAR *) Bump_Alloc(bump, len * sizeof(WCHAR));
	if (dst == NULL) {
		*slot = NULL;
		return;
	}
	for (UINT32 i = 0; i < len; ++i) {
		dst[i] = src[i];
	}
	*slot = dst;
}

static void Bump_Put_Wstr_From_Char(Bump_T *bump, const WCHAR **slot, const char *src, UINT32 len) {
	WCHAR *dst = (WCHAR *) Bump_Alloc(bump, (len + 1) * sizeof(WCHAR));
	if (dst == NULL) {
		*slot = NULL;
		return;
	}
	for (UINT32 i = 0; i < len; ++i) {
		dst[i] = (WCHAR) ((UINT8) src[i]);
	}
	dst[len] = UNICODE_NULL;
	*slot = dst;
}

// EXL, 02-Nov-2026: Count whitespace-separated tokens of an ASCII args string.
static UINT32 Count_Tokens(const char *params) {
	UINT32 count = 0;
	const char *p = params;

	while (*p) {
		while ((*p == ' ') || (*p == '\t')) {
			p++;
		}
		if (*p == ASCII_NULL) {
			break;
		}
		count++;
		while ((*p != ASCII_NULL) && (*p != ' ') && (*p != '\t')) {
			p++;
		}
	}

	return count;
}

// EXL, 02-Nov-2026: ASCII string length up to the first separator (end of token).
static UINT32 Token_Len(const char *p) {
	UINT32 len = 0;
	while ((p[len] != ASCII_NULL) && (p[len] != ' ') && (p[len] != '\t')) {
		len++;
	}
	return len;
}

// EXL, 02-Nov-2026: Copy the directory part of a WCHAR path (with trailing '/').
static UINT32 Dir_Len(const WCHAR *path) {
	UINT32 last_sep = 0;
	BOOL found = FALSE;

	for (UINT32 i = 0; path[i] != UNICODE_NULL; ++i) {
		if (path[i] == L'/') {
			last_sep = i;
			found = TRUE;
		}
	}

	return found ? (last_sep + 1) : 0;
}

EP3_ImageArgs_T *EP3_ImageArgs_Build(const WCHAR *path, const char *params, UINT32 event) {
	if ((path == NULL) || (path[0] == UNICODE_NULL)) {
		return NULL;
	}

	// EXL, 02-Nov-2026: Pass 1 — measure everything.
	UINT32 path_len = (UINT32) PORTABLE_u_strlen(path) + 1;
	UINT32 dir_len = Dir_Len(path);
	UINT32 argc = 1;                      // argv[0] = path.
	const char *p = params;
	UINT32 env_cnt = EP3_Env_Count();

	if (params != NULL) {
		argc += Count_Tokens(params);
	}

	UINT32 argv_bytes_len = 0;
	UINT32 env_bytes_len = 0;
	for (UINT32 i = 0; i < argc - 1; ++i) {
		UINT32 len = Token_Len(p);
		argv_bytes_len += len + 1;
		p += len;
		while ((*p == ' ') || (*p == '\t')) {
			p++;
		}
	}
	for (UINT32 i = 0; i < env_cnt; ++i) {
		const char *name = NULL;
		const char *value = NULL;
		if (EP3_Env_Get_At(i, &name, &value) == RESULT_OK) {
			env_bytes_len += (UINT32) PORTABLE_strlen(name) + 1 + (UINT32) PORTABLE_strlen(value) + 1;
		}
	}

	UINT32 total = sizeof(EP3_ImageArgs_T) + ((argc + 1) * sizeof(WCHAR *)) + ((env_cnt + 1) * sizeof(WCHAR *)) +
		(path_len * sizeof(WCHAR)) + (dir_len * sizeof(WCHAR)) +
		((argv_bytes_len + env_bytes_len) * sizeof(WCHAR)) + 8;

	// EXL, 02-Nov-2026: Pass 2 — fill.
	Bump_T bump;
	bump.base = (UINT8 *) EP3_Memory_Alloc(total);
	if (bump.base == NULL) {
		return NULL;
	}
	bump.ptr = bump.base;
	bump.left = total;

	EP3_ImageArgs_T *args = (EP3_ImageArgs_T *) Bump_Alloc(&bump, sizeof(EP3_ImageArgs_T));
	if (args == NULL) {
		EP3_Memory_Free(bump.base);
		return NULL;
	}
	memset(args, 0, sizeof(EP3_ImageArgs_T));

	const WCHAR **argv = (const WCHAR **) Bump_Alloc(&bump, (argc + 1) * sizeof(WCHAR *));
	const WCHAR **envp = (const WCHAR **) Bump_Alloc(&bump, (env_cnt + 1) * sizeof(WCHAR *));

	Bump_Put_Wstr(&bump, &args->path, path);
	Bump_Put_Wstr(&bump, &args->dir, path);   // Full path, truncated to dir below.
	args->event = event;
	args->img_man = &EP3_Image_Manager;
	args->api = &EP3_Loader_Api;

	// EXL, 02-Nov-2026: Truncate the dir copy to the last '/'.
	if (args->dir != NULL) {
		((WCHAR *) args->dir)[dir_len] = UNICODE_NULL;
	}

	if (argv != NULL) {
		argv[0] = args->path;
		const char *tok = params;
		for (UINT32 i = 1; i < argc; ++i) {
			while ((*tok == ' ') || (*tok == '\t')) {
				tok++;
			}
			UINT32 len = Token_Len(tok);
			Bump_Put_Wstr_From_Char(&bump, &argv[i], tok, len);
			tok += len;
		}
		argv[argc] = NULL;
	}
	args->argc = argc;
	args->argv = argv;

	if (envp != NULL) {
		for (UINT32 i = 0; i < env_cnt; ++i) {
			const char *name = NULL;
			const char *value = NULL;
			if (EP3_Env_Get_At(i, &name, &value) != RESULT_OK) {
				envp[i] = NULL;
				continue;
			}
			UINT32 name_len = (UINT32) PORTABLE_strlen(name);
			UINT32 value_len = (UINT32) PORTABLE_strlen(value);
			WCHAR *dst = (WCHAR *) Bump_Alloc(&bump, (name_len + 1 + value_len + 1) * sizeof(WCHAR));
			if (dst == NULL) {
				envp[i] = NULL;
				continue;
			}
			UINT32 pos = 0;
			for (UINT32 j = 0; j < name_len; ++j) {
				dst[pos++] = (WCHAR) ((UINT8) name[j]);
			}
			dst[pos++] = (WCHAR) ((UINT8) '=');
			for (UINT32 j = 0; j < value_len; ++j) {
				dst[pos++] = (WCHAR) ((UINT8) value[j]);
			}
			dst[pos] = UNICODE_NULL;
			envp[i] = dst;
		}
		envp[env_cnt] = NULL;
	}
	args->envp = envp;

	return args;
}
