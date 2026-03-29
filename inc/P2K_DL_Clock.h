#ifndef P2K_DL_CLOCK_H
#define P2K_DL_CLOCK_H

#include "P2K_SDK_Base.h"
#include "P2K_SUAPI.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef SU_TIME                        DL_CLK_TIMER_PERIOD_T;

typedef struct tagDL_CLK_TIMER_IFACE_DATA_T {
	DL_CLK_TIMER_PERIOD_T              timer_period;
	UINT32                             timer_data;
} DL_CLK_TIMER_IFACE_DATA_T;

typedef struct tagDL_CLK_DATE_T {
	UINT8                              day;
	UINT8                              month;
	UINT8                              year;
} DL_CLK_DATE_T;

typedef struct tagDL_CLK_TIME_T {
	UINT8                              hour;
	UINT8                              minute;
	UINT8                              seconds;
} DL_CLK_TIME_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_DL_CLOCK_H */
