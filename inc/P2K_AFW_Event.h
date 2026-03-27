#ifndef P2K_AFW_EVENT_H
#define P2K_AFW_EVENT_H

#include "P2K_SDK_Base.h"
#include "P2K_AFW.h"
#include "P2K_DL.h"
#include "P2K_DL_Keypad.h"
#include "P2K_DL_Clock.h"
#include "P2K_UIS_Event.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AFW_GENERIC_BYTE_ARRAY_SIZE    (20)

typedef UINT32                         AFW_EVENT_CODE_T;
typedef UINT8                          AFW_EVENT_CATG_T;
typedef UINT8                          AFW_EVENT_TIME_T;
typedef UINT8                          AFW_REQUEUE_FLAG_T;
typedef UINT32                         AFW_AUF_SEQ_NUM_T;
typedef UINT8                          AFW_FIRST_PASS_FLAG_T;
typedef UINT32                         AFW_EV_ROUTE_ID_T;
typedef UINT8                          AFW_EV_BUF_FLAG_T;
typedef UINT16                         AFW_EV_BUF_SIZE_T;
typedef UINT8                          AFW_EV_BUF_T;

typedef AFW_EVENT_CODE_T               AFW_UNION_DATA_ID_T;
typedef UINT8                          AFW_EV_DATA_GENERIC_BYTE_ARR_T;
typedef UINT32                         AFW_REG_DATA_T;
typedef UINT32                         AFW_RESET_CAUSES_T;

typedef UINT8                          AUF_EV_DATA_KEY_T;
typedef UINT8                          AUF_EV_DATA_GENERIC_T;

enum tagAFW_TONE_CMD_T {
	AFW_NO_TONE_ACTION = 0,
	AFW_START_TONE,
	AFW_STOP_TONE
};
typedef UINT8                          AFW_TONE_CMD_T;

typedef struct tagAFW_EV_DATA_APP_REG_T {
	AFW_EVENT_CODE_T                   ev_code_subscribing_to;
	VOID *                             app_start_func;     /* `AFW_START_APP_FUNCTION_T` replaced by VOID * (complex) */
	AFW_REG_DATA_T                     app_reg_handle;     /* new opaque handle */
	UINT32                             options;
	AFW_APP_REGISTRY_ID_T              app_reg_id;
	AFW_APP_CALL_API_T                 fntype;
} AFW_EV_DATA_APP_REG_T;

typedef struct tagAFW_EV_DATA_RESET_T {
	AFW_RESET_CAUSES_T                 cause;
} AFW_EV_DATA_RESET_T;

typedef struct tagAFW_EV_DATA_TYPES_UNION_T {
	union {
		AFW_TOKEN_T                    token;
		AFW_EV_DATA_GENERIC_BYTE_ARR_T generic_short_data[AFW_GENERIC_BYTE_ARRAY_SIZE];
		AFW_EV_DATA_APP_REG_T          app_registration;
		AFW_EV_DATA_RESET_T            reset_cause;
		AFW_APP_STACK_T                add_to_stack;
		AFW_EVENT_CODE_T               ev_code;
	} AFW_EV_DATA_U;
} AFW_EV_DATA_TYPES_UNION_T;

typedef struct tagAUF_EV_DATA_TYPES_UNION_T {
	union {
		AUF_EV_DATA_KEY_T              key;
		AUF_EV_DATA_GENERIC_T          auf_ev_data;
		DL_CLK_TIMER_IFACE_DATA_T      timeout;
	} AUF_EV_DATA_U;
} AUF_EV_DATA_TYPES_UNION_T;

typedef struct tagAFW_EVENT_DATA_T {
	AFW_UNION_DATA_ID_T                udata_id;
	union {
		AFW_EV_DATA_TYPES_UNION_T      afw_data_union;
		AUF_EV_DATA_TYPES_UNION_T      auf_data_union;
		UIS_EV_DATA_TYPES_UNION_T      uis_data_union;
	} AFW_UNION_DATA_T;
} AFW_EVENT_DATA_T;

typedef struct tagAFW_EVENT_T          AFW_EVENT_T;
struct __attribute__((aligned(4))) tagAFW_EVENT_T {
	AFW_EVENT_CODE_T                   ev_code;
	AFW_AUF_SEQ_NUM_T                  ev_seqn;
	AFW_EV_ROUTE_ID_T                  ev_rtid;
	AFW_EVENT_CATG_T                   ev_catg;
	AFW_CONSUMED_FLAG_T                consumed;
	AFW_FIRST_PASS_FLAG_T              first_pass;
	AFW_EVENT_TIME_T                   ev_time;
	AFW_DATA_LOG_APP_ID_T              ev_creator;
	AFW_DATA_LOG_APP_ID_T              ev_consumer;
	AFW_EVENT_T *                      ev_list;
	AFW_EVENT_DATA_T                   ev_data;
	AFW_EV_BUF_FLAG_T                  ev_bflag;
	AFW_EV_BUF_SIZE_T                  ev_bsize;
	AFW_EV_BUF_T *                     ev_auxd;
	UINT8                              uis_handling_flag;
};

typedef struct tagAFW_KEYPAD_FEEDBACK_TONES_T {
	AFW_TONE_CMD_T                     key_cmd;
	AFW_TONE_CMD_T                     aud_cmd;
	DL_KEY_CODES_T                     key_tone;
	DL_AUDIO_TONE_TYPE_T               aud_tone;
} AFW_KEYPAD_FEEDBACK_TONES_T;

typedef struct tagAFW_EVENT_GROUP_T {
	AFW_EVENT_CODE_T                   ev_code;
	AFW_EVENT_CATG_T                   ev_catg;
	AFW_EVENT_TIME_T                   ev_last_upd_time;
	BYTE                               padding_a[2];
	AFW_EVENT_T *                      active_list;
	AFW_EVENT_T *                      history_list;
	AFW_KEYPAD_FEEDBACK_TONES_T        key_tone;
	AFW_REQUEUE_FLAG_T                 requeue;
	BYTE                               padding_b;
} AFW_EVENT_GROUP_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_AFW_EVENT_H */
