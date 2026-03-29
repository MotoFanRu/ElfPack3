#ifndef P2K_UIS_H
#define P2K_UIS_H

#include "P2K_SDK_Base.h"
#include "P2K_Synergy.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef UINT32                         UIS_DIALOG_HANDLE_T;

extern SYN_RETURN_STATUS_T UIS_Delete(UIS_DIALOG_HANDLE_T handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_UIS_H */
