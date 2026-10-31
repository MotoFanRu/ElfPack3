#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_UIS_Ustring.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_Hash_Portable.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_File_System.h>
#include <P2K_EP3_Image_Manager.h>
#include <P2K_EP3_SO_Loader.h>

// EXL, 31-Oct-2026: Shared (.so) library loader. Based on the plain-GCC approach of the
//   Siemens loader (sie_src/loader3): one flat image per module, SysV DT_HASH export
//   lookup, direct JUMP_SLOT/GOT slot patching, DT_NEEDED recursion with refcounting.
//   A library is freed only when its last user releases it.

static EP3_SO *EP3_SO_Table = NULL;
static UINT32 EP3_SO_Table_Size = 0;

static DEF_LIB_Symbol *EP3_SO_Def_Lib = NULL;
static UINT32 EP3_SO_Def_Cnt = 0;

static char EP3_SO_LibPaths[EP3_SO_MAX_LIBPATHS][EP3_SO_LIBPATH_LEN];
static UINT32 EP3_SO_LibPath_Cnt = 0;

static char EP3_SO_Last_Error[EP3_SO_ERR_LEN] = { ASCII_NULL };

static void SO_Set_Error(const char *message) {
	PORTABLE_strncpy(EP3_SO_Last_Error, message, EP3_SO_ERR_LEN - 1);
	EP3_SO_Last_Error[EP3_SO_ERR_LEN - 1] = ASCII_NULL;
}

// EXL, 31-Oct-2026: The freestanding SDK has no memcpy declaration, copy words manually.
static void SO_Word_Copy(UINT32 *dst, const UINT32 *src, UINT32 words) {
	for (UINT32 i = 0; i < words; ++i) {
		dst[i] = src[i];
	}
}

// EXL, 31-Oct-2026: Standard SysV ELF symbol hash.
static UINT32 elf_hash(const unsigned char *name) {
	UINT32 h = 0;
	UINT32 g;

	while (*name != 0) {
		h = (h << 4) + *(name++);
		g = h & 0xF0000000;
		if (g != 0) {
			h ^= g >> 24;
		}
		h &= ~g;
	}

	return h;
}

// EXL, 31-Oct-2026: Software 32-bit unsigned division, the freestanding build has no
//   libgcc helpers like __aeabi_uidivmod. Shift-subtract, 32 iterations max.
static UINT32 SO_UDiv(UINT32 num, UINT32 den) {
	UINT32 q = 0;
	UINT32 bit = 1;

	if (den == 0) {
		return num;
	}
	while (!(den & 0x80000000UL) && (den < num)) {
		den <<= 1;
		bit <<= 1;
	}
	while (bit != 0) {
		if (num >= den) {
			num -= den;
			q |= bit;
		}
		den >>= 1;
		bit >>= 1;
	}

	return q;
}

// EXL, 31-Oct-2026: SysV DT_HASH bucket/chain export lookup. Returns the absolute
//   address (Thumb functions carry the interworking bit in st_value) or 0.
static UINT32 SO_Find_Export(const EP3_SO *so, const char *sym_str) {
	if (so->hash_table == NULL || so->sym_table == NULL || so->str_table == NULL) {
		return 0;
	}

	UINT32 hash = elf_hash((const unsigned char *) sym_str);

	UINT32 nbucket = so->hash_table[0];
	UINT32 *bucket = &so->hash_table[2];
	UINT32 *chain = &so->hash_table[2 + nbucket];

	for (UINT32 sym_idx = bucket[hash - SO_UDiv(hash, nbucket) * nbucket]; sym_idx != SHN_UNDEF; sym_idx = chain[sym_idx]) {
		Elf32_Sym *sym = &so->sym_table[sym_idx];

		if (sym->st_value == 0 || sym->st_shndx == SHN_UNDEF) {
			continue;
		}
		if (PORTABLE_strncmp(&so->str_table[sym->st_name], sym_str, EP3_SO_NAME_LEN) != 0) {
			continue;
		}

		UINT8 sym_type = sym->st_info & 0xF;
		if ((sym_type != STT_NOTYPE) && (sym_type != STT_OBJECT) && (sym_type != STT_FUNC)) {
			continue;
		}

		UINT32 sym_addr = (UINT32) so->mem_base + sym->st_value - so->virtual_base;

		D("[EP3 SO]: Found export '%s' in '%s': 0x%08X\n", sym_str, so->name, sym_addr);

		return sym_addr;
	}

	return 0;
}

UINT32 EP3_Resolve_Symbol(const char *sym_str) {
	if (sym_str == NULL || sym_str[0] == ASCII_NULL) {
		return 0;
	}

	// EXL, 31-Oct-2026: 1. DEF library (firmware functions, FNV1A binary search).
	if (EP3_SO_Def_Lib != NULL && EP3_SO_Def_Cnt > 0) {
		UINT32 sym_addr = EP3_DEF_Library_Get(EP3_SO_Def_Lib, EP3_SO_Def_Cnt, FNV1A32(sym_str));
		if (sym_addr != 0) {
			D("[EP3 SO]: Found '%s' in DEF Lib: 0x%08X\n", sym_str, sym_addr);
			return sym_addr;
		}
	}

	// EXL, 31-Oct-2026: 2. Loaded .so libraries and exporting ELF providers.
	for (UINT32 i = 0; i < EP3_SO_Table_Size; ++i) {
		EP3_SO *so = &EP3_SO_Table[i];
		if (!so->active) {
			continue;
		}
		UINT32 sym_addr = SO_Find_Export(so, sym_str);
		if (sym_addr != 0) {
			return sym_addr;
		}
	}

	D("[EP3 SO]: WARNING! No '%s' symbol found!\n", sym_str);

	return 0;
}

void EP3_SO_Init(
	UINT32 max_so, DEF_LIB_Symbol *def_symbols, UINT32 def_cnt,
	char libpaths[][EP3_SO_LIBPATH_LEN], UINT32 libpath_cnt
) {
	if (max_so == 0) {
		max_so = EP3_SO_DEFAULT_MAX;
	}
	if (max_so > EP3_SO_MAX) {
		L("[EP3 SO]: SO count clamped to %d (requested %d).\n", EP3_SO_MAX, max_so);
		max_so = EP3_SO_MAX;
	}

	if (EP3_SO_Table != NULL) {
		EP3_Memory_Free(EP3_SO_Table);
		EP3_SO_Table = NULL;
	}

	EP3_SO_Table = (EP3_SO *) EP3_Memory_Alloc(sizeof(EP3_SO) * max_so);
	if (EP3_SO_Table == NULL) {
		L("[EP3 SO]: Failed to allocate %d bytes for %d slots.\n", sizeof(EP3_SO) * max_so, max_so);
		EP3_SO_Table_Size = 0;
		return;
	}
	memset(EP3_SO_Table, 0, sizeof(EP3_SO) * max_so);
	EP3_SO_Table_Size = max_so;

	EP3_SO_Def_Lib = def_symbols;
	EP3_SO_Def_Cnt = def_cnt;

	EP3_SO_LibPath_Cnt = 0;
	for (UINT32 i = 0; (i < libpath_cnt) && (i < EP3_SO_MAX_LIBPATHS); ++i) {
		char *dst = EP3_SO_LibPaths[EP3_SO_LibPath_Cnt];
		PORTABLE_strncpy(dst, libpaths[i], EP3_SO_LIBPATH_LEN - 2);

		// EXL, 31-Oct-2026: Ensure the trailing separator, SO_Build_Path concatenates.
		UINT32 len = (UINT32) PORTABLE_strlen(dst);
		if ((len > 0) && (dst[len - 1] != '/')) {
			dst[len++] = '/';
			dst[len] = ASCII_NULL;
		}
		EP3_SO_LibPath_Cnt++;
	}

	L("[EP3 SO]: Initialized with %d slots at 0x%08X, libpaths=%d.\n", EP3_SO_Table_Size, EP3_SO_Table, EP3_SO_LibPath_Cnt);
}

EP3_SO *EP3_SO_Find_By_Name(const char *name) {
	if (name == NULL || name[0] == ASCII_NULL) {
		return NULL;
	}

	for (UINT32 i = 0; i < EP3_SO_Table_Size; ++i) {
		EP3_SO *so = &EP3_SO_Table[i];
		if (so->active && (so->kind == EP3_SO_KIND_LIB) && (PORTABLE_strncmp(so->name, name, EP3_SO_NAME_LEN) == 0)) {
			return so;
		}
	}

	return NULL;
}

// EXL, 31-Oct-2026: Build "dir + name" when the name is not an absolute path.
static void SO_Build_Path(const char *dir, const char *name, char *out, UINT32 out_size) {
	UINT32 pos = 0;

	if ((dir[0] != ASCII_NULL) && (name[0] != '/')) {
		while ((dir[pos] != ASCII_NULL) && (pos < (out_size - 1))) {
			out[pos] = dir[pos];
			pos++;
		}
	}

	while ((*name != ASCII_NULL) && (pos < (out_size - 1))) {
		out[pos++] = *(name++);
	}

	out[pos] = ASCII_NULL;
}

// EXL, 31-Oct-2026: Copy the directory part of an ASCII path (with trailing separator).
static void SO_Dir_Of(const char *path, char *out, UINT32 out_size) {
	const char *last_sep = NULL;

	for (const char *c = path; *c != ASCII_NULL; ++c) {
		if (*c == '/') {
			last_sep = c;
		}
	}

	if (last_sep == NULL) {
		out[0] = ASCII_NULL;
		return;
	}

	UINT32 len = (UINT32) (last_sep - path) + 1;
	if (len >= out_size) {
		len = out_size - 1;
	}
	PORTABLE_strncpy(out, path, len);
	out[len] = ASCII_NULL;
}

// EXL, 31-Oct-2026: Extract DT_SONAME (or fall back to the file name) into the record.
static void SO_Fill_Name(EP3_SO *so, const EP3_DYN_Context *ctx, const char *file_name) {
	if (ctx->dyn_tags[DT_SONAME] != 0 && ctx->str_table != NULL) {
		const char *soname = &ctx->str_table[ctx->dyn_tags[DT_SONAME]];
		if (soname[0] != ASCII_NULL) {
			PORTABLE_strncpy(so->name, soname, EP3_SO_NAME_LEN - 1);
			so->name[EP3_SO_NAME_LEN - 1] = ASCII_NULL;
			return;
		}
	}

	PORTABLE_strncpy(so->name, file_name, EP3_SO_NAME_LEN - 1);
	so->name[EP3_SO_NAME_LEN - 1] = ASCII_NULL;
}

// EXL, 31-Oct-2026: Load (or dedup) one .so by name. "dir" is the directory of the
//   loading module — the first search location, then the configured libpaths.
EP3_SO *EP3_SO_Load(const char *name, const char *dir) {
	if ((name == NULL) || (name[0] == ASCII_NULL)) {
		SO_Set_Error("Invalid library name.");
		return NULL;
	}
	if (EP3_SO_Table == NULL) {
		SO_Set_Error("SO subsystem is not initialized.");
		L("[EP3 SO]: %s\n", "SO subsystem is not initialized.");
		return NULL;
	}

	// EXL, 31-Oct-2026: Already loaded? Just bump the refcount.
	EP3_SO *existing = EP3_SO_Find_By_Name(name);
	if (existing != NULL) {
		existing->refs++;
		D("[EP3 SO]: Reusing '%s', refs=%d.\n", existing->name, existing->refs);
		return existing;
	}

	SO_Set_Error("");

	// EXL, 31-Oct-2026: Find a free record slot.
	EP3_SO *so = NULL;
	for (UINT32 i = 0; i < EP3_SO_Table_Size; ++i) {
		if (!EP3_SO_Table[i].active) {
			so = &EP3_SO_Table[i];
			break;
		}
	}
	if (so == NULL) {
		SO_Set_Error("Too many loaded libraries.");
		L("[EP3 SO]: Too many loaded .so libraries, max %d.\n", EP3_SO_Table_Size);
		return NULL;
	}

	// EXL, 31-Oct-2026: Build the search candidates: if the name carries its own
	//   directory part, it is used as-is; otherwise module dir first, then libpaths.
	char path[EP3_SO_LIBPATH_LEN];
	char name_dir[EP3_SO_LIBPATH_LEN];
	WCHAR wpath[EP3_SO_LIBPATH_LEN];

	SO_Dir_Of(name, name_dir, EP3_SO_LIBPATH_LEN);

	char candidates[1 + 1 + EP3_SO_MAX_LIBPATHS][EP3_SO_LIBPATH_LEN];
	UINT32 cand_cnt = 0;

	if (name_dir[0] != ASCII_NULL) {
		PORTABLE_strncpy(candidates[cand_cnt++], name, EP3_SO_LIBPATH_LEN - 1);
	} else {
		SO_Build_Path(dir, name, candidates[cand_cnt++], EP3_SO_LIBPATH_LEN);
		for (UINT32 i = 0; i < EP3_SO_LibPath_Cnt; ++i) {
			SO_Build_Path(EP3_SO_LibPaths[i], name, candidates[cand_cnt++], EP3_SO_LIBPATH_LEN);
		}
	}

	// EXL, 31-Oct-2026: The file name without any directory part (for the record name).
	const char *name_only = name;
	for (const char *c = name; *c != ASCII_NULL; ++c) {
		if (*c == '/') {
			name_only = c + 1;
		}
	}

	DL_FS_HANDLE_T file_handle = DL_FS_HANDLE_INVALID;
	for (UINT32 i = 0; i < cand_cnt; ++i) {
		PORTABLE_u_atou(candidates[i], wpath);
		DL_FS_SIZE_T file_size = DL_FsSGetFileSize(wpath, DL_FS_OWNER_RESERVED);
		if (file_size < FILE_SIZE_TOO_SMALL) {
			continue;
		}
		file_handle = DL_FsOpenFile(wpath, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
		if (file_handle != DL_FS_HANDLE_INVALID) {
			PORTABLE_strncpy(path, candidates[i], EP3_SO_LIBPATH_LEN - 1);
			path[EP3_SO_LIBPATH_LEN - 1] = ASCII_NULL;
			break;
		}
	}
	if (file_handle == DL_FS_HANDLE_INVALID) {
		SO_Set_Error("Library file not found.");
		L("[EP3 SO]: Failed to find '%s' in %d search paths.\n", name, cand_cnt);
		return NULL;
	}

	Elf32_Ehdr elf_header;
	if (EP3_ELF_Read_Header(file_handle, &elf_header) != RESULT_OK) {
		SO_Set_Error("Not a valid ELF file.");
		DL_FsCloseFile(file_handle);
		return NULL;
	}
	if (elf_header.e_type != ET_DYN) {
		SO_Set_Error("Library is not ET_DYN.");
		L("[EP3 SO]: '%s' has wrong e_type=%d, should be ET_DYN.\n", name, elf_header.e_type);
		DL_FsCloseFile(file_handle);
		return NULL;
	}
	if (EP3_ELF_Flavor(file_handle, &elf_header) != RESULT_OK) {
		SO_Set_Error("Unsupported ELF flavor.");
		DL_FsCloseFile(file_handle);
		return NULL;
	}

	// EXL, 31-Oct-2026: The library own directory is the search root for its DT_NEEDED.
	char lib_dir[EP3_SO_LIBPATH_LEN];
	SO_Dir_Of(path, lib_dir, EP3_SO_LIBPATH_LEN);

	// EXL, 31-Oct-2026: Two-pass scheme, pass 1: deploy the image and parse the tags.
	EP3_DYN_Context dyn_ctx;
	if (EP3_ELF_DYN_Load(file_handle, &elf_header, &dyn_ctx) != RESULT_OK) {
		SO_Set_Error("Failed to load library image.");
		DL_FsCloseFile(file_handle);
		return NULL;
	}

	DL_FsCloseFile(file_handle);

	// EXL, 31-Oct-2026: Fill the record and activate it BEFORE the link pass: the
	//   DT_NEEDED libraries may import this library exports (circular ELF <-> .so),
	//   export addresses are valid right after deployment.
	so->kind = EP3_SO_KIND_LIB;
	so->mem_base = dyn_ctx.mem_base;
	so->mem_size = dyn_ctx.mem_size;
	so->virtual_base = dyn_ctx.virtual_base;
	SO_Word_Copy(so->dyn_tags, dyn_ctx.dyn_tags, EP3_DYN_TAGS_COUNT);
	so->sym_table = dyn_ctx.sym_table;
	so->str_table = dyn_ctx.str_table;
	so->hash_table = (dyn_ctx.dyn_tags[DT_HASH] != 0) ?
		(Elf32_Word *) (dyn_ctx.phys_base + dyn_ctx.dyn_tags[DT_HASH] - dyn_ctx.virtual_base) : NULL;
	so->needed_cnt = 0;
	so->refs = 1;
	SO_Fill_Name(so, &dyn_ctx, name_only);
	so->active = TRUE;

	if (so->hash_table == NULL) {
		D("[EP3 SO]: WARNING! '%s' has no DT_HASH, it cannot export symbols.\n", so->name);
	}

	// EXL, 31-Oct-2026: Pass 2: load DT_NEEDED dependencies, then relocate.
	EP3_ELF_DYN_Link(&dyn_ctx, lib_dir);

	// EXL, 31-Oct-2026: Keep the loaded DT_NEEDED references for the release path.
	for (UINT32 i = 0; i < dyn_ctx.needed_so_cnt; ++i) {
		if (so->needed_cnt < EP3_SO_MAX_NEEDED) {
			so->needed[so->needed_cnt++] = dyn_ctx.needed_so[i];
		} else {
			D("[EP3 SO]: Too many '%s' DT_NEEDED dependencies to attach.\n", so->name);
			EP3_SO_Release(dyn_ctx.needed_so[i]);
		}
	}

	// EXL, 31-Oct-2026: Run the library constructors AFTER relocation, so they can
	//   use the resolved imports.
	EP3_ELF_Run_Init(&dyn_ctx);

	// EXL, 31-Oct-2026: Track the library in the unified image manager.
	PORTABLE_u_atou(path, wpath);
	EP3_Image *image = EP3_Image_Register(
		EP3_IMG_SO, dyn_ctx.mem_base, dyn_ctx.mem_size, wpath, 0, EP3_IMG_FLAG_FREE_MEM
	);
	if (image == NULL) {
		EP3_ELF_Run_Fini(&dyn_ctx);
		for (UINT32 i = 0; i < so->needed_cnt; ++i) {
			EP3_SO_Release(so->needed[i]);
		}
		EP3_Memory_Free(dyn_ctx.mem_base);
		memset(so, 0, sizeof(EP3_SO));
		SO_Set_Error("Image registry is full.");
		return NULL;
	}

	L("[EP3 SO]: Loaded '%s' at 0x%08X (%d bytes), refs=%d.\n", so->name, so->mem_base, so->mem_size, so->refs);

	return so;
}

STATUS EP3_SO_Release(EP3_SO *so) {
	if (so == NULL) {
		SO_Set_Error("Invalid library handle.");
		return RESULT_FAIL;
	}
	if (!so->active || (so->kind != EP3_SO_KIND_LIB)) {
		SO_Set_Error("Invalid library handle.");
		return RESULT_FAIL;
	}

	if (so->refs > 1) {
		so->refs--;
		D("[EP3 SO]: Released '%s', refs=%d.\n", so->name, so->refs);
		return RESULT_OK;
	}

	// EXL, 31-Oct-2026: Last user is gone — full teardown.
	L("[EP3 SO]: Unloading '%s' at 0x%08X (%d bytes).\n", so->name, so->mem_base, so->mem_size);

	EP3_DYN_Context dyn_ctx;
	memset(&dyn_ctx, 0, sizeof(dyn_ctx));
	dyn_ctx.phys_base = (Elf32_Addr) so->mem_base;
	dyn_ctx.virtual_base = so->virtual_base;
	SO_Word_Copy(dyn_ctx.dyn_tags, so->dyn_tags, EP3_DYN_TAGS_COUNT);
	EP3_ELF_Run_Fini(&dyn_ctx);

	STATUS status = RESULT_OK;

	// EXL, 31-Oct-2026: Release the DT_NEEDED dependencies of this library.
	for (UINT32 i = 0; i < so->needed_cnt; ++i) {
		if (EP3_SO_Release(so->needed[i]) != RESULT_OK) {
			status = RESULT_FAIL;
		}
	}

	// EXL, 31-Oct-2026: Free the image via the image manager (frees mem + uri).
	EP3_Image *image = EP3_Image_Find(so->mem_base);
	if (image != NULL) {
		if (EP3_Image_Unregister(image) != RESULT_OK) {
			status = RESULT_FAIL;
		}
	} else {
		EP3_Memory_Free(so->mem_base);
	}

	memset(so, 0, sizeof(EP3_SO));

	return status;
}

EP3_SO *EP3_SO_Register_ELF(EP3_DYN_Context *ctx) {
	if ((ctx == NULL) || (ctx->mem_base == NULL)) {
		return NULL;
	}

	EP3_SO *so = NULL;
	for (UINT32 i = 0; i < EP3_SO_Table_Size; ++i) {
		if (!EP3_SO_Table[i].active) {
			so = &EP3_SO_Table[i];
			break;
		}
	}
	if (so == NULL) {
		D("[EP3 SO]: %s\n", "No free slot for the ELF symbol provider.");
		return NULL;
	}

	so->kind = EP3_SO_KIND_ELF_PROVIDER;
	so->mem_base = ctx->mem_base;
	so->mem_size = ctx->mem_size;
	so->virtual_base = ctx->virtual_base;
	SO_Word_Copy(so->dyn_tags, ctx->dyn_tags, EP3_DYN_TAGS_COUNT);
	so->sym_table = ctx->sym_table;
	so->str_table = ctx->str_table;
	so->hash_table = (ctx->dyn_tags[DT_HASH] != 0) ?
		(Elf32_Word *) (ctx->phys_base + ctx->dyn_tags[DT_HASH] - ctx->virtual_base) : NULL;
	so->needed_cnt = 0;                 // Dependencies are attached after EP3_ELF_DYN_Link.
	so->refs = 0;                       // ELF providers are not refcounted here.
	PORTABLE_strncpy(so->name, "elf:provider", EP3_SO_NAME_LEN - 1);
	so->active = TRUE;

	L("[EP3 SO]: ELF at 0x%08X registered as symbol provider.\n", so->mem_base);

	return so;
}

void EP3_SO_Attach_Needed(EP3_SO *so, EP3_DYN_Context *ctx) {
	if ((so == NULL) || (so->kind != EP3_SO_KIND_ELF_PROVIDER)) {
		return;
	}

	for (UINT32 i = 0; i < ctx->needed_so_cnt; ++i) {
		if (so->needed_cnt < EP3_SO_MAX_NEEDED) {
			so->needed[so->needed_cnt++] = ctx->needed_so[i];
		} else {
			D("[EP3 SO]: %s\n", "Too many ELF DT_NEEDED dependencies to attach.");
			EP3_SO_Release(ctx->needed_so[i]);
		}
	}
}

STATUS EP3_SO_Unregister_ELF(void *elf_mem_base) {
	if (elf_mem_base == NULL) {
		return RESULT_FAIL;
	}

	for (UINT32 i = 0; i < EP3_SO_Table_Size; ++i) {
		EP3_SO *so = &EP3_SO_Table[i];
		if (so->active && (so->kind == EP3_SO_KIND_ELF_PROVIDER) && (so->mem_base == elf_mem_base)) {
			// EXL, 31-Oct-2026: Release the .so dependencies the ELF holds.
			for (UINT32 j = 0; j < so->needed_cnt; ++j) {
				EP3_SO_Release(so->needed[j]);
			}
			memset(so, 0, sizeof(EP3_SO));
			D("[EP3 SO]: ELF provider at 0x%08X unregistered.\n", elf_mem_base);
			return RESULT_OK;
		}
	}

	return RESULT_FAIL;
}

STATUS EP3_SO_Process_Needed(EP3_DYN_Context *ctx, const char *dir) {
	for (UINT32 i = 0; i < ctx->needed_cnt; ++i) {
		const char *lib_name = &ctx->str_table[ctx->needed_off[i]];

		EP3_SO *so = EP3_SO_Load(lib_name, dir);
		if (so == NULL) {
			L("[EP3 SO]: DT_NEEDED '%s' failed: %s\n", lib_name, EP3_dlerror());
			continue;   // Non-fatal: unresolved imports produce warnings later.
		}

		if (ctx->needed_so_cnt < EP3_SO_MAX_NEEDED) {
			ctx->needed_so[ctx->needed_so_cnt++] = so;
		} else {
			D("[EP3 SO]: Too many loaded DT_NEEDED dependencies, max %d.\n", EP3_SO_MAX_NEEDED);
			EP3_SO_Release(so);
		}
	}

	return (ctx->needed_so_cnt == ctx->needed_cnt) ? RESULT_OK : RESULT_FAIL;
}

// EXL, 31-Oct-2026: dl* API. Handles are EP3_SO records registered into the DEF library
//   table at boot, so ELFs call them like any firmware function.

void *EP3_dlopen(const char *name) {
	EP3_SO_Last_Error[0] = ASCII_NULL;

	return EP3_SO_Load(name, "");
}

void *EP3_dlsym(void *handle, const char *name) {
	EP3_SO_Last_Error[0] = ASCII_NULL;

	EP3_SO *so = (EP3_SO *) handle;
	if ((so == NULL) || !so->active) {
		SO_Set_Error("Invalid dlsym handle.");
		return NULL;
	}

	UINT32 sym_addr = SO_Find_Export(so, name);
	if (sym_addr == 0) {
		SO_Set_Error("Symbol not found.");
		return NULL;
	}

	return (void *) sym_addr;
}

STATUS EP3_dlclose(void *handle) {
	EP3_SO_Last_Error[0] = ASCII_NULL;

	return EP3_SO_Release((EP3_SO *) handle);
}

const char *EP3_dlerror(void) {
	return (EP3_SO_Last_Error[0] != ASCII_NULL) ? EP3_SO_Last_Error : NULL;
}
