#ifndef P2K_UIS_EVENT_H
#define P2K_UIS_EVENT_H

#include "P2K_SDK_Base.h"
#include "P2K_SDK_Platform.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define UIS_EV_DIALOG_DONE             (0x201C)
#define UIS_EV_DONE                    (0x2020)

typedef union tagUIS_EV_DATA_TYPES_UNION_T {
	BYTE                               padding[P2K_UIS_EV_DATA_UNION_SIZE];
} UIS_EV_DATA_TYPES_UNION_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_UIS_EVENT_H */
