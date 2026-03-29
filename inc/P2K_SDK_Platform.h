#ifndef P2K_SDK_PLATFORM_H
#define P2K_SDK_PLATFORM_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(FTR_C330)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xAC)
	#define P2K_UIS_MAX_NUM_VALUES                         (10)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (20)
#elif defined(FTR_E1)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xFC)
	#define P2K_UIS_MAX_NUM_VALUES                         (20)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (30)
#else
	#error "Please determine P2K platform struct/union type sizes."
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_PLATFORM_H */
