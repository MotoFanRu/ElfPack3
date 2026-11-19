#ifndef P2K_SDK_PLATFORM_H
#define P2K_SDK_PLATFORM_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * P2K_UIS_EV_DATA_UNION_SIZE
 * padding is sizeof(AFW_EVENT_T) - 0x2C (44 bytes)
 * Check AFW_NewEv(), AFW_NewEvGrp() functions.
 * New: AFW_AddEvPriv() logic!
 *
 * P2K_UIS_MAX_NUM_VALUES
 * 40 + 4 + (8 * (10 or 20))
 * 124 == 10
 * 204 == 20
 * Check APP_PHBK_MainCommonNoticeEnter() function.
 *
 * P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS
 * UIS_ACTION_LIST_T
 * struct UIS_ACTION_LIST_T {
 *     UINT8 num_actions;
 *     // padding byte
 *     // padding byte
 *     // padding byte
 *     UIS_ACTION_LIST_ELEMENT_T action[20];
 * };
 * 1 + 1 + 1 + 1 + (12 * (20 or 30))
 * 244 == 20
 * 364 == 30
 * EnterSubsidyLockLockedState, MyTelViewEnter, atc_al_mback_mselect
 */

#if defined(FTR_V60)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0x1C)
	#define P2K_UIS_MAX_NUM_VALUES                         (10)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (20)
#endif /* FTR_V60 */

#if defined(FTR_V60I)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0x30)
	#define P2K_UIS_MAX_NUM_VALUES                         (10)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (20)
#endif /* FTR_V60I */

#if defined(FTR_C330)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xAC)
	#define P2K_UIS_MAX_NUM_VALUES                         (10)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (20)
#endif /* FTR_C330 */

#if defined(FTR_C350L)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xAC)
	#define P2K_UIS_MAX_NUM_VALUES                         (10)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (20)
#endif /* FTR_C350L */

#if defined(FTR_C650)
	#define P2K_UIS_EV_DATA_UNION_SIZE                     (0xFC)
	#define P2K_UIS_MAX_NUM_VALUES                         (20)
	#define P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS              (30)
#endif /* FTR_C650 */

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
	!defined(P2K_UIS_EV_DATA_UNION_SIZE)                   || \
	!defined(P2K_UIS_MAX_NUM_VALUES)                       || \
	!defined(P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS)
		#error "Please determine P2K platform struct/union type sizes."
#endif /* All Platform Defines. */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_PLATFORM_H */
