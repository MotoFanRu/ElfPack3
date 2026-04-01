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

#define AFW_INVALID_EVENT_CODE         (-1)
#define APP_END_EV_TABLE               ((UINT32) AFW_INVALID_EVENT_CODE)
#define APP_INVALID_FUNCTION_PTR       (SYN_NULL)

#define APP_NO_HISTORY                 (0) /* No data saved in history. */
#define APP_ONE_LEVEL_HISTORY          (1) /* No state history. */
/* App priority levels are 0...10, 0 being the lowest. */
#define PRIORITY_00                    (0)
#define PRIORITY_01                    (1)
#define PRIORITY_02                    (2)
#define PRIORITY_03                    (3)
#define PRIORITY_04                    (4)
#define PRIORITY_05                    (5)
#define PRIORITY_06                    (6)
#define PRIORITY_07                    (7)
#define PRIORITY_08                    (8)
#define PRIORITY_09                    (9)
#define PRIORITY_10                    (10)
#define LOW_TOKEN_PRIORITY             (PRIORITY_00)
#define USERINTER_TOKEN_PRIORITY       (PRIORITY_01)
#define MID_TOKEN_PRIORITY             (PRIORITY_05)
#define HIGH_TOKEN_PRIORITY            (PRIORITY_07)
#define MAX_TOKEN_PRIORITY             (PRIORITY_10)

typedef const char *                   APP_NAME_T;
typedef const char *                   APP_STATE_NAME_T;

typedef UINT8                          APP_STATE_T;
typedef UINT8                          APP_HISTORY_DATA_T;
typedef AFW_EVENT_CODE_T               APP_EVENT_REG_T;

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

/* should be first in APP_INSTANCE_T */
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

extern SYN_RETURN_STATUS_T APP_Register(
	const APP_EVENT_REG_T              event_reg_table[],
	UINT8                              number_of_events,
	const APP_STATE_TRANSIT_T *        p_state_trans_table,
	APP_STATE_T                        max_states,
	void *                             p_startup_function
);

extern SYN_RETURN_STATUS_T APP_HandleUITokenGranted(AFW_EVENT_GROUP_T *p_evg, void *p_apd);
extern SYN_RETURN_STATUS_T APP_HandleUITokenRevoked(AFW_EVENT_GROUP_T *p_evg, void *p_apd);

extern APP_INSTANCE_DATA_T *APP_InitAppData(
	void *                             handle_event_function,
	UINT32                             app_data_size,
	AFW_APP_REGISTRY_ID_T              reg_id,
	UINT32                             history_size,
	UINT16                             max_state_levels,
	AFW_APP_PRIORITY_T                 token_priority,
	AFW_APP_CENTRICNESS_T              centricity,
	AFW_APP_RSTACK_TYPE_T              routing_stack,
	AFW_APP_RSTACK_POS_T               stack_priority
);

extern void APP_HandleEvent(
	AFW_EVENT_GROUP_T *                p_evg,
	APP_INSTANCE_DATA_T *              p_apd,
	AFW_APP_INSTANCE_ID_T              app_id,
	AFW_APP_REGISTRY_ID_T              reg_id
);

extern void APP_HandleEventPrepost (
	AFW_EVENT_GROUP_T *                p_evg,
	APP_INSTANCE_DATA_T *              p_apd,
	AFW_APP_INSTANCE_ID_T              app_id,
	AFW_APP_REGISTRY_ID_T              reg_id
);

extern SYN_RETURN_STATUS_T APP_Start(
	AFW_EVENT_GROUP_T *                p_evg,
	APP_INSTANCE_DATA_T *              p_apd,
	APP_STATE_T                        init_state,
	const APP_STATE_TRANSIT_T *        p_state_trans_table,
	APP_HANDLER_FUNC_T *               exit_app_function,
	const char *                       p_app_name,
	const APP_STATE_NAME_TABLE_T *     p_state_name_table
);

extern SYN_RETURN_STATUS_T APP_HandleFailedAppStart(
	AFW_EVENT_GROUP_T *                p_evg,
	APP_INSTANCE_DATA_T *              p_apd,
	void *                             p_data
);

extern SYN_RETURN_STATUS_T APP_ConsumeEv(AFW_EVENT_GROUP_T *p_evg, void *p_apd);

extern SYN_RETURN_STATUS_T APP_UtilChangeState(
	APP_STATE_T                        new_state,
	AFW_EVENT_GROUP_T *                p_evg,
	APP_INSTANCE_DATA_T *              p_apd
);

extern SYN_RETURN_STATUS_T APP_UtilConsumeEvChangeState(
	APP_STATE_T                        new_state,
	AFW_EVENT_GROUP_T *                p_evg,
	APP_INSTANCE_DATA_T *              p_apd
);

extern SYN_RETURN_STATUS_T APP_Exit(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd, void *p_data);

extern SYN_RETURN_STATUS_T APP_UtilUISDialogDelete(UIS_DIALOG_HANDLE_T *p_dialog_hdl);

extern SYN_RETURN_STATUS_T APP_HandleConsumeEvAndExit(AFW_EVENT_GROUP_T *p_evg, void *p_apd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_APPLICATION_H */
