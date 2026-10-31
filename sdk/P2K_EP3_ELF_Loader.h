#ifndef P2K_EP3_ELF_LOADER_H
#define P2K_EP3_ELF_LOADER_H

#include "P2K_SDK_Base.h"
#include "P2K_EP3_DEF_Library.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 30-Oct-2026: ELF entry point, "start" stub which calls "Register" ELF entry point.
typedef UINT32 (*EP3_ELF_Entry_T)(const char *file_uri, char *params, UINT32 reserve);

// EXL, 30-Oct-2026: Load GCC (ET_DYN) ELF, resolve symbols via the DEF library (ep3.def)
//   and loaded .so providers, apply relocations, register the image and start it from "e_entry".
//   "event" is passed through to the ELF entry ("reserve" slot).
//   On success *out_elf receives a handle (image base) usable with EP3_ELF_Unload().
extern STATUS EP3_ELF_Load(const WCHAR *file_path, const char *params, UINT32 event, void **out_elf);

// EXL, 30-Oct-2026: Unload previously loaded ELF, freeing its image memory.
extern STATUS EP3_ELF_Unload(void *elf_ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_ELF_LOADER_H */
