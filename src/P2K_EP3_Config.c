#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_UIS_Ustring.h>
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
			} else if (Is_Directive(line, "libpath") == RESULT_OK) {
				char token[EP3_CFG_MAX_PATH] = { ASCII_NULL };
				const char *q = line + 7;
				if (config->libpath_cnt < EP3_CFG_MAX_LIBPATHS) {
					if (Read_Token(&q, token, sizeof(token)) > 0) {
						PORTABLE_strncpy(config->libpaths[config->libpath_cnt], token, EP3_CFG_MAX_PATH - 1);
						config->libpath_cnt++;
						D("[EP3 CFG]: libpath='%s'\n", config->libpaths[config->libpath_cnt - 1]);
					}
				} else {
					L("[EP3 CFG]: Too many libpath entries, max %d.\n", EP3_CFG_MAX_LIBPATHS);
				}
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

// EXL, 31-Oct-2026: Per-image config "<image>.cfg" (e.g. app.elf.cfg), same directive
//   style as ep3.cfg. Only two directives are meaningful here: "load_address" (hex or
//   decimal) and "leave_it" (decimal). Missing file or fields yield the defaults.

static void Image_Config_Defaults(EP3_Image_Cfg_T *cfg) {
	cfg->load_address = 0;
	cfg->leave_it = 0;
}

static void Image_Config_Parse(const char *buffer, EP3_Image_Cfg_T *cfg) {
	const char *p = buffer;

	while (*p) {
		if (Is_Line_Content(p) == RESULT_OK) {
			const char *line = p;

			if (Is_Directive(line, "load_address") == RESULT_OK) {
				const char *q = line + 12;
				cfg->load_address = Read_Number(&q, 0);
				D("[EP3 CFG]: load_address=0x%08X\n", cfg->load_address);
			} else if (Is_Directive(line, "leave_it") == RESULT_OK) {
				const char *q = line + 8;
				cfg->leave_it = Read_Number(&q, 0);
				D("[EP3 CFG]: leave_it=%d\n", cfg->leave_it);
			} else {
				char directive[16] = { ASCII_NULL };
				const char *q = line;
				Read_Token(&q, directive, sizeof(directive));
				L("[EP3 CFG]: Unknown directive '%s', skipped.\n", directive);
			}
		}

		Skip_To_Next_Line(&p);
	}
}

STATUS EP3_Image_Config_Load(const WCHAR *image_path, EP3_Image_Cfg_T *cfg) {
	if ((image_path == NULL) || (image_path[0] == UNICODE_NULL) || (cfg == NULL)) {
		return RESULT_FAIL;
	}

	Image_Config_Defaults(cfg);

	// EXL, 31-Oct-2026: The config path is the image path + ".cfg" suffix.
	WCHAR cfg_path[DL_FS_MAX_URI_NAME_LENGTH + 5];
	UINT32 len = 0;
	while ((image_path[len] != UNICODE_NULL) && (len < DL_FS_MAX_URI_NAME_LENGTH)) {
		cfg_path[len] = image_path[len];
		len++;
	}
	cfg_path[len++] = '.';
	cfg_path[len++] = 'c';
	cfg_path[len++] = 'f';
	cfg_path[len++] = 'g';
	cfg_path[len] = UNICODE_NULL;

	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(cfg_path, DL_FS_OWNER_RESERVED);
	if (file_size < EP3_CFG_TOO_SMALL) {
		D("[EP3 CFG]: No per-image config for this image (%d bytes).\n", file_size);
		return RESULT_OK;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(cfg_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		D("[EP3 CFG]: %s\n", "Failed to open the per-image config.");
		return RESULT_OK;
	}

	char *buffer = (char *) EP3_Memory_Alloc(file_size + 2);
	if (buffer == NULL) {
		L("[EP3 CFG]: Failed to allocate %d bytes.\n", file_size + 2);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	memset(buffer, 0, file_size + 2);

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(buffer, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		L("[EP3 CFG]: %s\n", "Failed to read the per-image config.");
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

	DL_FsCloseFile(file_handle);

	// EXL, 31-Oct-2026: Add proper line endings (if missed).
	if ((buffer[file_size - 1] != '\r') && (buffer[file_size - 1] != '\n')) {
		buffer[file_size + 0] = '\r';
		buffer[file_size + 1] = '\n';
	}

	Image_Config_Parse(buffer, cfg);

	EP3_Memory_Free(buffer);

	L("[EP3 CFG]: Image config: load_address=0x%08X leave_it=%d\n", cfg->load_address, cfg->leave_it);

	return RESULT_OK;
}

// EXL, 01-Nov-2026: Simple option storage API. Text files use the same "<param> <value>"
//   directive format as ep3.cfg (the same tokenizer walks them), binary files are plain
//   blobs. File path = "<path>/<name>", "path" may be NULL/empty for a flat file system.

#define EP3_CFG_MAX_VALUE              (96)

// EXL, 01-Nov-2026: Build "<path>/<name>" (with separator auto-fix) or just "name".
static void CFG_Build_Path(const WCHAR *path, const WCHAR *name, WCHAR *out, UINT32 out_size) {
	UINT32 pos = 0;

	if ((path != NULL) && (path[0] != UNICODE_NULL)) {
		while ((path[pos] != UNICODE_NULL) && (pos < (out_size - 2))) {
			out[pos] = path[pos];
			pos++;
		}
		if ((pos > 0) && (out[pos - 1] != L'/')) {
			out[pos++] = L'/';
		}
	}

	while ((*name != UNICODE_NULL) && (pos < (out_size - 1))) {
		out[pos] = *(name++);
		pos++;
	}
	out[pos] = UNICODE_NULL;
}

// EXL, 01-Nov-2026: Read the whole file to a heap buffer (caller frees). Missing or
//   empty files yield (*out_buffer == NULL, *out_size == 0) without a failure.
static STATUS CFG_TXT_Read_All(const WCHAR *wpath, char **out_buffer, UINT32 *out_size) {
	*out_buffer = NULL;
	*out_size = 0;

	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(wpath, DL_FS_OWNER_RESERVED);
	if (file_size < EP3_CFG_TOO_SMALL) {
		return RESULT_OK;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(wpath, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		return RESULT_OK;
	}

	char *buffer = (char *) EP3_Memory_Alloc(file_size + 2);
	if (buffer == NULL) {
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	memset(buffer, 0, file_size + 2);

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(buffer, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		EP3_Memory_Free(buffer);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	DL_FsCloseFile(file_handle);

	// EXL, 01-Nov-2026: Guarantee proper line endings for the line walker.
	if ((buffer[file_size - 1] != '\r') && (buffer[file_size - 1] != '\n')) {
		buffer[file_size + 0] = '\r';
		buffer[file_size + 1] = '\n';
	}

	*out_buffer = buffer;
	*out_size = file_size;

	return RESULT_OK;
}

// EXL, 01-Nov-2026: Write the whole buffer, truncating any previous content.
static STATUS CFG_Write_All(const WCHAR *wpath, const void *data, UINT32 size) {
	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(wpath, DL_FS_WRITE_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		L("[EP3 CFG]: Failed to open '%p' for writing.\n", wpath);
		return RESULT_FAIL;
	}

	DL_FS_COUNT_T elements_written;
	DL_FS_RESULT_T result = DL_FsWriteFile((void *) data, 1, size, file_handle, &elements_written);
	DL_FsCloseFile(file_handle);

	if ((result != DL_FS_RESULT_SUCCESS) || (elements_written != size)) {
		L("[EP3 CFG]: Failed to write %d bytes (written %d).\n", size, elements_written);
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

// EXL, 01-Nov-2026: Sign-aware decimal / 0xHEX token parser (Read_Number is unsigned).
static INT32 CFG_Parse_Int_Token(const char *token) {
	INT32 sign = 1;
	if ((token[0] == '-') || (token[0] == '+')) {
		sign = (token[0] == '-') ? -1 : 1;
		token++;
	}

	UINT32 val = 0;
	if ((token[0] == '0') && ((token[1] == 'x') || (token[1] == 'X'))) {
		val = hex2addr(token + 2);
	} else {
		while ((token[0] >= '0') && (token[0] <= '9')) {
			val = (val * 10) + (UINT32) (token[0] - '0');
			token++;
		}
	}

	return (INT32) val * sign;
}

// EXL, 01-Nov-2026: Read one option from a "<param> <value>" text file.
STATUS EP3_API_CFG_TXT_ReadOption(
	const WCHAR *path, const WCHAR *name, const char *param, EP3_CFG_Type_T type, void *value, UINT32 value_size
) {
	if ((name == NULL) || (name[0] == UNICODE_NULL) || (param == NULL) || (param[0] == ASCII_NULL) || (value == NULL)) {
		L("[EP3 CFG]: %s\n", "ReadOption: invalid arguments.");
		return RESULT_FAIL;
	}

	WCHAR wpath[DL_FS_MAX_URI_NAME_LENGTH + 1];
	CFG_Build_Path(path, name, wpath, DL_FS_MAX_URI_NAME_LENGTH + 1);

	char *buffer = NULL;
	UINT32 buffer_size = 0;
	if (CFG_TXT_Read_All(wpath, &buffer, &buffer_size) != RESULT_OK) {
		return RESULT_FAIL;
	}
	if (buffer == NULL) {
		D("[EP3 CFG]: %s\n", "ReadOption: option file is missing.");
		return RESULT_FAIL;
	}

	STATUS status = RESULT_FAIL;
	const char *p = buffer;

	while (*p) {
		if (Is_Line_Content(p) == RESULT_OK) {
			char token[EP3_CFG_MAX_VALUE] = { ASCII_NULL };
			const char *q = p;

			if ((Read_Token(&q, token, sizeof(token)) > 0) && (PORTABLE_strncmp(token, param, EP3_CFG_MAX_VALUE) == 0)) {
				if (type == EP3_CFG_TYPE_INT) {
					char num_token[32] = { ASCII_NULL };
					Read_Token(&q, num_token, sizeof(num_token));
					*((INT32 *) value) = CFG_Parse_Int_Token(num_token);
				} else {
					// EXL, 01-Nov-2026: STRING: the rest of the line (comment symbols are
					//   not stripped here, keep them out of stored values).
					Skip_Spaces(&q);
					UINT32 len = 0;
					while (*q && (*q != '\r') && (*q != '\n') && ((len + 1) < value_size)) {
						((char *) value)[len++] = *(q);
						q++;
					}
					((char *) value)[len] = ASCII_NULL;
				}
				status = RESULT_OK;
				break;
			}
		}

		Skip_To_Next_Line(&p);
	}

	EP3_Memory_Free(buffer);

	if (status != RESULT_OK) {
		D("[EP3 CFG]: ReadOption: '%s' not found.\n", param);
	}

	return status;
}

// EXL, 01-Nov-2026: Save one option into a "<param> <value>" text file: the first
//   matching line is replaced in place, comments and all other lines are preserved,
//   a missing option is appended. The file is created when it does not exist.

// EXL, 01-Nov-2026: Save one option into a "<param> <value>" text file: the first
//   matching line is replaced in place, comments and all other lines are preserved,
//   a missing option is appended. The file is created when it does not exist.
// EXL, 01-Nov-2026: Save one option into a "<param> <value>" text file: the first
//   matching line is replaced in place, comments and all other lines are preserved,
//   a missing option is appended. The file is created when it does not exist.
STATUS EP3_API_CFG_TXT_SaveOption(
	const WCHAR *path, const WCHAR *name, const char *param, EP3_CFG_Type_T type, const void *value
) {
	if ((name == NULL) || (name[0] == UNICODE_NULL) || (param == NULL) || (param[0] == ASCII_NULL) || (value == NULL)) {
		L("[EP3 CFG]: %s\n", "SaveOption: invalid arguments.");
		return RESULT_FAIL;
	}

	// EXL, 01-Nov-2026: Render the value text. A STRING must be a single line token.
	char value_text[EP3_CFG_MAX_VALUE] = { ASCII_NULL };
	if (type == EP3_CFG_TYPE_INT) {
		snprintf(value_text, sizeof(value_text), "%d", *((const INT32 *) value));
	} else {
		const char *src = (const char *) value;
		UINT32 len = 0;
		while (src[len] != ASCII_NULL) {
			if ((src[len] == ' ') || (src[len] == '\t') || (src[len] == '\r') || (src[len] == '\n') || ((len + 1) >= sizeof(value_text))) {
				L("[EP3 CFG]: %s\n", "SaveOption: STRING value must be a single line token.");
				return RESULT_FAIL;
			}
			len++;
		}
		PORTABLE_strncpy(value_text, src, sizeof(value_text) - 1);
	}

	WCHAR wpath[DL_FS_MAX_URI_NAME_LENGTH + 1];
	CFG_Build_Path(path, name, wpath, DL_FS_MAX_URI_NAME_LENGTH + 1);

	char *old_buffer = NULL;
	UINT32 old_size = 0;
	if (CFG_TXT_Read_All(wpath, &old_buffer, &old_size) != RESULT_OK) {
		return RESULT_FAIL;
	}

	UINT32 param_len = (UINT32) PORTABLE_strlen(param);
	UINT32 value_len = (UINT32) PORTABLE_strlen(value_text);
	UINT32 new_line_len = param_len + 1 + value_len + 2;

	char *out = (char *) EP3_Memory_Alloc(old_size + new_line_len + 4);
	if (out == NULL) {
		L("[EP3 CFG]: %s\n", "SaveOption: failed to allocate the output buffer.");
		EP3_Memory_Free(old_buffer);
		return RESULT_FAIL;
	}

	// EXL, 01-Nov-2026: Rebuild the file line by line, replacing the first match.
	const char *p = old_buffer;
	char *w = out;
	BOOL replaced = FALSE;

	while (*p) {
		const char *line = p;
		const char *le = p;
		while (*le && (*le != '\r') && (*le != '\n')) {
			le++;
		}
		UINT32 line_len = (UINT32) (le - p);
		UINT32 nl_len = 0;
		if (*le == '\r') {
			nl_len++;
			le++;
		}
		if (*le == '\n') {
			nl_len++;
			le++;
		}

		// EXL, 01-Nov-2026: Content lines only, first token must be the "param".
		BOOL match = FALSE;
		if ((line_len > 0) && (line[0] != '#') && (line[0] != ';') && (line[0] != ' ') && (line[0] != '\t')) {
			UINT32 tlen = 0;
			while ((tlen < line_len) && (line[tlen] != ' ') && (line[tlen] != '\t')) {
				tlen++;
			}
			if ((tlen == param_len) && (PORTABLE_strncmp(line, param, param_len) == 0)) {
				match = TRUE;
			}
		}

		if (match && !replaced) {
			for (UINT32 i = 0; i < param_len; ++i) {
				*(w++) = param[i];
			}
			*(w++) = ' ';
			for (UINT32 i = 0; i < value_len; ++i) {
				*(w++) = value_text[i];
			}
			*(w++) = '\r';
			*(w++) = '\n';
			replaced = TRUE;
		} else {
			for (UINT32 i = 0; i < (line_len + nl_len); ++i) {
				*(w++) = line[i];
			}
		}

		p = le;
	}

	// EXL, 01-Nov-2026: No match — append the option as a new last line.
	if (!replaced) {
		if ((w > out) && (w[-1] != '\n')) {
			*(w++) = '\r';
			*(w++) = '\n';
		}
		for (UINT32 i = 0; i < param_len; ++i) {
			*(w++) = param[i];
		}
		*(w++) = ' ';
		for (UINT32 i = 0; i < value_len; ++i) {
			*(w++) = value_text[i];
		}
		*(w++) = '\r';
		*(w++) = '\n';
	}

	EP3_Memory_Free(old_buffer);

	STATUS status = CFG_Write_All(wpath, out, (UINT32) (w - out));

	EP3_Memory_Free(out);

	L("[EP3 CFG]: SaveOption '%s' = '%s': %s\n", param, value_text, (status == RESULT_OK) ? "saved" : "FAILED");

	return status;
}

// EXL, 01-Nov-2026: Write a binary record, truncating any previous content.
STATUS EP3_API_CFG_BIN_Save(const WCHAR *path, const WCHAR *name, const void *data, UINT32 size) {
	if ((name == NULL) || (name[0] == UNICODE_NULL) || (data == NULL) || (size == 0)) {
		L("[EP3 CFG]: %s\n", "BIN_Save: invalid arguments.");
		return RESULT_FAIL;
	}

	WCHAR wpath[DL_FS_MAX_URI_NAME_LENGTH + 1];
	CFG_Build_Path(path, name, wpath, DL_FS_MAX_URI_NAME_LENGTH + 1);

	STATUS status = CFG_Write_All(wpath, data, size);

	L("[EP3 CFG]: BIN_Save %d bytes: %s\n", size, (status == RESULT_OK) ? "saved" : "FAILED");

	return status;
}

// EXL, 01-Nov-2026: Read a binary record, the file must hold at least "size" bytes.
STATUS EP3_API_CFG_BIN_Read(const WCHAR *path, const WCHAR *name, void *data, UINT32 size) {
	if ((name == NULL) || (name[0] == UNICODE_NULL) || (data == NULL) || (size == 0)) {
		L("[EP3 CFG]: %s\n", "BIN_Read: invalid arguments.");
		return RESULT_FAIL;
	}

	WCHAR wpath[DL_FS_MAX_URI_NAME_LENGTH + 1];
	CFG_Build_Path(path, name, wpath, DL_FS_MAX_URI_NAME_LENGTH + 1);

	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(wpath, DL_FS_OWNER_RESERVED);
	if (file_size < (DL_FS_SIZE_T) size) {
		D("[EP3 CFG]: BIN_Read: file too small (%d < %d bytes).\n", file_size, size);
		return RESULT_FAIL;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(wpath, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		L("[EP3 CFG]: %s\n", "BIN_Read: failed to open the file.");
		return RESULT_FAIL;
	}

	DL_FS_COUNT_T elements_read;
	DL_FS_RESULT_T result = DL_FsReadFile(data, 1, size, file_handle, &elements_read);
	DL_FsCloseFile(file_handle);

	if ((result != DL_FS_RESULT_SUCCESS) || (elements_read != size)) {
		L("[EP3 CFG]: BIN_Read: failed to read %d bytes (read %d).\n", size, elements_read);
		return RESULT_FAIL;
	}

	return RESULT_OK;
}
