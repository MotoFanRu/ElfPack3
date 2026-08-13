#ifndef P2K_EP3_BIN_LOADER_H
#define P2K_EP3_BIN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BINLOADER_FUNC_NAME            "ep3/bin"

typedef STATUS (* EP3_ELF_LDR_ENTRY_POINT_T)(const UINTPTR *args);

typedef STATUS (* EP3_BIN_LOAD_T)(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL a_bit, BOOL free_it);
extern STATUS EP3_API_BIN_Load(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL a_bit, BOOL free_it);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_BIN_LOADER_H */
