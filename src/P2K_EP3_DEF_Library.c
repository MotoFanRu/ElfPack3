#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_Hash_Portable.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_File_System.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_DEF_Library.h>

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

static STATUS Is_Line_PFW(const char *line) {
	if (line == NULL || line[0] == ASCII_NULL) {
		return RESULT_FAIL;
	}

	if (line[0] == '#' && line[1] == ' ' && line[2] == 'P' && line[3] == 'F' && line[4] == 'W') {
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static STATUS Is_Line_VER(const char *line) {
	if (line == NULL || line[0] == ASCII_NULL) {
		return RESULT_FAIL;
	}

	if (line[0] == '#' && line[1] == ' ' && line[2] == 'V' && line[3] == 'E' && line[4] == 'R') {
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static STATUS Is_Line_CPU(const char *line) {
	if (line == NULL || line[0] == ASCII_NULL) {
		return RESULT_FAIL;
	}

	if (line[0] == '#' && line[1] == ' ' && line[2] == 'C' && line[3] == 'P' && line[4] == 'U') {
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static STATUS Is_Line_Content(const char *line) {
	if (line == NULL || line[0] == ASCII_NULL) {
		return RESULT_FAIL;
	}

	if (line[0] == '#' || line[0] == '\r' || line[0] == '\n' || line[0] == ' ') {
		return RESULT_FAIL;
	}

	if (line[0] != '0' || line[1] != 'x') {
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

static void Skip_To_Next_Line(const char **p) {
	while (**p && **p != '\n' && **p != '\r') {
		(*p)++;
	}
	while (**p == '\n' || **p == '\r') {
		(*p)++;
	}
}

static STATUS Check_PFW(const char *p) {
	INT32 pfw_len = PORTABLE_strlen_line(p + DEF_LIB_PARSER_HEADER_VAL);
	if (pfw_len <= 0) {
		return RESULT_FAIL;
	}

	char pfw_str[DEF_LIB_MAX_SYM_NAME_LENGTH] = { ASCII_NULL };
	PORTABLE_strncpy(pfw_str, p + DEF_LIB_PARSER_HEADER_VAL, pfw_len);

	UINT32 separator_offset = PORTABLE_strcspn(pfw_str, "_");
	if (separator_offset == 0) {
		return RESULT_FAIL;
	}
	pfw_str[separator_offset] = ASCII_NULL;

	const char *EP3_phone_str = (const char *) GET_DATA_ADDR(EP3_phone);
	if (PORTABLE_strncmp(pfw_str, EP3_phone_str, separator_offset) != 0) {
		L("[EP3 DEF]: Wrong phone: '%s' != '%s'\n", pfw_str, EP3_phone_str);
		return RESULT_FAIL;
	}

	const char *EP3_firmware_str = (const char *) GET_DATA_ADDR(EP3_firmware);
	if (PORTABLE_strncmp(pfw_str + (separator_offset + 1), EP3_firmware_str, pfw_len - (separator_offset + 1)) != 0) {
		L("[EP3 DEF]: Wrong firmware: '%s' != '%s'\n", pfw_str + (separator_offset + 1), EP3_firmware_str);
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

static STATUS Check_VER(const char *p) {
	INT32 ver_len = PORTABLE_strlen_line(p + DEF_LIB_PARSER_HEADER_VAL);
	if (ver_len <= 0) {
		return RESULT_FAIL;
	}

	char ver_str[DEF_LIB_MAX_SYM_NAME_LENGTH] = { ASCII_NULL };
	PORTABLE_strncpy(ver_str, p + DEF_LIB_PARSER_HEADER_VAL, ver_len);

	UINT32 separator_offset = PORTABLE_strcspn(ver_str, "_");
	if (separator_offset == 0) {
		return RESULT_FAIL;
	}
	ver_str[separator_offset] = ASCII_NULL;

	if (PORTABLE_strncmp(ver_str, DEF_LIB_EP3_VERSION_SLUG, separator_offset) != 0) {
		L("[EP3 DEF]: Wrong version: '%s' != '%s'\n", ver_str, DEF_LIB_EP3_VERSION_SLUG);
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

static STATUS Check_CPU(const char *p) {
	INT32 cpu_len = PORTABLE_strlen_line(p + DEF_LIB_PARSER_HEADER_VAL);
	if (cpu_len <= 0) {
		return RESULT_FAIL;
	}

	char cpu_str[DEF_LIB_MAX_SYM_NAME_LENGTH] = { ASCII_NULL };
	PORTABLE_strncpy(cpu_str, p + DEF_LIB_PARSER_HEADER_VAL, cpu_len);

	if (PORTABLE_strncmp(cpu_str, DEF_LIB_PARSER_HEADER_CPU_NONE, cpu_len) == 0) {
		L("[EP3 DEF]: Wrong CPU: '%s'!\n", DEF_LIB_PARSER_HEADER_CPU_NONE);
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

static STATUS Check_Content(const char *p, DEF_LIB_Symbol *symbols, UINT32 i) {
	INT32 sym_len = PORTABLE_strlen_line(p + DEF_LIB_PARSER_CONTENT_NAME);
	if (sym_len <= 0) {
		return RESULT_FAIL;
	}

	char sym_str[DEF_LIB_MAX_SYM_NAME_LENGTH] = { ASCII_NULL };
	PORTABLE_strncpy(sym_str, p + DEF_LIB_PARSER_CONTENT_NAME, sym_len);

	symbols[i].hash = FNV1A32(sym_str);
	symbols[i].addr = hex2addr(p + DEF_LIB_PARSER_CONTENT_ADDR);

	D("Added: 0x%08X 0x%08X %s\n", symbols[i].hash, symbols[i].addr, sym_str);

	return RESULT_OK;
}

static UINT32 DEF_Library_Parse(const char *buffer, DEF_LIB_Symbol *def_symbols) {
	STATUS is_pfw_okay = RESULT_FAIL;
	STATUS is_ver_okay = RESULT_FAIL;
	STATUS is_cpu_okay = RESULT_FAIL;

	const char *p = buffer;
	UINT32 i = 0;

	while (*p && i < DEF_LIB_MAX_SYM_COUNT) {
		if (Is_Line_PFW(p) == RESULT_OK) {
			is_pfw_okay = Check_PFW(p);
		} else if (Is_Line_VER(p) == RESULT_OK) {
			is_ver_okay = Check_VER(p);
		} else if (Is_Line_CPU(p) == RESULT_OK) {
			is_cpu_okay = Check_CPU(p);
		} else if (Is_Line_Content(p) == RESULT_OK) {
			if (Check_Content(p, def_symbols, i) != RESULT_OK) {
				L("[EP3 DEF]: Bad content line at symbol: %d\n", i);
				return 0;
			}
			i++;
		}

		Skip_To_Next_Line(&p);
	}

	if (is_pfw_okay != RESULT_OK) {
		L("[EP3 DEF]: %s\n", "Cannot parse PFW from header.");
		return 0;
	}
	if (is_ver_okay != RESULT_OK) {
		L("[EP3 DEF]: %s\n", "Cannot parse VER from header.");
		return 0;
	}
	if (is_cpu_okay != RESULT_OK) {
		L("[EP3 DEF]: %s\n", "Cannot parse CPU from header.");
		return 0;
	}

	L("[EP3 DEF]: Library parsed successfully, symbols: %d\n", i);
	return i;
}

UINT32 EP3_DEF_Library_Load(DEF_LIB_Symbol *def_symbols) {
	if (!def_symbols) {
		L("[EP3 DEF]: %s\n", "Pointer to DEF Library is NULL!\n");
		return 0;
	}

	WCHAR file_path[PATH_MAX_SHORT];
	if (!EP3_Find_Internal_System_Component(EP3_ELF_LDR_DEF_NAME, file_path)) {
		L("[EP3 DEF]: Failed to find '%s' system component.\n", EP3_ELF_LDR_DEF_NAME);
		return 0;
	}

	/* Use `DL_FsSGetFileSize()` instead `DL_FsGetFileSize()` to reduce the number of functions. */
	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(file_path, DL_FS_OWNER_RESERVED);
	if (file_size < DEF_LIB_TOO_SMALL) {
		L("[EP3 DEF]: File '%s' too small (%d bytes).\n", EP3_ELF_LDR_DEF_NAME, file_size);
		return 0;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		L("[EP3 DEF]: Failed to open '%s' file.\n", EP3_ELF_LDR_DEF_NAME);
		return 0;
	}

	/* Add additional 4 bytes to add line ending characters. */
	char *load_addr = (char *) EP3_Memory_Alloc(file_size + 4);
	if (load_addr == NULL) {
		L("[EP3 DEF]: Failed to allocate %d bytes of memory.\n", file_size + 4);
		DL_FsCloseFile(file_handle);
		return 0;
	}

	memset(load_addr, 0, file_size + 4);

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(load_addr, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		L("[EP3 DEF]: Failed to read '%s' into memory at '0x%08X' address.\n", EP3_ELF_LDR_DEF_NAME, load_addr);
		DL_FsCloseFile(file_handle);
		return 0;
	}
	if (elements_read != 1) {
		L("[EP3 DEF]: elements_read=%d, should be 1.\n", elements_read);
		DL_FsCloseFile(file_handle);
		return 0;
	}
	L("[EP3 DEF]: Loaded %d bytes to '0x%08X' memory address.\n", file_size, load_addr);

	DL_FsCloseFile(file_handle);

	/* Add a proper line endings (if missed). */
	if (load_addr[file_size - 1] != '\r' && load_addr[file_size - 1] != '\n') {
		load_addr[file_size + 0] = '\r';
		load_addr[file_size + 1] = '\n';
	}

	UINT32 symbols_count = DEF_Library_Parse(load_addr, def_symbols);

	EP3_Memory_Free(load_addr);

	return symbols_count;
}
