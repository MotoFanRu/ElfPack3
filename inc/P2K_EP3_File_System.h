#ifndef P2K_EP3_BASE_FILE_SYSTEM_H
#define P2K_EP3_BASE_FILE_SYSTEM_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PATH_MAX_SHORT                                     (32)

extern BOOL EP3_Find_Internal_System_Component(const char *file_name, WCHAR *out_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_BASE_FILE_SYSTEM_H */
