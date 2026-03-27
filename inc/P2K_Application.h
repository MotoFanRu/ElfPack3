#ifndef P2K_APPLICATION_H
#define P2K_APPLICATION_H

#include "P2K_SDK_Base.h"
#include "P2K_Synergy.h"
#include "P2K_AFW.h"
#include "P2K_AFW_Event.h"
#include "P2K_DL_DB_Shortcut.h"
#include "P2K_UIS.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// TODO: Can I use it for handlers instead of void*?
//typedef struct tagAPP_INSTANCE_DATA_T  APP_INSTANCE_DATA_T;

#define APP_NAME_SIZE                  (12)
#define APP_STATE_NAME_SIZE            (12)

typedef const char *                   APP_NAME_T;
typedef const char *                   APP_STATE_NAME_T;

typedef UINT8                          APP_STATE_T;
typedef UINT8                          APP_HISTORY_DATA_T;

enum tagAPP_TOKEN_STATUS_T {
	APP_TOKEN_NONE = 0,
	APP_TOKEN_REQUESTED,
	APP_TOKEN_GRANTED,
	APP_TOKEN_REVOKED
};
typedef UINT8                          APP_TOKEN_STATUS_T;

enum tagAPP_STATE_ENTRY_ACTION_T {
	APP_STATE_ENTER = 0,
	APP_STATE_RESUME
};
typedef UINT8                          APP_STATE_ENTRY_ACTION_T;

enum tagAPP_STATE_EXIT_ACTION_T {
	APP_STATE_EXIT = 0,
	APP_STATE_SUSPEND
};
typedef UINT8                          APP_STATE_EXIT_ACTION_T;

typedef SYN_RETURN_STATUS_T APP_HANDLER_FUNC_T(
	AFW_EVENT_GROUP_T *                p_evg,
	void *                             p_apd
);

typedef SYN_RETURN_STATUS_T APP_STATE_ENTRY_FUNC_T(
	AFW_EVENT_GROUP_T *                p_evg,
	void *                             p_apd,
	APP_STATE_ENTRY_ACTION_T           action
);

typedef SYN_RETURN_STATUS_T APP_STATE_EXIT_FUNC_T(
	AFW_EVENT_GROUP_T *                p_evg,
	void *                             p_apd,
	APP_STATE_EXIT_ACTION_T            action
);

typedef struct tagAPP_EV_HANDLER_ENTRY_T {
	AFW_EVENT_CODE_T                   ev_code;
	APP_HANDLER_FUNC_T *               p_ev_func;
} APP_EV_HANDLER_ENTRY_T;

typedef struct tagAPP_STATE_TRANSIT_T {
	APP_STATE_T                        state;
	APP_STATE_ENTRY_FUNC_T *           entry_function;
	APP_STATE_EXIT_FUNC_T *            exit_function;
	const APP_EV_HANDLER_ENTRY_T *     ev_handler;
} APP_STATE_TRANSIT_T;

typedef struct tagAPP_STATE_NAME_TABLE_T {
	APP_STATE_T                        state;
	char                               state_name[APP_STATE_NAME_SIZE + 1];
} APP_STATE_NAME_TABLE_T;

/* size = 0x50, 80 bytes was used on all platforms */
typedef struct tagAPP_INSTANCE_DATA_T {
	AFW_TOKEN_T                        userinter_token;
	AFW_DATA_LOG_APP_ID_T              log_id;
	AFW_EV_ROUTE_ID_T                  route_id;
	UIS_DIALOG_HANDLE_T                dialog_hdl;
	UIS_DIALOG_HANDLE_T                dialog_hdl_notice;
	AFW_APP_PORT_HDL_T                 port_id;
	APP_HISTORY_DATA_T *               p_history;
	APP_HANDLER_FUNC_T *               exit_app_function;
	const APP_STATE_TRANSIT_T *        p_state_trans_table;
	const APP_STATE_NAME_TABLE_T *     state_names;
	DL_DB_SHORTCUT_RECORD_T *          p_sc_record;
	APP_NAME_T                         app_name;
	AFW_APP_INSTANCE_ID_T              app_id;
	AFW_APP_REGISTRY_ID_T              reg_id;
	UINT32                             timer_handle;
	APP_TOKEN_STATUS_T                 userinter_token_status;
	SYN_BOOL                           exit_status;
	SYN_BOOL                           in_focus;
	APP_STATE_T                        state;
	APP_STATE_T                        substate;
	UINT8                              security_level;
	BYTE                               padding[2];
} APP_INSTANCE_DATA_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_APPLICATION_H */
