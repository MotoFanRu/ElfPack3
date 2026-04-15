#ifndef P2K_SDK_PLATFORM_H
#define P2K_SDK_PLATFORM_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * P2K_UIS_EV_DATA_UNION_SIZE
 *
 * padding is sizeof(AFW_EVENT_T) - 0x2C (44 bytes)
 *
 * Check AFW_NewEv(), AFW_NewEvGrp() functions.
 *
 * P2K_UIS_MAX_NUM_VALUES
 *
 * Check APP_PHBK_MainCommonNoticeEnter() function.
 */


#if defined(FTR_C330)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xAC)
	#define P2K_UIS_MAX_NUM_VALUES                         (10)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (20)
#endif /* FTR_C330 */

#if defined(FTR_E1)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xFC)
	#define P2K_UIS_MAX_NUM_VALUES                         (20)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (30)
#endif /* FTR_E1 */

#if defined(FTR_A830)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xAC)
	#define P2K_UIS_MAX_NUM_VALUES                         (10)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (20)
#endif /* FTR_E1 */

#if \
	!defined(P2K_UIS_EV_DATA_UNION_SIZE) || \
	!defined(P2K_UIS_MAX_NUM_VALUES) || \
	!defined(P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS)
		#error "Please determine P2K platform struct/union type sizes."
#endif /* All Platform Defines. */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_PLATFORM_H */
