#ifndef P2K_EP3_BIN_LOADER_H
#define P2K_EP3_BIN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BINLOADER_FUNC_NAME            "ep3/bin"

#include "P2K_EP3_Image_Args.h"

// EXL, 02-Nov-2026: The BIN entry is "EP3_Main" with the unified EP3_ImageArgs_T
//   (argv is NULL for BINs: the legacy "args" pointer array has no text form).
typedef UINT32 (* EP3_ELF_LDR_ENTRY_POINT_T)(EP3_ImageArgs_T *args);

typedef STATUS (* EP3_BIN_LOAD_T)(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL a_bit, BOOL free_it);
extern STATUS EP3_API_BIN_Load(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL a_bit, BOOL free_it);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_BIN_LOADER_H */
