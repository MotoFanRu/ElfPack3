#ifndef P2K_EP3_CONFIG_H
#define P2K_EP3_CONFIG_H

#include "P2K_SDK_Base.h"
#include "P2K_EP3_DEF_Library.h"
#include "P2K_EP3_Image_Manager.h"
#include "P2K_EP3_SO_Loader.h"
#include "P2K_EP3_Environment.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 31-Oct-2026: ep3.cfg loader. Line-based directive format, one directive per line,
//   full-line '#' or ';' comments and blank lines are skipped (no inline comments),
//   both LF and CR LF line endings are accepted:
//
//     # ElfPack settings
//     deflib ep3.def            # def library name/path
//     lib_capacity 1024         # def lib symbol capacity (~8 bytes per symbol)
//     max_elfs 8                # max concurrently loaded images: ELFs + BINs + .so
//                               #   libraries + ELF symbol providers (one shared registry)
//     env_max 8                 # shared environment variables: count limit,
//     env_name_max 16           #   name length limit and value length limit
//     env_value_max 32          #   (buffers include the NUL terminator)
//     libpath a/elf/lib/        # extra .so search dir (trailing slash optional, up to 3 lines)
//
//     # Autorun: autorun <path> [event] [args...]
//     autorun a/elf/my.elf 0x5999 hello world
//     autorun a/elf/launcher.elf
//
//   The event (hex) and the rest of the line (args) are optional. Unknown directives
//   are logged and skipped for forward compatibility.

// EXL, 31-Oct-2026: Load the per-image config "<image_path>.cfg" (see EP3_Image_Cfg_T
//   in P2K_EP3_Image_Manager.h). Missing file yields defaults (system allocator).
extern STATUS EP3_Image_Config_Load(const WCHAR *image_path, EP3_Image_Cfg_T *cfg);

// EXL, 01-Nov-2026: Simple option storage API on top of the ep3.cfg-style text parser
//   and a plain binary blob storage. Files are addressed as "<path>/<name>" (when
//   "path" is NULL or empty the "name" is used as-is for a flat file system; "name"
//   must not be NULL). Text options are stored one per line as "<param> <value>",
//   unknown lines and comments are preserved on rewrite.
//
//     EP3_API_CFG_TXT_SaveOption(L"/a/elf", L"my.cfg", "volume", EP3_CFG_TYPE_INT, &vol);
//     EP3_API_CFG_TXT_SaveOption(NULL, L"my.cfg", "nick", EP3_CFG_TYPE_STRING, "moto");
//     EP3_API_CFG_TXT_ReadOption(L"/a/elf", L"my.cfg", "volume", EP3_CFG_TYPE_INT, &vol);
//
//   Text notes: an INT is stored as a sign-aware decimal, a STRING is stored verbatim
//   and must not contain spaces/tabs/newlines (it is a single line token). "value_size"
//   limits the STRING copy on read (ignored for INT). The first "param" occurrence is
//   replaced, later duplicates are left untouched, a missing "param" is appended.
//
//   Binary notes: EP3_API_CFG_BIN_Save writes exactly "size" bytes (truncating any
//   previous content), EP3_API_CFG_BIN_Read requires the file to hold at least "size"
//   bytes and copies them into "data". Structured records need stable packing.
typedef enum tagEP3_CFG_Type {
	EP3_CFG_TYPE_INT = 0,                 // Sign-aware decimal (or 0xHEX) value, UINT32/INT32.
	EP3_CFG_TYPE_STRING                   // Whitespace-free string token.
} EP3_CFG_Type_T;

extern STATUS EP3_API_CFG_TXT_SaveOption(
	const WCHAR *path, const WCHAR *name, const char *param, EP3_CFG_Type_T type, const void *value
);
extern STATUS EP3_API_CFG_TXT_ReadOption(
	const WCHAR *path, const WCHAR *name, const char *param, EP3_CFG_Type_T type, void *value, UINT32 value_size
);
extern STATUS EP3_API_CFG_BIN_Save(const WCHAR *path, const WCHAR *name, const void *data, UINT32 size);
extern STATUS EP3_API_CFG_BIN_Read(const WCHAR *path, const WCHAR *name, void *data, UINT32 size);

#define EP3_CFG_MAX_PATH               (48)
#define EP3_CFG_MAX_ARGS               (64)
#define EP3_CFG_MAX_AUTORUN            (16)
#define EP3_CFG_MAX_LIBPATHS           (3)

#define EP3_CFG_DEFAULT_LIB_CAPACITY   (DEF_LIB_MAX_SYM_COUNT)
#define EP3_CFG_DEFAULT_MAX_ELFS       (EP3_IMG_DEFAULT_SLOTS)
#define EP3_CFG_DEFAULT_ENV_MAX        (8)
#define EP3_CFG_DEFAULT_ENV_NAME_MAX   (16)
#define EP3_CFG_DEFAULT_ENV_VALUE_MAX  (32)

typedef struct tagEP3_Autorun_Entry {
	char uri[EP3_CFG_MAX_PATH];           // ASCII path, converted to WCHAR at start time.
	UINT32 event;                         // Optional event number, 0 if not set.
	char args[EP3_CFG_MAX_ARGS];          // Optional args string, empty if not set.
} EP3_Autorun_Entry_T;

typedef struct tagEP3_Config {
	char deflib[EP3_CFG_MAX_PATH];        // Def library name or path for EP3_Find_Internal_System_Component().
	UINT32 lib_capacity;                  // Def lib symbol table capacity in symbols.
	UINT32 max_elfs;                      // Max concurrently loaded images (ELF + BIN + .so + providers).
	UINT32 env_max;                       // Shared environment variables: count, name and
	UINT32 env_name_max;                  //   value length limits (buffers include the NUL).
	UINT32 env_value_max;
	UINT32 libpath_cnt;                   // .so search paths (after the module own dir).
	char libpaths[EP3_CFG_MAX_LIBPATHS][EP3_CFG_MAX_PATH];
	UINT32 autorun_cnt;
	EP3_Autorun_Entry_T autorun[EP3_CFG_MAX_AUTORUN];
} EP3_Config_T;

// EXL, 31-Oct-2026: Fill the config with built-in defaults (no file access).
extern void EP3_Config_Defaults(EP3_Config_T *config);

// EXL, 31-Oct-2026: Load and parse ep3.cfg. Missing or broken file yields defaults, no autorun.
extern STATUS EP3_Config_Load(EP3_Config_T *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_CONFIG_H */
