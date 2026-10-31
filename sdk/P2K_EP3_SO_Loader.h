#ifndef P2K_EP3_SO_LOADER_H
#define P2K_EP3_SO_LOADER_H

#include "P2K_SDK_Base.h"
#include "P2K_EP3_ELF.h"
#include "P2K_EP3_DEF_Library.h"
#include "P2K_EP3_ELF_Loader.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 31-Oct-2026: Shared (.so) library loader. Plain GCC "-fPIC -shared" images loaded
//   into the flat address space, SysV DT_HASH exports, refcounted DT_NEEDED dependencies
//   and a dlopen/dlsym/dlclose/dlerror API. Exporting ELFs (built with --export-dynamic)
//   can also act as symbol providers, so a .so can call back into the ELF that uses it.
//
//   Symbol resolution order for any module import:
//     1. DEF library (ep3.def, firmware functions, FNV1A binary search).
//     2. Loaded .so libraries (registration order) and ELF providers.
//     3. Fallback: the module image base (loader workaround for unresolved symbols).

#define EP3_SO_MAX                     (16)
#define EP3_SO_DEFAULT_MAX             (8)
#define EP3_SO_NAME_LEN                (32)
#define EP3_SO_MAX_NEEDED              (8)
#define EP3_SO_MAX_LIBPATHS            (3)
#define EP3_SO_LIBPATH_LEN             (48)
#define EP3_SO_ERR_LEN                 (64)

// EXL, 31-Oct-2026: Dynamic tags cache size, covers DT_INIT/FINI arrays (up to 28).
#define EP3_DYN_TAGS_COUNT             (DT_FINI_ARRAYSZ + 1)

typedef enum tagEP3_SO_Kind {
	EP3_SO_KIND_LIB = 0,                  // Real .so image, refcounted.
	EP3_SO_KIND_ELF_PROVIDER              // Exporting ELF provider (no refcount of its own).
} EP3_SO_Kind_T;

typedef struct tagEP3_SO {
	EP3_SO_Kind_T kind;
	BOOL active;
	char name[EP3_SO_NAME_LEN];           // DT_SONAME or file name.
	void *mem_base;                       // Physical base of loaded image.
	UINT32 mem_size;
	Elf32_Addr virtual_base;
	Elf32_Word dyn_tags[EP3_DYN_TAGS_COUNT];
	Elf32_Sym *sym_table;                 // Exports (inside own image).
	char *str_table;
	Elf32_Word *hash_table;               // SysV DT_HASH (NULL if absent).
	struct tagEP3_SO *needed[EP3_SO_MAX_NEEDED];
	UINT32 needed_cnt;
	UINT32 refs;                          // Users of a EP3_SO_KIND_LIB.
} EP3_SO;

// EXL, 31-Oct-2026: Shared DYN core (PT_LOAD deploy + dynamic tags + relocations),
//   used by both ELF and .so loading in P2K_EP3_ELF_Loader.c.
typedef struct tagEP3_DYN_Context {
	void *mem_base;                       // Allocated image base.
	UINT32 mem_size;
	Elf32_Addr virtual_base;
	Elf32_Addr phys_base;
	Elf32_Word dyn_tags[EP3_DYN_TAGS_COUNT];
	Elf32_Word needed_off[EP3_SO_MAX_NEEDED];  // DT_NEEDED string offsets (into strtab).
	UINT32 needed_cnt;
	EP3_SO *needed_so[EP3_SO_MAX_NEEDED];      // Loaded DT_NEEDED dependencies (refs held).
	UINT32 needed_so_cnt;
	Elf32_Sym *sym_table;
	char *str_table;
} EP3_DYN_Context;

extern STATUS EP3_ELF_Read_Header(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header);
extern STATUS EP3_ELF_Flavor(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header);

// EXL, 31-Oct-2026: First pass of the DYN core: deploy an ET_DYN image and parse the
//   dynamic tags. Register the module as a symbol provider BEFORE the second pass!
extern STATUS EP3_ELF_DYN_Load(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header, EP3_DYN_Context *ctx);

// EXL, 31-Oct-2026: Second pass of the DYN core: load DT_NEEDED dependencies (recursive)
//   and apply all relocations (REL + JMPREL/PLT). "dir" is the ASCII directory of the
//   loading module ("" if unknown), the first search location for DT_NEEDED .so files.
extern STATUS EP3_ELF_DYN_Link(EP3_DYN_Context *ctx, const char *dir);

// EXL, 31-Oct-2026: Init/fini helpers for DT_INIT + DT_INIT_ARRAY / DT_FINI_ARRAY + DT_FINI.
extern void EP3_ELF_Run_Init(const EP3_DYN_Context *ctx);
extern void EP3_ELF_Run_Fini(const EP3_DYN_Context *ctx);

// EXL, 31-Oct-2026: Resolve a symbol by name: DEF library, then .so registry + ELF providers.
//   Returns 0 when not found. Thumb functions carry the interworking bit in the address.
extern UINT32 EP3_Resolve_Symbol(const char *sym_str);

// EXL, 31-Oct-2026: SO subsystem lifecycle and registry. "def_symbols/def_cnt" feed the
//   symbol resolver; "libpaths" are extra .so search dirs (after the loading module dir).
extern void EP3_SO_Init(
	UINT32 max_so, DEF_LIB_Symbol *def_symbols, UINT32 def_cnt,
	char libpaths[][EP3_SO_LIBPATH_LEN], UINT32 libpath_cnt
);
extern EP3_SO *EP3_SO_Load(const char *name, const char *dir);   // Loads or dedups (refs++).
extern STATUS EP3_SO_Release(EP3_SO *so);                        // refs--; frees at zero.
extern EP3_SO *EP3_SO_Find_By_Name(const char *name);

// EXL, 31-Oct-2026: Resolve ctx->needed_off[] (DT_NEEDED) to loaded, refcounted .so images.
//   Called from the DYN core after dynamic tag parsing, before relocations.
extern STATUS EP3_SO_Process_Needed(EP3_DYN_Context *ctx, const char *dir);

// EXL, 31-Oct-2026: ELF provider registration (a PIE ELF with DT_HASH exports).
//   Must be called AFTER EP3_ELF_DYN_Load and BEFORE EP3_ELF_DYN_Link, so the DT_NEEDED
//   libraries can resolve the ELF exports during their own relocation. Returns the
//   provider record; attach the loaded dependencies after EP3_ELF_DYN_Link.
extern EP3_SO *EP3_SO_Register_ELF(EP3_DYN_Context *ctx);
extern void EP3_SO_Attach_Needed(EP3_SO *so, EP3_DYN_Context *ctx);
extern STATUS EP3_SO_Unregister_ELF(void *elf_mem_base);

// EXL, 31-Oct-2026: dl API, registered into the DEF library table at boot.
extern void *EP3_dlopen(const char *name);
extern void *EP3_dlsym(void *handle, const char *name);
extern STATUS EP3_dlclose(void *handle);
extern const char *EP3_dlerror(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_SO_LOADER_H */
