#ifndef P2K_SDK_CONSTANTS_H
#define P2K_SDK_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// WARNING! This file is maintained by const.py external tool,
// do not add constants manually!

// This file exists because some firmware event codes differ between platforms and
// firmwares, while ELFs must stay firmware-independent: the values below are NOT
// real codes but 0xC0-tagged FNV1A32 hashes of the constant names (computed by the
// const.py tool). At load time the ELF Loader resolves them from the ep3.def
// constants ("0xVALUE C NAME" lines) — see sdk/P2K_EP3_DEF_Library.h.

// If you want to add some constant, just place it there with (0x00000000) value.
// And run const.py tool to generate a proper hash for it.
// #define UIS_MY_SUPER_CONST             (0x00000000)
//
// Usage in an ELF:
//   static const APP_EV_HANDLER_ENTRY_T tbl[] = {
//       { UIS_MY_SUPER_CONST, MyHandler },   // 0xC0-tagged placeholder
//   };
//   UINT32 EP3_Main(EP3_ImageArgs_T *args) {
//       EP3_API_Init_Event_Handlers_Tbl(tbl, args->event);   // resolves the tags
//       APP_Register(...);
//       return RESULT_OK;
//   }

#define UIS_MY_SUPER_CONST             (0xC11A604B)
#define STRANGE_CONST                  (0xCD231BC3)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_CONSTANTS_H */
