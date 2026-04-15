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

/*
 * Fits on all platforms.
 */
#define AFW_AFW_RANGE_START            (10)
#define AFW_EV_NULL                    (AFW_AFW_RANGE_START + 1)
#define AFW_EV_APP_DONE                (AFW_AFW_RANGE_START + 2)
#define AFW_EV_SHUT_DOWN_ALL           (AFW_AFW_RANGE_START + 5)
#define AFW_EV_POWER_DELAY_REQ         (AFW_AFW_RANGE_START + 6)
#define AFW_EV_POWER_DN                (AFW_AFW_RANGE_START + 7)
#define AFW_EV_GAIN_FOCUS              (AFW_AFW_RANGE_START + 10)
#define AFW_EV_LOSE_FOCUS              (AFW_AFW_RANGE_START + 11)
#define AFW_EV_RENDER                  (AFW_AFW_RANGE_START + 15)
#define AFW_EV_GRANT_TOKEN             (AFW_AFW_RANGE_START + 20)
#define AFW_EV_REVOKE_TOKEN            (AFW_AFW_RANGE_START + 21)

#define AFW_GENERIC_BYTE_ARRAY_SIZE    (20)

#define AFW_SEQN_GENERIC               (-1)
#define AFW_ROUTE_ID_GENERIC           (36)
#define AFW_FRAMEWORK_EV_CATG          (0)
#define AFW_LOG_ID_GENERIC             (0)
#define AFW_CONSUMER_GENERIC           AFW_LOG_ID_GENERIC

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

enum tagAFW_SVC_EV_TYPE_T {
	AFW_NON_SVC_REQ = 0,
	AFW_REPORT_UNHANDLED_SVC_REQ,
	AFW_UNHANDLED_SVC_REQ_OCCURRED,
	AFW_SVC_REQ_NULL_EVG,
	AFW_SVC_REQ_NULL_EV,
	AFW_SERVICE_RESPONSE
};
typedef UINT8                          AFW_SVC_EV_TYPE_T;

typedef struct tagAFW_EVENT_T          AFW_EVENT_T;
typedef struct tagAFW_EV_DATA_APP_REG_T AFW_EV_DATA_APP_REG_T;

typedef SYN_RETURN_STATUS_T (* AFW_START_APP_FUNCTION_T)(
	AFW_EVENT_GROUP_T *                p_evg,
	AFW_APP_REGISTRY_ID_T              reg_id,
	const AFW_EV_DATA_APP_REG_T *      reg_hdl
);

struct tagAFW_EV_DATA_APP_REG_T {
	AFW_EVENT_CODE_T                   ev_code_subscribing_to;
	AFW_START_APP_FUNCTION_T           app_start_func;
	AFW_REG_DATA_T                     app_reg_handle;     /* new opaque handle */
	UINT32                             options;
	AFW_APP_REGISTRY_ID_T              app_reg_id;
	AFW_APP_CALL_API_T                 fntype;
};

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

struct tagAFW_EVENT_T {
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

struct tagAFW_EVENT_GROUP_T {
	AFW_EVENT_CODE_T                   ev_code;
	AFW_EVENT_CATG_T                   ev_catg;
	AFW_EVENT_TIME_T                   ev_last_upd_time;
	AFW_EVENT_T *                      active_list;
	AFW_EVENT_T *                      history_list;
	AFW_KEYPAD_FEEDBACK_TONES_T        key_tone;
	AFW_REQUEUE_FLAG_T                 requeue;
};

extern AFW_EVENT_T *AFW_GetEv(AFW_EVENT_GROUP_T *evg);

extern AFW_EVENT_CODE_T AFW_GetEvCode(AFW_EVENT_GROUP_T *evg);

extern AFW_AUF_SEQ_NUM_T AFW_GetEvSeqn(AFW_EVENT_GROUP_T *evg);

#define AFW_HAS_EVDATA(data)           (data != (AFW_EVENT_DATA_T *) SYN_NULL)
#define AFW_HAS_AXDATA(bsize, auxd)    ((bsize != 0) && (auxd != (AFW_EV_BUF_T *) SYN_NULL))

#define AFW_AddEv(evg, code, catg, log_id, data, bflag, bsize, auxd) \
	( \
		((!AFW_HAS_EVDATA(data)) && (!AFW_HAS_AXDATA(bsize, auxd))) ? \
			AFW_AddEvNoD(evg, code) : \
			(AFW_HAS_EVDATA(data) && (!AFW_HAS_AXDATA(bsize, auxd))) ? \
				AFW_AddEvEvD(evg, code, data) : \
				((!AFW_HAS_EVDATA(data)) &&  AFW_HAS_AXDATA(bsize, auxd)) ? \
					AFW_AddEvAux(evg, code, bflag, bsize, auxd) : \
					AFW_AddEvAuxD(evg, code, data, bflag, bsize, auxd) \
	)

#define AFW_AddEvNoD(evg, code) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC, \
		AFW_ROUTE_ID_GENERIC, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		SYN_NULL, \
		AFW_BUF_FLAG_READ_ONLY, \
		0, \
		SYN_NULL, \
		AFW_NON_SVC_REQ \
	)

#define AFW_AddEvEvD(evg, code, data) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC, \
		AFW_ROUTE_ID_GENERIC, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		data, \
		AFW_BUF_FLAG_READ_ONLY, \
		0, \
		SYN_NULL, \
		AFW_NON_SVC_REQ \
	)

#define AFW_AddEvAux(evg, code, bflag, bsize, auxd) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC, \
		AFW_ROUTE_ID_GENERIC, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		SYN_NULL, \
		bflag, \
		bsize, \
		auxd, \
		AFW_NON_SVC_REQ \
	)

#define AFW_AddEvAuxD(evg, code, data, bflag, bsize, auxd) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC, \
		AFW_ROUTE_ID_GENERIC, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		data, \
		bflag, \
		bsize, \
		auxd, \
		AFW_NON_SVC_REQ \
	)

#define AFW_AddSvcEv(evg, code, seqn, rtid, catg, log_id, data, bflag, bsize, auxd, svc_type) \
	( \
		((!AFW_HAS_EVDATA(data)) && (!AFW_HAS_AXDATA(bsize, auxd))) ? \
			AFW_AddSvcEvNoD(evg, code, svc_type, seqn, rtid) : \
			(AFW_HAS_EVDATA(data) && (!AFW_HAS_AXDATA(bsize, auxd))) ? \
				AFW_AddSvcEvEvD(evg, code, data, svc_type, seqn, rtid) : \
				((!AFW_HAS_EVDATA(data)) &&  AFW_HAS_AXDATA(bsize, auxd)) ? \
					AFW_AddSvcEvAux(evg, code, bflag, bsize, auxd, svc_type, seqn, rtid) : \
					AFW_AddSvcEvAuxD(evg, code, data, bflag, bsize, auxd, svc_type, seqn, rtid) \
	)

#define AFW_AddSvcEvNoD(evg, code, svc_type, seqn, rtid) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		seqn, \
		rtid, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		SYN_NULL, \
		AFW_BUF_FLAG_READ_ONLY, \
		0, \
		SYN_NULL, \
		svc_type, \
	)

#define AFW_AddSvcEvEvD(evg, code, data, svc_type, seqn, rtid) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		seqn, \
		rtid, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		data, \
		AFW_BUF_FLAG_READ_ONLY, \
		0, \
		SYN_NULL, \
		svc_type, \
	)

#define AFW_AddSvcEvAux(evg, code, bflag, bsize, auxd, svc_type, seqn, rtid) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		seqn, \
		rtid, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		SYN_NULL, \
		bflag, \
		bsize, \
		auxd, \
		svc_type, \
	)

#define AFW_AddSvcEvAuxD(evg, code, data, bflag, bsize, auxd, svc_type, seqn, rtid) \
	AFW_AddEvPriv( \
		evg, \
		code, \
		seqn, \
		rtid, \
		AFW_FRAMEWORK_EV_CATG, \
		SYN_FALSE, \
		SYN_TRUE, \
		AFW_LOG_ID_GENERIC, \
		AFW_CONSUMER_GENERIC, \
		data, \
		bflag, \
		bsize, \
		auxd, \
		svc_type, \
	)

extern SYN_RETURN_STATUS_T AFW_AddEvPriv(
	AFW_EVENT_GROUP_T *                evg,
	AFW_EVENT_CODE_T                   ev_code,
	AFW_AUF_SEQ_NUM_T                  ev_seqn,
	AFW_EV_ROUTE_ID_T                  ev_rtid,
	AFW_EVENT_CATG_T                   ev_catg,
	AFW_CONSUMED_FLAG_T                consumed,
	AFW_FIRST_PASS_FLAG_T              first_pass,
	AFW_DATA_LOG_APP_ID_T              log_id,
	AFW_DATA_LOG_APP_ID_T              ev_consumer,
	AFW_EVENT_DATA_T*                  ev_data,
	AFW_EV_BUF_FLAG_T                  ev_bflag,
	AFW_EV_BUF_SIZE_T                  ev_bsize,
	AFW_EV_BUF_T *                     ev_auxd,
	AFW_SVC_EV_TYPE_T                  svc_type
);

#define AFW_TranslateEv(evg, code, catg, log_id, data, bflag, bsize, auxd) \
	( \
		((!AFW_HAS_EVDATA(data)) && (!AFW_HAS_AXDATA(bsize, auxd))) ? \
			AFW_TranslateEvNoD(evg, code) : \
			(AFW_HAS_EVDATA(data) && (!AFW_HAS_AXDATA(bsize, auxd))) ? \
				AFW_TranslateEvEvD(evg, code, data) : \
				((!AFW_HAS_EVDATA(data)) &&  AFW_HAS_AXDATA(bsize, auxd)) ? \
					AFW_TranslateEvAux(evg, code, bflag, bsize, auxd) : \
					AFW_TranslateEvAuxD (evg, code, data, bflag, bsize, auxd) \
	)

extern SYN_RETURN_STATUS_T AFW_TranslateEvNoD(
	AFW_EVENT_GROUP_T *                evg,
	AFW_EVENT_CODE_T                   ev_code
);

extern SYN_RETURN_STATUS_T AFW_TranslateEvEvD(
	AFW_EVENT_GROUP_T *                evg,
	AFW_EVENT_CODE_T                   ev_code,
	AFW_EVENT_DATA_T *                 ev_data
);

extern SYN_RETURN_STATUS_T AFW_TranslateEvAux(
	AFW_EVENT_GROUP_T *                evg,
	AFW_EVENT_CODE_T                   ev_code,
	AFW_EV_BUF_FLAG_T                  ev_bflag,
	AFW_EV_BUF_SIZE_T                  ev_bsize,
	AFW_EV_BUF_T *                     ev_auxd
);

extern SYN_RETURN_STATUS_T AFW_TranslateEvAuxD(
	AFW_EVENT_GROUP_T *                evg,
	AFW_EVENT_CODE_T                   ev_code,
	AFW_EVENT_DATA_T *                 ev_data,
	AFW_EV_BUF_FLAG_T                  ev_bflag,
	AFW_EV_BUF_SIZE_T                  ev_bsize,
	AFW_EV_BUF_T *                     ev_auxd
);

#define AFW_CreateInternalQueuedEv(code, catg, log_id, data, bflag, bsize, auxd) \
	( \
		(AFW_HAS_EVDATA(data)) ? \
			AFW_CreateInternalQueuedEvAuxD (code, data, bflag, bsize, auxd) : \
			AFW_CreateInternalQueuedEvAux(code, bflag, bsize, auxd) \
	)

#define AFW_CreateInternalQueuedSvcEv(code, seqn, rtid, catg, log_id, data, bflag, bsize, auxd, svc) \
	( \
		(AFW_HAS_EVDATA(data)) ? \
			AFW_CreateInternalQueuedSvcEvAuxD(code, seqn, rtid, data, bflag, bsize, auxd, svc) : \
			AFW_CreateInternalQueuedSvcEvAux(code, seqn, rtid, bflag, bsize, auxd, svc) \
	)

#define AFW_CreateInternalQueuedEvAux(code, bflag, bsize, auxd) \
	AFW_CreateInternalQueuedEvPriv( \
		code, \
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC, \
		AFW_ROUTE_ID_GENERIC, \
		AFW_FRAMEWORK_EV_CATG, \
		AFW_LOG_ID_GENERIC, \
		SYN_NULL, \
		bflag, \
		bsize, \
		auxd, \
		AFW_NON_SVC_REQ \
	)

#define AFW_CreateInternalQueuedEvAuxD(code, data, bflag, bsize, auxd) \
	AFW_CreateInternalQueuedEvPriv( \
		code, \
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC, \
		AFW_ROUTE_ID_GENERIC, \
		AFW_FRAMEWORK_EV_CATG, \
		AFW_LOG_ID_GENERIC, \
		data, \
		bflag, \
		bsize, \
		auxd, \
		AFW_NON_SVC_REQ \
	)

#define AFW_CreateInternalQueuedSvcEvAux(code, seqn, rtid, bflag, bsize, auxd, svc) \
	AFW_CreateInternalQueuedSvcEvPriv( \
		code, \
		seqn, \
		rtid, \
		AFW_FRAMEWORK_EV_CATG, \
		AFW_LOG_ID_GENERIC, \
		SYN_NULL, \
		bflag, \
		bsize, \
		auxd, \
		svc \
	)

#define AFW_CreateInternalQueuedSvcEvAuxD(code, seqn, rtid, data, bflag, bsize, auxd, svc) \
	AFW_CreateInternalQueuedSvcEvPriv( \
		code, \
		seqn, \
		rtid, \
		AFW_FRAMEWORK_EV_CATG, \
		AFW_LOG_ID_GENERIC, \
		data, \
		bflag, \
		bsize, \
		auxd, \
		svc \
	)

extern SYN_RETURN_STATUS_T AFW_CreateInternalQueuedEvPriv(
	AFW_EVENT_CODE_T                   ev_code,
	AFW_AUF_SEQ_NUM_T                  ev_seqn,
	AFW_EV_ROUTE_ID_T                  ev_rtid,
	AFW_EVENT_CATG_T                   ev_catg,
	AFW_DATA_LOG_APP_ID_T              log_id,
	AFW_EVENT_DATA_T *                 ev_data,
	AFW_EV_BUF_FLAG_T                  ev_bflag,
	AFW_EV_BUF_SIZE_T                  ev_bsize,
	AFW_EV_BUF_T *                     ev_auxd,
	AFW_SVC_EV_TYPE_T                  svc_type
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_AFW_EVENT_H */
