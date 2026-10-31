#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_SUAPI.h>
#include <P2K_AFW_Event.h>
#include <P2K_Application.h>
#include <P2K_UIS.h>
#include <P2K_Logger.h>

typedef enum {
	APP_STATE_ANY = 0x0,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATES_T;

typedef struct {
	APP_INSTANCE_DATA_T apd;
} APP_DATA_T;

static SYN_RETURN_STATUS_T AppStart(
	AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, const AFW_EV_DATA_APP_REG_T *reg_hdl
);
static SYN_RETURN_STATUS_T AppExit(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd);

static SYN_RETURN_STATUS_T AppView(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd);

static const APP_EV_HANDLER_ENTRY_T app_state_any_ev_table[] = {
	{ AFW_EV_SHUT_DOWN_ALL, AppExit },
	{ APP_END_EV_TABLE, APP_INVALID_FUNCTION_PTR }
};

static const APP_EV_HANDLER_ENTRY_T app_state_view_ev_table[] = {
	{ AFW_EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ AFW_EV_GAIN_FOCUS, AppView },
	{ UIS_EV_DIALOG_DONE, APP_HandleConsumeEvAndExit },
	{ UIS_EV_DONE, APP_HandleConsumeEvAndExit },
	{ APP_END_EV_TABLE, APP_INVALID_FUNCTION_PTR }
};

static const APP_STATE_TRANSIT_T app_state_trans_table[] = {
	{ APP_STATE_ANY, APP_INVALID_FUNCTION_PTR, APP_INVALID_FUNCTION_PTR, app_state_any_ev_table },
	{ APP_STATE_VIEW, APP_INVALID_FUNCTION_PTR, APP_INVALID_FUNCTION_PTR, app_state_view_ev_table },
};

static const AFW_EVENT_CODE_T app_event_reg_table[] = {
	0x0000AFED
};

static const char app_name_string[APP_NAME_SIZE + 1] = "HelloUI";

void Register(void) {
	D("[HelloUI]: %s\n", "Enter!");

	SYN_RETURN_STATUS_T status = APP_Register(
		&app_event_reg_table[0],
		ARRAY_SIZE(app_event_reg_table),
		app_state_trans_table,
		APP_STATE_MAX,
		AppStart
	);
	if (status != SYN_SUCCESS) {
		D("[HelloUI]: %s\n", "Cannot register EP3_APP_Viewer application.");
	}

	status |= AFW_CreateInternalQueuedEvAux(
		0x0000AFED,
		AFW_BUF_FLAG_READ_ONLY,
		0,
		NULL
	);

	D("[HelloUI]: %s\n", "Exit!");
}

static SYN_RETURN_STATUS_T AppStart(
	AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, const AFW_EV_DATA_APP_REG_T *reg_hdl
) {
	D("[HelloUI]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X\n", "Enter!", p_evg, reg_id, reg_hdl);

	AFW_EVENT_T *event = AFW_GetEv(p_evg);
	if (event->ev_code != 0x0000AFED) {
		D("[HelloUI]: Unknown event ev_code=0x%08X\n", event->ev_code);
		return SYN_FAIL;
	}

	if (AFW_InquireRoutingStackByRegId(reg_id) == SYN_SUCCESS) {
		D("[HelloUI]: App is already running! ev_code=0x%08X, reg_id=0x%04X\n", event->ev_code, reg_id);
		return SYN_FAIL;
	}

	APP_DATA_T *appd = (APP_DATA_T *) APP_InitAppData(
		APP_HandleEvent,
		sizeof(APP_DATA_T),
		reg_id,
		APP_NO_HISTORY, APP_ONE_LEVEL_HISTORY, USERINTER_TOKEN_PRIORITY, AFW_APP_CENTRICITY_SECONDARY,
		AFW_FOCUS, AFW_POSITION_TOP
	);
	if (appd == SYN_NULL) {
		D("[HelloUI]: %s\n", "Cannot initialize app instance with data!");
		return SYN_FAIL;
	}

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	status |= APP_Start(
		p_evg,
		&(appd->apd),
		APP_STATE_VIEW,
		app_state_trans_table,
		AppExit,
		app_name_string,
		SYN_NULL
	);
	if (status != SYN_SUCCESS) {
		D("[HelloUI]: %s\n", "Cannot start app!");
		status = APP_HandleFailedAppStart(p_evg, &(appd->apd), SYN_NULL);
	}

	D("[HelloUI]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X, ret=%d\n", "Exit!", p_evg, reg_id, reg_hdl, status);
	return status;
}

static SYN_RETURN_STATUS_T AppExit(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[HelloUI]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;
	status |= APP_UtilUISDialogDelete(&(appd->apd.dialog_hdl));

	status |= APP_Exit(p_evg, p_apd, SYN_NULL);

	D("[HelloUI]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}

static SYN_RETURN_STATUS_T AppView(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[HelloUI]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	UIS_APP_ENV_T app_env;
	UIS_CONTENT_T content;

	if (appd->apd.dialog_hdl == UIS_NULL_HANDLE) {
		app_env.routeId = appd->apd.port_id;

		memset(&content, 0, sizeof(UIS_CONTENT_T));
		status |= UIS_MakeContentFromString("q0Nq1", &content, L"ELF 1 Title!", L"Hello Moto 1 ELF!");

		appd->apd.dialog_hdl = UIS_CreateViewer(&app_env, &content, SYN_NULL);
		status |= (appd->apd.dialog_hdl == UIS_NULL_HANDLE);
	}

	D("[HelloUI]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}
