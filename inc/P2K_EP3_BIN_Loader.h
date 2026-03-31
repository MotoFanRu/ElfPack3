#ifndef P2K_EP3_BIN_LOADER_H
#define P2K_EP3_BIN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FILE_SIZE_TOO_SMALL            (32)
#define BINLOADER_FUNC_NAME            "ep3/bin"

typedef STATUS (*EP3_ELF_LDR_ENTRY_POINT_T)(const UINTPTR *args);

/*
 * This function address will be added to `app_pra_registration_table[]` table instead of `APP_CALC_MainRegister()` or
 * some other registration functions.
 *
 * There should be a call to `APP_CALC_MainRegister()` inside this function so that the
 * calculator application can also register itself in the system.
 */
extern void EP3_BIN_Loader_MainRegister(void);

/*
 * Since the calculator is the most common stock P2K/Synergy application,
 * we can use an injection into its registration function address and hook it back.
 *
 * The registration function can be any function in the `app_pra_registration_table[]` table,
 * but its name should be `APP_CALC_MainRegister()` for compatibility purposes.
 */
extern void APP_CALC_MainRegister(void);

typedef STATUS (*EP3_BIN_LOAD_T)(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL set_bit, BOOL free_it);
extern STATUS EP3_API_BIN_Load(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL set_bit, BOOL free_it);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_BIN_LOADER_H */
