#ifndef P2K_EP3_BIN_LOADER_H
#define P2K_EP3_BIN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FILE_SIZE_TOO_SMALL                                (32)

typedef void (*EP3_ELF_LDR_ENTRY_POINT_T) (void);

/*
 * Since the calculator is the most common stock P2K/Synergy application,
 * we can use an injection into its registration function address and hook it back.
 *
 * The registration function can be any function in the `app_pra_registration_table[]` table,
 * but its name should be `APP_CALC_MainRegister()` for compatibility purposes.
 */
extern void APP_CALC_MainRegister(void);

/*
 * This function address will be added to `app_pra_registration_table[]` table instead of `APP_CALC_MainRegister()` or
 * some other registration functions.
 *
 * There should be a call to `APP_CALC_MainRegister()` inside this function so that the
 * calculator application can also register itself in the system.
 */
extern void EP3_BIN_Loader_MainRegister(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_BIN_LOADER_H */
