#ifndef P2K_EP3_CONFIG_H
#define P2K_EP3_CONFIG_H

#include "P2K_SDK_Base.h"
#include "P2K_EP3_DEF_Library.h"
#include "P2K_EP3_Image_Manager.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 31-Oct-2026: ep3.cfg loader. Line-based directive format, one directive per line,
//   full-line '#' or ';' comments and blank lines are skipped (no inline comments),
//   both LF and CR LF line endings are accepted:
//
//     # ElfPack settings
//     deflib ep3.def
//     lib_capacity 1024
//     max_elfs 8
//
//     # Autorun: autorun <path> [event] [args...]
//     autorun a/elf/my.elf 0x5999 hello world
//     autorun a/elf/launcher.elf
//
//   The event (hex) and the rest of the line (args) are optional. Unknown directives
//   are logged and skipped for forward compatibility.

#define EP3_CFG_MAX_PATH               (48)
#define EP3_CFG_MAX_ARGS               (64)
#define EP3_CFG_MAX_AUTORUN            (16)

#define EP3_CFG_DEFAULT_LIB_CAPACITY   (DEF_LIB_MAX_SYM_COUNT)
#define EP3_CFG_DEFAULT_MAX_ELFS       (EP3_IMG_DEFAULT_SLOTS)

typedef struct tagEP3_Autorun_Entry {
	char uri[EP3_CFG_MAX_PATH];           // ASCII path, converted to WCHAR at start time.
	UINT32 event;                         // Optional event number, 0 if not set.
	char args[EP3_CFG_MAX_ARGS];          // Optional args string, empty if not set.
} EP3_Autorun_Entry_T;

typedef struct tagEP3_Config {
	char deflib[EP3_CFG_MAX_PATH];        // Def library name or path for EP3_Find_Internal_System_Component().
	UINT32 lib_capacity;                  // Def lib symbol table capacity in symbols.
	UINT32 max_elfs;                      // Max concurrently loaded images.
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
