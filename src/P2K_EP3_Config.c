#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_File_System.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_Config.h>

// EXL, 31-Oct-2026: ep3.cfg parser. Same whole-file read + line-walk style as the
//   ep3.def parser (P2K_EP3_DEF_Library.c). One tokenizer handles settings directives
//   and the "autorun" directive uniformly, so future directives can be added easily.

#define EP3_CFG_TOO_SMALL              (2)

// EXL, 31-Oct-2026: Same as hex2addr in P2K_EP3_DEF_Library.c, kept local to keep
//   the config parser self-contained.
static UINT32 hex2addr(const char *p) {
	UINT32 addr = 0;
	while (*p) {
		char c = *(p++);
		if (c >= '0' && c <= '9') {
			addr = (addr << 4) | (c - '0');
		} else if (c >= 'a' && c <= 'f') {
			addr = (addr << 4) | (c - 'a' + 10);
		} else if (c >= 'A' && c <= 'F') {
			addr = (addr << 4) | (c - 'A' + 10);
		} else {
			break;
		}
	}
	return addr;
}

static void Skip_Spaces(const char **p) {
	while ((**p == ' ') || (**p == '\t')) {
		(*p)++;
	}
}

static void Skip_To_Next_Line(const char **p) {
	while (**p && (**p != '\n') && (**p != '\r')) {
		(*p)++;
	}
	while ((**p == '\n') || (**p == '\r')) {
		(*p)++;
	}
}

// EXL, 31-Oct-2026: Read one whitespace-terminated token into "dst" (always NULL-terminated),
//   advance "*p" past it. Returns token length or 0 if no token left on the line.
static UINT32 Read_Token(const char **p, char *dst, UINT32 dst_size) {
	Skip_Spaces(p);

	UINT32 len = 0;
	while ((**p != ASCII_NULL) && (**p != ' ') && (**p != '\t') && (**p != '\r') && (**p != '\n')) {
		if ((len + 1) < dst_size) {
			dst[len] = **p;
			len++;
		}
		(*p)++;
	}
	dst[len] = ASCII_NULL;

	return len;
}

static UINT32 Read_Number(const char **p, UINT32 def_val) {
	char token[16] = { ASCII_NULL };

	if (Read_Token(p, token, sizeof(token)) == 0) {
		return def_val;
	}

	// EXL, 31-Oct-2026: Decimal or 0xHEX (hex2addr parses both forms).
	if ((token[0] == '0') && ((token[1] == 'x') || (token[1] == 'X'))) {
		return hex2addr(token + 2);
	}

	UINT32 val = 0;
	for (UINT32 i = 0; (token[i] >= '0') && (token[i] <= '9'); ++i) {
		val = (val * 10) + (UINT32) (token[i] - '0');
	}
	return val;
}

static STATUS Is_Line_Content(const char *line) {
	if (line == NULL || line[0] == ASCII_NULL) {
		return RESULT_FAIL;
	}

	// EXL, 31-Oct-2026: Full-line comments only ('#' or ';'), no inline comments.
	if (line[0] == '#' || line[0] == ';' || line[0] == '\r' || line[0] == '\n' || line[0] == ' ' || line[0] == '\t') {
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

void EP3_Config_Defaults(EP3_Config_T *config) {
	memset(config, 0, sizeof(EP3_Config_T));
	PORTABLE_strncpy(config->deflib, EP3_ELF_LDR_DEF_NAME, EP3_CFG_MAX_PATH - 1);
	config->lib_capacity = EP3_CFG_DEFAULT_LIB_CAPACITY;
	config->max_elfs = EP3_CFG_DEFAULT_MAX_ELFS;
}

// EXL, 31-Oct-2026: "autorun <path> [0xEVENT] [args...]" — path is a single token,
//   the optional event is a 0xHEX token, the rest of the line (verbatim) is args.
static void Parse_Autorun(const char *p, EP3_Autorun_Entry_T *entry) {
	char token[EP3_CFG_MAX_PATH] = { ASCII_NULL };

	if (Read_Token(&p, token, sizeof(token)) == 0) {
		L("[EP3 CFG]: %s\n", "Autorun line without a path, skipped.");
		return;
	}
	PORTABLE_strncpy(entry->uri, token, EP3_CFG_MAX_PATH - 1);

	// EXL, 31-Oct-2026: Optional event, must start with "0x" to not eat the args string.
	Skip_Spaces(&p);
	if ((p[0] == '0') && (p[1] == 'x')) {
		entry->event = hex2addr(p + 2);
		char ev_token[16] = { ASCII_NULL };
		Read_Token(&p, ev_token, sizeof(ev_token));
	}

	// EXL, 31-Oct-2026: The rest of the line is the args string (may contain spaces).
	Skip_Spaces(&p);
	UINT32 args_len = 0;
	while (*p && (*p != '\r') && (*p != '\n')) {
		if ((args_len + 1) < EP3_CFG_MAX_ARGS) {
			entry->args[args_len] = *p;
			args_len++;
		}
		p++;
	}
	entry->args[args_len] = ASCII_NULL;

	D("[EP3 CFG]: Autorun: '%s' event=0x%08X args='%s'\n", entry->uri, entry->event, entry->args);
}

// EXL, 31-Oct-2026: Directive match: exact prefix followed by whitespace or end of line,
//   so "max_elfs" does not swallow "max_elfs2"-style words.
static STATUS Is_Directive(const char *line, const char *name) {
	UINT32 name_len = (UINT32) PORTABLE_strlen(name);
	if (PORTABLE_strncmp(line, name, name_len) != 0) {
		return RESULT_FAIL;
	}
	char next = line[name_len];
	if ((next == ASCII_NULL) || (next == ' ') || (next == '\t')) {
		return RESULT_OK;
	}
	return RESULT_FAIL;
}

static UINT32 Config_Parse(const char *buffer, EP3_Config_T *config) {
	const char *p = buffer;
	UINT32 autorun_cnt = 0;

	while (*p) {
		if (Is_Line_Content(p) == RESULT_OK) {
			const char *line = p;

			if (Is_Directive(line, "deflib") == RESULT_OK) {
				char token[EP3_CFG_MAX_PATH] = { ASCII_NULL };
				// EXL, 31-Oct-2026: Skip the directive word itself, take the value token.
				const char *q = line + 6;
				if (Read_Token(&q, token, sizeof(token)) > 0) {
					PORTABLE_strncpy(config->deflib, token, EP3_CFG_MAX_PATH - 1);
					D("[EP3 CFG]: deflib='%s'\n", config->deflib);
				}
			} else if (Is_Directive(line, "lib_capacity") == RESULT_OK) {
				const char *q = line + 12;
				config->lib_capacity = Read_Number(&q, EP3_CFG_DEFAULT_LIB_CAPACITY);
				D("[EP3 CFG]: lib_capacity=%d\n", config->lib_capacity);
			} else if (Is_Directive(line, "max_elfs") == RESULT_OK) {
				const char *q = line + 8;
				config->max_elfs = Read_Number(&q, EP3_CFG_DEFAULT_MAX_ELFS);
				D("[EP3 CFG]: max_elfs=%d\n", config->max_elfs);
			} else if (Is_Directive(line, "autorun") == RESULT_OK) {
				if (autorun_cnt < EP3_CFG_MAX_AUTORUN) {
					const char *q = line + 7;
					Parse_Autorun(q, &config->autorun[autorun_cnt]);
					autorun_cnt++;
				} else {
					L("[EP3 CFG]: Too many autorun entries, max %d.\n", EP3_CFG_MAX_AUTORUN);
				}
			} else {
				char directive[16] = { ASCII_NULL };
				const char *q = line;
				Read_Token(&q, directive, sizeof(directive));
				L("[EP3 CFG]: Unknown directive '%s', skipped.\n", directive);
			}
		}

		Skip_To_Next_Line(&p);
	}

	return autorun_cnt;
}

STATUS EP3_Config_Load(EP3_Config_T *config) {
	if (config == NULL) {
		L("[EP3 CFG]: %s\n", "Pointer to config is NULL!");
		return RESULT_FAIL;
	}

	EP3_Config_Defaults(config);

	WCHAR file_path[PATH_MAX_SHORT];
	if (!EP3_Find_Internal_System_Component(EP3_ELF_LDR_CFG_NAME, file_path)) {
		L("[EP3 CFG]: Failed to find '%s' system component, using defaults.\n", EP3_ELF_LDR_CFG_NAME);
		return RESULT_OK;
	}

	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(file_path, DL_FS_OWNER_RESERVED);
	if (file_size < EP3_CFG_TOO_SMALL) {
		L("[EP3 CFG]: File '%s' too small (%d bytes), using defaults.\n", EP3_ELF_LDR_CFG_NAME, file_size);
		return RESULT_OK;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		L("[EP3 CFG]: Failed to open '%s' file, using defaults.\n", EP3_ELF_LDR_CFG_NAME);
		return RESULT_OK;
	}

	// EXL, 31-Oct-2026: Read the whole file into RAM, then walk it line by line.
	char *buffer = (char *) EP3_Memory_Alloc(file_size + 2);
	if (buffer == NULL) {
		L("[EP3 CFG]: Failed to allocate %d bytes.\n", file_size + 2);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}

	memset(buffer, 0, file_size + 2);

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(buffer, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		L("[EP3 CFG]: Failed to read '%s' into memory at '0x%08X' address.\n", EP3_ELF_LDR_CFG_NAME, buffer);
		EP3_Memory_Free(buffer);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	if (elements_read != 1) {
		L("[EP3 CFG]: elements_read=%d, should be 1.\n", elements_read);
		EP3_Memory_Free(buffer);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	L("[EP3 CFG]: Loaded %d bytes to '0x%08X' memory address.\n", file_size, buffer);

	DL_FsCloseFile(file_handle);

	// EXL, 31-Oct-2026: Add proper line endings (if missed).
	if ((buffer[file_size - 1] != '\r') && (buffer[file_size - 1] != '\n')) {
		buffer[file_size + 0] = '\r';
		buffer[file_size + 1] = '\n';
	}

	UINT32 autorun_cnt = Config_Parse(buffer, config);

	// EXL, 31-Oct-2026: Config_Parse only returns the count, store it in the config here.
	config->autorun_cnt = autorun_cnt;

	EP3_Memory_Free(buffer);

	L("[EP3 CFG]: Config parsed: lib_capacity=%d max_elfs=%d autorun=%d\n",
		config->lib_capacity, config->max_elfs, autorun_cnt
	);

	return RESULT_OK;
}
