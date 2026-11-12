#ifndef P2K_EP3_DEF_LIBRARY_H
#define P2K_EP3_DEF_LIBRARY_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DEF_LIB_TOO_SMALL              (32)

#define DEF_LIB_MAX_SYM_NAME_LENGTH    (80)
#define DEF_LIB_MAX_SYM_COUNT          (1024)

#define DEF_LIB_PARSER_HEADER_VAL      (6)
#define DEF_LIB_PARSER_HEADER_CPU_NONE "NONE"
#define DEF_LIB_PARSER_CONTENT_ADDR    (2)
#define DEF_LIB_PARSER_CONTENT_TYPE    (11)
#define DEF_LIB_PARSER_CONTENT_NAME    (13)

#define DEF_LIB_EP3_VERSION_SLUG       "EP3"

// EXL, 02-Nov-2026: Constant namespace. The def file carries constants as
//   "0xVALUE C NAME" lines; they are parsed into a separate hash table so a constant
//   and a function may share a name without collision.
#define DEF_LIB_TYPE_CONST             'C'

// EXL, 02-Nov-2026: Compile-time placeholder tag for constants inside static tables
//   (the C99 preprocessor cannot hash strings, so the consts tool emits these):
//
//     #define UIS_EV_DONE  0xC0000000 | (FNV1A32("UIS_EV_DONE") & 0x0FFFFFFF)
//
//   (the tool computes FNV1A32 — the same hash the loader uses, see
//   P2K_Hash_Portable.h: basis 0x811C9DC5, prime 0x01000193, per-byte multiply-xor.
//   The tag prefix fixes the top nibble at 0xC, so tags span 0xC0000000..0xCFFFFFFF
//   and 28 hash bits are kept — zero extra cost on ARM7TDMI-S. The consts tool
//   must fail on 28-bit collisions between emitted constants.)
//   At runtime the image calls EP3_API_Const_Fix_Table() or
//   EP3_API_Init_Event_Handlers_Tbl() once per table containing tagged words; the
//   loader rewrites them with the real def-lib values.
#define DEF_LIB_CONST_TAG_PREFIX       (0xC0000000UL)
#define DEF_LIB_CONST_TAG_MASK         (0x0FFFFFFFUL)

typedef struct tagDEF_LIB_Symbol {
	UINT32 hash;
	UINT32 addr;
} DEF_LIB_Symbol;

extern RAW_DATA UINT32 EP3_phone;
extern RAW_DATA UINT32 EP3_firmware;
extern RAW_DATA UINT32 EP3_version;

extern UINT32 EP3_DEF_Library_Load(DEF_LIB_Symbol *def_symbols, UINT32 max_count);
// EXL, 13-Nov-2026: Read and parse both namespaces in a single file pass.
extern STATUS EP3_DEF_Library_Load_All(
	DEF_LIB_Symbol *def_symbols,
	DEF_LIB_Symbol *const_symbols,
	UINT32 max_count,
	UINT32 *out_symbol_count,
	UINT32 *out_const_count
);

// EXL, 31-Oct-2026: Hot reload: re-read ep3.def from the file system and MERGE it into
//   the live table — new symbols are inserted, changed addresses/types are applied in
//   place, removed symbols are KEPT (the table may contain runtime-registered entries
//   like dlopen and functions already resolved by loaded modules). Non-destructive:
//   a failed re-read leaves the table untouched.
extern STATUS EP3_DEF_Library_Reload(DEF_LIB_Symbol *def_lib, UINT32 *p_def_cnt, UINT32 max_count);
// EXL, 13-Nov-2026: Reload both namespaces from one read; constants are replaced.
extern STATUS EP3_DEF_Library_Reload_All(
	DEF_LIB_Symbol *def_lib,
	UINT32 *p_def_cnt,
	DEF_LIB_Symbol *const_symbols,
	UINT32 *p_const_cnt,
	UINT32 max_count
);

// EXL, 01-Nov-2026: Zero-argument hot reload of the live DEF library, callable from
//   any ELF (registered into the DEF library table by the loader, resolved like a
//   firmware function). Uses the loader global DEF library context.
extern void EP3_API_Def_Reload(void);

extern UINT32 EP3_DEF_Library_Get(const DEF_LIB_Symbol *def_lib, UINT32 def_cnt, UINT32 hash);
extern STATUS EP3_DEF_Library_Insert(DEF_LIB_Symbol *def_lib, UINT32 *p_def_cnt, UINT32 hash, UINT32 addr);
extern STATUS EP3_DEF_Library_Delete(DEF_LIB_Symbol *def_lib, UINT32 *p_def_cnt, UINT32 hash);

// EXL, 02-Nov-2026: Constants namespace ("C" def lines, separate hash table).
extern UINT32 EP3_DEF_Library_Load_Consts(DEF_LIB_Symbol *const_symbols, UINT32 max_count);
extern void EP3_DEF_Library_Set_Consts(DEF_LIB_Symbol *const_symbols, UINT32 const_cnt);
extern STATUS EP3_DEF_Library_Get_Const_By_Hash(UINT32 hash, UINT32 *out_value);

// EXL, 02-Nov-2026: Fix up DEF_LIB_CONST_TAG_PREFIX-tagged words inside a 4-byte
//   aligned memory range (e.g. a static event handler table): every tagged word is
//   replaced with the real value from the def constants namespace. Unresolvable
//   tags are left untouched and counted in the log.
extern STATUS EP3_API_Const_Fix_Table(void *mem, UINT32 size);

// EXL, 02-Nov-2026: Resolve one constant by name at runtime (FNV1A32(name) & mask).
extern STATUS EP3_API_Const_Get(const char *name, UINT32 *out_value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_DEF_LIBRARY_H */
