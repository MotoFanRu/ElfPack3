#ifndef P2K_UIS_EVENT_H
#define P2K_UIS_EVENT_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* padding is sizeof(AFW_EVENT_T) - 0x2C (44 bytes) */
typedef union tagUIS_EV_DATA_TYPES_UNION_T {
#if defined(FTR_C330)
	BYTE                               padding[0xAC];
#elif defined(FTR_E1)
	BYTE                               padding[0xFC];
#else
	#error                            "Please determine padding for UIS_EV_DATA_TYPES_UNION_T union type."
#endif
} UIS_EV_DATA_TYPES_UNION_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_UIS_EVENT_H */
