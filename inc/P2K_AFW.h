#ifndef P2K_AFW_H
#define P2K_AFW_H

#include "P2K_SDK_Base.h"
#include "P2K_Synergy.h"
#include "P2K_SUAPI.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef UINT16                         AFW_APP_INSTANCE_ID_T;
typedef UINT16                         AFW_APP_REGISTRY_ID_T;
typedef UINT32                         AFW_DATA_LOG_APP_ID_T;
typedef SU_PORT_HANDLE                 AFW_APP_PORT_HDL_T;
typedef UINT8                          AFW_STACK_ID_T;
typedef UINT8                          AFW_APP_CALL_API_T;
typedef UINT8                          AFW_SESSION_ID_T;
typedef UINT32                         AFW_APP_DATA_HDL_T;
typedef UINT32                         AFW_APP_PRIORITY_T;
typedef UINT32                         AFW_APP_COUNT_T;

enum tagAFW_TOKEN_MGR_T {
	AFW_TM_ID_DATALINE = 0,
	AFW_TM_ID_FAXLINE,
	AFW_TM_ID_MIC,
	AFW_TM_ID_RINGER,
	AFW_TM_ID_NETWORK1,
	AFW_TM_ID_NETWORK2,
	AFW_TM_ID_SPEAKER,
	AFW_TM_ID_USERINTER,
	AFW_TM_ID_VIBE,
	AFW_TM_ID_POWER_DOWN,
	AFW_TM_ID_SIGNALING,
	AFW_TM_ID_REMOTE_KEY_ROUTING
};
typedef UINT32                         AFW_TOKEN_MGR_T;

enum tagAFW_APP_RSTACK_TYPE_T {
	AFW_PREPROCESSING = 0,
	AFW_FOCUS,
	AFW_POSTPROCESSING,
	AFW_RSTACK_NUM
};
typedef UINT8                          AFW_APP_RSTACK_TYPE_T;

enum tagAFW_APP_RSTACK_POS_T {
	AFW_POSITION_TOP = 0,
	AFW_POSITION_P0,
	AFW_POSITION_P1,
	AFW_POSITION_P2,
	AFW_POSITION_P3,
	AFW_POSITION_P4,
	AFW_POSITION_P5,
	AFW_POSITION_BOTTOM
};
typedef UINT8                          AFW_APP_RSTACK_POS_T;

enum tagAFW_CONSUMED_FLAG_T {
	AFW_BUF_FLAG_CONSUME = 100,        /* this says you want the AFW to free it */
	AFW_BUF_FLAG_READ_ONLY,            /* this says it's yours and the AFW or apps are not to modify it */
	AFW_BUF_FLAG_INVALID
};
typedef UINT8                          AFW_CONSUMED_FLAG_T;

enum tagAFW_TKN_UNION_DATA_ID_T {
	TM_DEFAULT = 0,
	TM_REGISTER,
	TM_APP_CENTRIC
};
typedef UINT8                          AFW_TKN_UNION_DATA_ID_T;

enum tagAFW_APP_CENTRICNESS_T {
	AFW_APP_CENTRICITY_PRIMARY = 0,
	AFW_APP_CENTRICITY_SECONDARY,
	AFW_APP_CENTRICITY_NONE
};
typedef UINT8                          AFW_APP_CENTRICNESS_T;

typedef struct tagAFW_APP_STACK_T      AFW_APP_STACK_T;
typedef struct tagAFW_EVENT_GROUP_T    AFW_EVENT_GROUP_T;
typedef struct tagAPP_INSTANCE_DATA_T  APP_INSTANCE_DATA_T;

typedef struct tagAFW_TKN_DATA_DEFAULT_T {
	AFW_APP_INSTANCE_ID_T              app_id;
	AFW_APP_PRIORITY_T                 app_priority;
} AFW_TKN_DATA_DEFAULT_T;

typedef struct tagAFW_TKN_DATA_REGISTER_T {
	AFW_APP_COUNT_T                    acMaxShare;
	AFW_APP_COUNT_T                    acMaxWait;
} AFW_TKN_DATA_REGISTER_T;

typedef struct tagAFW_TKN_DATA_APP_CENTRICITY_T {
	AFW_APP_CENTRICNESS_T              app_centc;
	AFW_APP_INSTANCE_ID_T              app_id;
	AFW_APP_INSTANCE_ID_T              xfr_app_id;
	AFW_APP_PRIORITY_T                 app_priority;
} AFW_TKN_DATA_APP_CENTRICITY_T;

typedef union tagAFW_UNION_DATA_T {
	AFW_TKN_DATA_DEFAULT_T             def;
	AFW_TKN_DATA_REGISTER_T            tkn_reg;
	AFW_TKN_DATA_APP_CENTRICITY_T      app_centric;
} AFW_UNION_DATA_T;

typedef struct tagAFW_TOKEN_DATA_T {
	AFW_TKN_UNION_DATA_ID_T            token_par_id;
	AFW_UNION_DATA_T                   data_type;
} AFW_TOKEN_DATA_T;

typedef struct tagAFW_TOKEN_T {
	AFW_TOKEN_MGR_T                    tm;
	AFW_TOKEN_DATA_T                   tpars;
} AFW_TOKEN_T;

typedef void (* AFW_HANDLE_EVENT_FUNCTION_T)(
	AFW_EVENT_GROUP_T *                p_evg,
	APP_INSTANCE_DATA_T *              p_apd,
	AFW_APP_INSTANCE_ID_T              app_id,
	AFW_APP_REGISTRY_ID_T              reg_id
);

struct tagAFW_APP_STACK_T {
	AFW_STACK_ID_T                     stackId;
	UINT8                              create_order;
	AFW_HANDLE_EVENT_FUNCTION_T        hefn;
	AFW_APP_CALL_API_T                 fntype;
	AFW_APP_RSTACK_TYPE_T              process_class;
	AFW_APP_RSTACK_POS_T               pos;
	AFW_SESSION_ID_T                   session_id;
	AFW_APP_DATA_HDL_T                 data_hdl;
	AFW_APP_INSTANCE_ID_T              app_id;
	AFW_APP_REGISTRY_ID_T              reg_id;
	AFW_APP_STACK_T *                  prev;
	AFW_APP_STACK_T *                  next;
};

extern SYN_RETURN_STATUS_T AFW_InquireRoutingStackByRegId(AFW_APP_REGISTRY_ID_T reg_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_AFW_H */
