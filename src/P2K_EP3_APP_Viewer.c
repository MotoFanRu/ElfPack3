#include <P2K_Application.h>
#include <P2K_Logger.h>

#include <P2K_EP3_APP_Viewer.h>

typedef enum {
	APP_STATE_ANY = 0x0,
	APP_STATE_INIT,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATES_T;

typedef struct {
	APP_INSTANCE_DATA_T app;
} APP_INSTANCE_T;

static SYN_RETURN_STATUS_T AppMainStart(AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, void *reg_hdl);
static SYN_RETURN_STATUS_T AppMainExit(AFW_EVENT_GROUP_T *p_evg, void *p_apd);

static SYN_RETURN_STATUS_T AppViewEnter(AFW_EVENT_GROUP_T *p_evg, void *p_apd, APP_STATE_ENTRY_ACTION_T action);
static SYN_RETURN_STATUS_T AppViewExit(AFW_EVENT_GROUP_T *p_evg, void *p_apd, APP_STATE_ENTRY_ACTION_T action);

static const APP_EV_HANDLER_ENTRY_T app_state_any_ev_table[] = {
	{ AFW_EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ APP_END_EV_TABLE, APP_INVALID_FUNCTION_PTR }
};

static const APP_EV_HANDLER_ENTRY_T app_state_init_ev_table[] = {
	{ AFW_EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ APP_END_EV_TABLE, APP_INVALID_FUNCTION_PTR }
};

static const APP_EV_HANDLER_ENTRY_T app_state_view_ev_table[] = {
	{ APP_END_EV_TABLE, APP_INVALID_FUNCTION_PTR }
};

static const APP_STATE_TRANSIT_T app_state_trans_table[] = {
	{ APP_STATE_ANY, APP_INVALID_FUNCTION_PTR, APP_INVALID_FUNCTION_PTR, app_state_any_ev_table },
	{ APP_STATE_INIT, APP_INVALID_FUNCTION_PTR, APP_INVALID_FUNCTION_PTR, app_state_init_ev_table },
	{ APP_STATE_VIEW, AppViewEnter, AppViewExit, app_state_view_ev_table },
};

static const AFW_EVENT_CODE_T app_event_reg_table[] = {
	APP_VIEWER_STARTUP_EVENT
};

static const char app_name_string[APP_NAME_SIZE + 1] = "EP3_Viewer";

void EP3_APP_Viewer_Register(void) {
	SYN_RETURN_STATUS_T status = APP_Register(
		&app_event_reg_table[0], 1, app_state_trans_table, APP_STATE_MAX, (void *) (uintptr_t) AppMainStart
	);
	if (status != SYN_SUCCESS) {
		L("%s\n", "Cannot register EP3_APP_Viewer application.");
	}
}

static SYN_RETURN_STATUS_T AppMainStart(AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, void *reg_hdl) {
	UNUSED(reg_hdl);

	AFW_EVENT_CODE_T ev_code = AFW_GetEvCode(p_evg);
	if (ev_code != APP_VIEWER_STARTUP_EVENT) {
		D("Unknown event code 0x%08X\n", ev_code);
		return SYN_FAIL;
	}

	if (AFW_InquireRoutingStackByRegId(reg_id) == SYN_SUCCESS) {
		D("App is already running: 0x%08X 0x%08X\n", ev_code, reg_id);
		return SYN_FAIL;
	}

	APP_INSTANCE_T *appi = (APP_INSTANCE_T *) APP_InitAppData(
		(void *) (uintptr_t) APP_HandleEvent,
		sizeof(APP_INSTANCE_T),
		reg_id,
		APP_NO_HISTORY, APP_ONE_LEVEL_HISTORY, APP_PRIORITY_USER, AFW_APP_CENTRICITY_SECONDARY,
		AFW_FOCUS, AFW_POSITION_TOP
	);
	if (appi == SYN_NULL) {
		D("%s\n", "Cannot initialize app instance with data!");
		return SYN_FAIL;
	}

	/* appi default initialization! */

	SYN_RETURN_STATUS_T status = APP_Start(
		p_evg,
		&(appi->app),
		APP_STATE_INIT,
		app_state_trans_table,
		AppMainExit,
		app_name_string, SYN_NULL
	);
	if (status != SYN_SUCCESS) {
		D("%s\n", "Cannot start app, cleaning!");
		status = APP_HandleFailedAppStart(p_evg, &(appi->app), SYN_NULL);
		/* appi default cleaning! */
	}

	return status;
}

static SYN_RETURN_STATUS_T AppMainExit(AFW_EVENT_GROUP_T *p_evg, void *p_apd) {
	UNUSED(p_evg);
	UNUSED(p_apd);
	return SYN_SUCCESS;
}

static SYN_RETURN_STATUS_T AppViewEnter(AFW_EVENT_GROUP_T *p_evg, void *p_apd, APP_STATE_ENTRY_ACTION_T action) {
	UNUSED(p_evg);
	UNUSED(p_apd);
	UNUSED(action);

	if (action == APP_VIEW_ENTER) {

		// UIS_Delete
		// memset
		// memclr
		

	}

	return SYN_SUCCESS;
}

static SYN_RETURN_STATUS_T AppViewExit(AFW_EVENT_GROUP_T *p_evg, void *p_apd, APP_STATE_ENTRY_ACTION_T action) {
	UNUSED(p_evg);
	UNUSED(p_apd);
	UNUSED(action);
	return SYN_SUCCESS;
}
