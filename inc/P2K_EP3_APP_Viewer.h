#ifndef P2K_EP3_APP_VIEWER_H
#define P2K_EP3_APP_VIEWER_H

#include "P2K_AFW_Event.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_VIEWER_STARTUP_EVENT       ((AFW_EVENT_CODE_T) 0xAAA0)

extern void EP3_APP_Viewer_Register(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_APP_VIEWER_H */
