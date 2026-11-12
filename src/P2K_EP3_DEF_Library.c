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
#include <P2K_EP3_Text.h>
#include <P2K_EP3_PFW_Version.h>

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

static STATUS Check_PFW(const char *p) {
	INT32 pfw_len = PORTABLE_strlen_line(p + DEF_LIB_PARSER_HEADER_VAL);
	if (pfw_len <= 0) {
		return RESULT_FAIL;
	}

	char pfw_str[DEF_LIB_MAX_SYM_NAME_LENGTH] = { ASCII_NULL };
	PORTABLE_strncpy(pfw_str, p + DEF_LIB_PARSER_HEADER_VAL, pfw_len);

	const char *p_sep = PORTABLE_strchr(pfw_str, '_');
	if (p_sep == NULL) {
		return RESULT_FAIL;
	}
	UINT32 separator_offset = (UINT32) (p_sep - pfw_str);
	if (separator_offset == 0) {
		return RESULT_FAIL;
	}
	pfw_str[separator_offset] = ASCII_NULL;

	const char *EP3_phone_str = EP3_API_Get_Phone();
	if (PORTABLE_strncmp(pfw_str, EP3_phone_str, separator_offset) != 0) {
		L("[EP3 DEF]: Wrong phone: '%s' != '%s'\n", pfw_str, EP3_phone_str);
		// return RESULT_FAIL;
	}

	const char *EP3_firmware_str = EP3_API_Get_Firmware();
	if (PORTABLE_strncmp(pfw_str + (separator_offset + 1), EP3_firmware_str, pfw_len - (separator_offset + 1)) != 0) {
		L("[EP3 DEF]: Wrong firmware: '%s' != '%s'\n", pfw_str + (separator_offset + 1), EP3_firmware_str);
		// return RESULT_FAIL;
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

	const char *p_sep = PORTABLE_strchr(ver_str, '_');
	if (p_sep == NULL) {
		return RESULT_FAIL;
	}
	UINT32 separator_offset = (UINT32) (p_sep - ver_str);
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
	if (*(p + DEF_LIB_PARSER_CONTENT_TYPE) == 'C') {
		symbols[i].hash = (symbols[i].hash & DEF_LIB_CONST_TAG_MASK) | DEF_LIB_CONST_TAG_PREFIX;
	}
	symbols[i].addr = EP3_Text_Hex_To_U32(p + DEF_LIB_PARSER_CONTENT_ADDR);
	if (*(p + DEF_LIB_PARSER_CONTENT_TYPE) == 'T') {
		symbols[i].addr = symbols[i].addr | 1;
	}

	D("Added: 0x%08X 0x%08X %s\n", symbols[i].hash, symbols[i].addr, sym_str);

	return RESULT_OK;
}

static void DEF_Library_Sort_Symbols(DEF_LIB_Symbol *def_lib, UINT32 def_cnt) {
	/* Using insertion sort. */
	for (UINT32 i = 1; i < def_cnt; ++i) {
		DEF_LIB_Symbol key = def_lib[i];
		UINT32 j = i;

		while (j > 0 && def_lib[j - 1].hash > key.hash) {
			def_lib[j] = def_lib[j - 1];
			--j;
		}

		def_lib[j] = key;
	}
}

static STATUS DEF_Library_Check_Hash_Collisions(const DEF_LIB_Symbol *def_lib, UINT32 def_cnt) {
	/* Check only the sorted library. */
	if (def_cnt < 2) {
		return RESULT_OK;
	}

	for (UINT32 i = 1; i < def_cnt; ++i) {
		if (def_lib[i].hash == def_lib[i - 1].hash) {
			L("[EP3 DEF]: Collision at %d: 0x%08X 0x%08X\n", i - 1, def_lib[i - 1].hash, def_lib[i - 1].addr);
			return RESULT_FAIL;
		}
	}

	return RESULT_OK;
}

static STATUS DEF_Library_Parse(
	const char *buffer,
	DEF_LIB_Symbol *symbols,
	UINT32 max_symbol_count,
	UINT32 *out_symbol_count,
	DEF_LIB_Symbol *const_symbols,
	UINT32 max_const_count,
	UINT32 *out_const_count
) {
	STATUS is_pfw_okay = RESULT_FAIL;
	STATUS is_ver_okay = RESULT_FAIL;
	STATUS is_cpu_okay = RESULT_FAIL;

	const char *p = buffer;
	UINT32 symbol_count = 0;
	UINT32 const_count = 0;

	while (*p) {
		if (Is_Line_PFW(p) == RESULT_OK) {
			is_pfw_okay = Check_PFW(p);
		} else if (Is_Line_VER(p) == RESULT_OK) {
			is_ver_okay = Check_VER(p);
		} else if (Is_Line_CPU(p) == RESULT_OK) {
			is_cpu_okay = Check_CPU(p);
		} else if (Is_Line_Content(p) == RESULT_OK) {
			if (*(p + DEF_LIB_PARSER_CONTENT_TYPE) == DEF_LIB_TYPE_CONST) {
				if (const_symbols != NULL) {
					if (const_count >= max_const_count) {
						L("[EP3 DEF]: Constants exceed capacity: %d\n", max_const_count);
						return RESULT_FAIL;
					}
					if (Check_Content(p, const_symbols, const_count) != RESULT_OK) {
						L("[EP3 DEF]: Bad constant content line at %d\n", const_count);
						return RESULT_FAIL;
					}
					const_count++;
				}
			} else if (symbols != NULL) {
				if (symbol_count >= max_symbol_count) {
					L("[EP3 DEF]: Symbols exceed capacity: %d\n", max_symbol_count);
					return RESULT_FAIL;
				}
				if (Check_Content(p, symbols, symbol_count) != RESULT_OK) {
					L("[EP3 DEF]: Bad symbol content line at %d\n", symbol_count);
					return RESULT_FAIL;
				}
				symbol_count++;
			}
		}

		EP3_Text_Skip_To_Next_Line(&p);
	}

	if (is_pfw_okay != RESULT_OK) {
		L("[EP3 DEF]: %s\n", "Cannot parse PFW from header.");
		return RESULT_FAIL;
	}
	if (is_ver_okay != RESULT_OK) {
		L("[EP3 DEF]: %s\n", "Cannot parse VER from header.");
		return RESULT_FAIL;
	}
	if (is_cpu_okay != RESULT_OK) {
		L("[EP3 DEF]: %s\n", "Cannot parse CPU from header.");
		return RESULT_FAIL;
	}

	if (symbols != NULL) {
		DEF_Library_Sort_Symbols(symbols, symbol_count);
		if (DEF_Library_Check_Hash_Collisions(symbols, symbol_count) != RESULT_OK) {
			return RESULT_FAIL;
		}
		*out_symbol_count = symbol_count;
		L("[EP3 DEF]: Library parsed successfully, symbols: %d\n", symbol_count);
	}
	if (const_symbols != NULL) {
		DEF_Library_Sort_Symbols(const_symbols, const_count);
		*out_const_count = const_count;
		L("[EP3 DEF]: Constants parsed successfully, entries: %d\n", const_count);
	}

	return RESULT_OK;
}

// EXL, 02-Nov-2026: Read a def-format system file to a heap buffer (caller frees).
static STATUS DEF_Library_Read_File(const char *file_name, char **out_buffer, UINT32 *out_size) {
	WCHAR file_path[PATH_MAX_SHORT];
	if (!EP3_Find_Internal_System_Component(file_name, file_path)) {
		L("[EP3 DEF]: Failed to find '%s' system component.\n", file_name);
		return RESULT_FAIL;
	}

	/* Use `DL_FsSGetFileSize()` instead `DL_FsGetFileSize()` to reduce the number of functions. */
	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(file_path, DL_FS_OWNER_RESERVED);
	if (file_size < DEF_LIB_TOO_SMALL) {
		L("[EP3 DEF]: File '%s' too small (%d bytes).\n", file_name, file_size);
		return RESULT_FAIL;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		L("[EP3 DEF]: Failed to open '%s' file.\n", file_name);
		return RESULT_FAIL;
	}

	/* Add additional 4 bytes to add line ending characters. */
	char *load_addr = (char *) EP3_Memory_Alloc(file_size + 4);
	if (load_addr == NULL) {
		L("[EP3 DEF]: Failed to allocate %d bytes of memory.\n", file_size + 4);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}

	memset(load_addr, 0, file_size + 4);

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(load_addr, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		L("[EP3 DEF]: Failed to read '%s' into memory at '0x%08X' address.\n", file_name, load_addr);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	if (elements_read != 1) {
		L("[EP3 DEF]: elements_read=%d, should be 1.\n", elements_read);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	L("[EP3 DEF]: Loaded %d bytes to '0x%08X' memory address.\n", file_size, load_addr);

	DL_FsCloseFile(file_handle);

	/* Add a proper line endings (if missed). */
	if (load_addr[file_size - 1] != '\r' && load_addr[file_size - 1] != '\n') {
		load_addr[file_size + 0] = '\r';
		load_addr[file_size + 1] = '\n';
	}

	*out_buffer = load_addr;
	*out_size = file_size;

	return RESULT_OK;
}

static int DEF_Library_Get_Index(const DEF_LIB_Symbol *def_lib, UINT32 def_cnt, UINT32 hash) {
	/* Binary search. */
	if (def_cnt == 0) {
		return -1;
	}

	int low = 0;
	int high = (int) def_cnt - 1;

	while (low <= high) {
		int mid = low + (high - low) / 2;

		if (def_lib[mid].hash == hash) {
			return mid;
		}

		if (def_lib[mid].hash < hash) {
			low = mid + 1;
		} else {
			high = mid - 1;
		}
	}

	return -1;
}

UINT32 EP3_DEF_Library_Get(const DEF_LIB_Symbol *def_lib, UINT32 def_cnt, UINT32 hash) {
	int idx = DEF_Library_Get_Index(def_lib, def_cnt, hash);
	if (idx >= 0) {
		return def_lib[idx].addr;
	}
	return 0;
}

STATUS EP3_DEF_Library_Insert(DEF_LIB_Symbol *def_lib, UINT32 *p_def_cnt, UINT32 hash, UINT32 addr) {
	UINT32 def_cnt = *p_def_cnt;
	if (def_cnt >= DEF_LIB_MAX_SYM_COUNT) {
		L("[EP3 DEF]: Library is full: %d\n", def_cnt);
		return RESULT_FAIL;
	}

	int idx = DEF_Library_Get_Index(def_lib, def_cnt, hash);
	if (idx >= 0) {
		L("[EP3 DEF]: Collision at %d: 0x%08X 0x%08X\n", idx, def_lib[idx].hash, def_lib[idx].addr);
		def_lib[idx].addr = addr;
		return RESULT_FAIL;
	}

	UINT32 pos = 0;
	while (pos < def_cnt && def_lib[pos].hash < hash) {
		++pos;
	}

	for (UINT32 i = def_cnt; i > pos; --i) {
		def_lib[i] = def_lib[i - 1];
	}

	def_lib[pos].hash = hash;
	def_lib[pos].addr = addr;

	L("[EP3 DEF]: Inserted %d: 0x%08X 0x%08X\n", pos, def_lib[pos].hash, def_lib[pos].addr);

	++def_cnt;
	*p_def_cnt = def_cnt;

	return RESULT_OK;
}

STATUS EP3_DEF_Library_Delete(DEF_LIB_Symbol *def_lib, UINT32 *p_def_cnt, UINT32 hash) {
	UINT32 def_cnt = *p_def_cnt;
	int idx = DEF_Library_Get_Index(def_lib, def_cnt, hash);
	if (idx < 0) {
		L("[EP3 DEF]: Not found in DEF LIB: 0x%08X\n", hash);
		return RESULT_FAIL;
	}

	L("[EP3 DEF]: Deleted: %d: 0x%08X 0x%08X\n", idx, def_lib[idx].hash, def_lib[idx].addr);

	for (UINT32 i = (UINT32) idx; i < def_cnt - 1; ++i) {
		def_lib[i] = def_lib[i + 1];
	}

	def_lib[def_cnt - 1].hash = 0x00000000UL;
	def_lib[def_cnt - 1].addr = 0x00000000UL;

	--def_cnt;
	*p_def_cnt = def_cnt;

	return RESULT_OK;
}

static void DEF_Library_Merge(
	DEF_LIB_Symbol *def_lib,
	UINT32 *p_def_cnt,
	const DEF_LIB_Symbol *fresh,
	UINT32 fresh_cnt
) {
	UINT32 def_cnt = *p_def_cnt;
	UINT32 inserted = 0;
	UINT32 updated = 0;
	UINT32 unchanged = 0;

	for (UINT32 i = 0; i < fresh_cnt; ++i) {
		int idx = DEF_Library_Get_Index(def_lib, def_cnt, fresh[i].hash);

		if (idx >= 0) {
			if (def_lib[idx].addr != fresh[i].addr) {
				L("[EP3 DEF]: Reload: updated 0x%08X: 0x%08X => 0x%08X\n", fresh[i].hash, def_lib[idx].addr, fresh[i].addr);
				def_lib[idx].addr = fresh[i].addr;
				updated++;
			} else {
				unchanged++;
			}
		} else if (EP3_DEF_Library_Insert(def_lib, &def_cnt, fresh[i].hash, fresh[i].addr) == RESULT_OK) {
			inserted++;
		}
	}

	*p_def_cnt = def_cnt;
	L("[EP3 DEF]: Reload done: +%d new, ~%d updated, =%d unchanged, total %d (removed are kept).\n",
		inserted, updated, unchanged, def_cnt
	);
}

STATUS EP3_DEF_Library_Reload(DEF_LIB_Symbol *def_lib, UINT32 *p_def_cnt, UINT32 max_count) {
	if ((def_lib == NULL) || (p_def_cnt == NULL) || (max_count == 0)) {
		L("[EP3 DEF]: %s\n", "Reload: invalid arguments.");
		return RESULT_FAIL;
	}

	// EXL, 31-Oct-2026: Parse the fresh ep3.def into a scratch table, then merge it
	//   into the live one. The live table is never wiped: runtime-registered entries
	//   (dlopen and friends) and already-resolved functions stay untouched.
	DEF_LIB_Symbol *fresh = (DEF_LIB_Symbol *) EP3_Memory_Alloc(sizeof(DEF_LIB_Symbol) * max_count);
	if (fresh == NULL) {
		L("[EP3 DEF]: Reload: failed to allocate %d bytes for the scratch table.\n", sizeof(DEF_LIB_Symbol) * max_count);
		return RESULT_FAIL;
	}

	UINT32 fresh_cnt = EP3_DEF_Library_Load(fresh, max_count);
	if (fresh_cnt == 0) {
		L("[EP3 DEF]: %s\n", "Reload: nothing to merge, the live table is kept as is.");
		EP3_Memory_Free(fresh);
		return RESULT_FAIL;
	}

	DEF_Library_Merge(def_lib, p_def_cnt, fresh, fresh_cnt);

	EP3_Memory_Free(fresh);

	return RESULT_OK;
}

// EXL, 02-Nov-2026: Constants ("C" lines) form a separate hash namespace, so a
//   constant and a function may share a name without collision, and the resolver
//   never confuses a raw value with a code address.

static STATUS DEF_Library_Load_Tables(
	DEF_LIB_Symbol *symbols,
	UINT32 max_symbol_count,
	UINT32 *out_symbol_count,
	DEF_LIB_Symbol *const_symbols,
	UINT32 max_const_count,
	UINT32 *out_const_count
) {
	if (((symbols != NULL) && (out_symbol_count == NULL || max_symbol_count == 0))
		|| ((const_symbols != NULL) && (out_const_count == NULL || max_const_count == 0))
		|| ((symbols == NULL) && (const_symbols == NULL))) {
		L("[EP3 DEF]: %s\n", "Load: invalid table arguments.");
		return RESULT_FAIL;
	}

	if (out_symbol_count != NULL) {
		*out_symbol_count = 0;
	}
	if (out_const_count != NULL) {
		*out_const_count = 0;
	}
	if (symbols != NULL) {
		memset(symbols, 0, sizeof(DEF_LIB_Symbol) * max_symbol_count);
	}
	if (const_symbols != NULL) {
		memset(const_symbols, 0, sizeof(DEF_LIB_Symbol) * max_const_count);
	}

	char *buffer = NULL;
	UINT32 buffer_size = 0;
	if (DEF_Library_Read_File(EP3_ELF_LDR_DEF_NAME, &buffer, &buffer_size) != RESULT_OK) {
		return RESULT_FAIL;
	}

	STATUS result = DEF_Library_Parse(
		buffer,
		symbols, max_symbol_count, out_symbol_count,
		const_symbols, max_const_count, out_const_count
	);
	EP3_Memory_Free(buffer);

	return result;
}

STATUS EP3_DEF_Library_Load_All(
	DEF_LIB_Symbol *def_symbols,
	DEF_LIB_Symbol *const_symbols,
	UINT32 max_count,
	UINT32 *out_symbol_count,
	UINT32 *out_const_count
) {
	if ((def_symbols == NULL) || (const_symbols == NULL)
		|| (out_symbol_count == NULL) || (out_const_count == NULL)) {
		L("[EP3 DEF]: %s\n", "Load_All: invalid arguments.");
		return RESULT_FAIL;
	}
	if (max_count == 0) {
		max_count = DEF_LIB_MAX_SYM_COUNT;
	}

	return DEF_Library_Load_Tables(
		def_symbols, max_count, out_symbol_count,
		const_symbols, max_count, out_const_count
	);
}

UINT32 EP3_DEF_Library_Load(DEF_LIB_Symbol *def_symbols, UINT32 max_count) {
	if (!def_symbols) {
		L("[EP3 DEF]: %s\n", "Pointer to DEF Library is NULL!\n");
		return 0;
	}
	if (max_count == 0) {
		max_count = DEF_LIB_MAX_SYM_COUNT;
	}

	UINT32 symbols_count = 0;
	if (DEF_Library_Load_Tables(def_symbols, max_count, &symbols_count, NULL, 0, NULL) != RESULT_OK) {
		return 0;
	}
	return symbols_count;
}

UINT32 EP3_DEF_Library_Load_Consts(DEF_LIB_Symbol *const_symbols, UINT32 max_count) {
	if (!const_symbols) {
		L("[EP3 DEF]: %s\n", "Pointer to the DEF constants is NULL!");
		return 0;
	}
	if (max_count == 0) {
		max_count = DEF_LIB_MAX_SYM_COUNT;
	}

	UINT32 const_cnt = 0;
	if (DEF_Library_Load_Tables(NULL, 0, NULL, const_symbols, max_count, &const_cnt) != RESULT_OK) {
		return 0;
	}
	return const_cnt;
}

STATUS EP3_DEF_Library_Reload_All(
	DEF_LIB_Symbol *def_lib,
	UINT32 *p_def_cnt,
	DEF_LIB_Symbol *const_symbols,
	UINT32 *p_const_cnt,
	UINT32 max_count
) {
	if ((def_lib == NULL) || (p_def_cnt == NULL) || (const_symbols == NULL)
		|| (p_const_cnt == NULL) || (max_count == 0)) {
		L("[EP3 DEF]: %s\n", "Reload_All: invalid arguments.");
		return RESULT_FAIL;
	}

	UINT32 scratch_size = sizeof(DEF_LIB_Symbol) * max_count * 2;
	DEF_LIB_Symbol *fresh = (DEF_LIB_Symbol *) EP3_Memory_Alloc(scratch_size);
	if (fresh == NULL) {
		L("[EP3 DEF]: Reload_All: failed to allocate %d bytes for scratch tables.\n", scratch_size);
		return RESULT_FAIL;
	}

	DEF_LIB_Symbol *fresh_consts = fresh + max_count;
	UINT32 fresh_cnt = 0;
	UINT32 fresh_const_cnt = 0;
	if (DEF_Library_Load_Tables(
		fresh, max_count, &fresh_cnt,
		fresh_consts, max_count, &fresh_const_cnt
	) != RESULT_OK) {
		EP3_Memory_Free(fresh);
		return RESULT_FAIL;
	}
	if (fresh_cnt == 0) {
		L("[EP3 DEF]: %s\n", "Reload_All: no symbols to merge, live tables are kept as is.");
		EP3_Memory_Free(fresh);
		return RESULT_FAIL;
	}

	DEF_Library_Merge(def_lib, p_def_cnt, fresh, fresh_cnt);
	memset(const_symbols, 0, sizeof(DEF_LIB_Symbol) * max_count);
	for (UINT32 i = 0; i < fresh_const_cnt; ++i) {
		const_symbols[i] = fresh_consts[i];
	}
	*p_const_cnt = fresh_const_cnt;

	EP3_Memory_Free(fresh);

	L("[EP3 DEF]: Constants reloaded: %d entries.\n", fresh_const_cnt);
	return RESULT_OK;
}

// EXL, 02-Nov-2026: Live const-table context for the fixup API (set by the loader).
static DEF_LIB_Symbol *EP3_Def_Consts = NULL;
static UINT32 EP3_Def_Const_Cnt = 0;

void EP3_DEF_Library_Set_Consts(DEF_LIB_Symbol *const_symbols, UINT32 const_cnt) {
	EP3_Def_Consts = const_symbols;
	EP3_Def_Const_Cnt = const_cnt;
}

STATUS EP3_DEF_Library_Get_Const_By_Hash(UINT32 hash, UINT32 *out_value) {
	if ((EP3_Def_Consts == NULL) || (out_value == NULL)) {
		return RESULT_FAIL;
	}

	hash = (hash & DEF_LIB_CONST_TAG_MASK) | DEF_LIB_CONST_TAG_PREFIX;
	int idx = DEF_Library_Get_Index(EP3_Def_Consts, EP3_Def_Const_Cnt, hash);
	if (idx < 0) {
		return RESULT_FAIL;
	}

	*out_value = EP3_Def_Consts[idx].addr;

	return RESULT_OK;
}

// EXL, 02-Nov-2026: Runtime constant resolution + static table fixup (Option A):
//   images bake DEF_LIB_CONST_TAG_PREFIX-tagged FNV hashes into their const tables
//   (the values are computed by the consts tool, since C99 cannot hash at compile
//   time); EP3_API_Const_Fix_Table rewrites the tagged words with real values.

STATUS EP3_API_Const_Get(const char *name, UINT32 *out_value) {
	if ((name == NULL) || (name[0] == ASCII_NULL) || (out_value == NULL)) {
		return RESULT_FAIL;
	}

	return EP3_DEF_Library_Get_Const_By_Hash(FNV1A32(name) & DEF_LIB_CONST_TAG_MASK, out_value);
}

STATUS EP3_API_Const_Fix_Table(void *mem, UINT32 size) {
	if ((mem == NULL) || (size < sizeof(UINT32))) {
		L("[EP3 DEF]: %s\n", "Const_Fix_Table: invalid arguments.");
		return RESULT_FAIL;
	}

	UINT32 *word = (UINT32 *) mem;
	UINT32 count = size / sizeof(UINT32);
	UINT32 fixed = 0;
	UINT32 missed = 0;

	for (UINT32 i = 0; i < count; ++i) {
		if ((word[i] & ~DEF_LIB_CONST_TAG_MASK) != DEF_LIB_CONST_TAG_PREFIX) {
			continue;
		}

		UINT32 value = 0;
		if (EP3_DEF_Library_Get_Const_By_Hash(word[i] & DEF_LIB_CONST_TAG_MASK, &value) == RESULT_OK) {
			D("[EP3 DEF]: Const 0x%08X => %d\n", word[i], value);
			word[i] = value;
			fixed++;
		} else {
			D("[EP3 DEF]: WARNING! Const 0x%08X not found in the def library.\n", word[i]);
			missed++;
		}
	}

	L("[EP3 DEF]: Const_Fix_Table 0x%08X (%d bytes): %d fixed, %d missed.\n", mem, size, fixed, missed);

	return (missed == 0) ? RESULT_OK : RESULT_FAIL;
}
