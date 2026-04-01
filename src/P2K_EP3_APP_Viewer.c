#include <P2K_C_Arg.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_SUAPI.h>
#include <P2K_AFW_Event.h>
#include <P2K_Application.h>
#include <P2K_UIS.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_Logger.h>

#include <P2K_EP3_APP_Viewer.h>

typedef enum {
	APP_STATE_ANY = 0x0,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATES_T;

typedef struct {
	APP_INSTANCE_DATA_T apd;
	WCHAR msg[VIEWER_MESSAGE_MAX_LENGTH];
} APP_DATA_T;

static SYN_RETURN_STATUS_T AppMainStart(AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, void *reg_hdl);
static SYN_RETURN_STATUS_T AppMainExit(AFW_EVENT_GROUP_T *p_evg, void *p_apd);

static SYN_RETURN_STATUS_T AppShowView(AFW_EVENT_GROUP_T *p_evg, void *p_apd);

static const APP_EV_HANDLER_ENTRY_T app_state_any_ev_table[] = {
	{ AFW_EV_SHUT_DOWN_ALL, AppMainExit },
	{ APP_END_EV_TABLE, APP_INVALID_FUNCTION_PTR }
};

static const APP_EV_HANDLER_ENTRY_T app_state_view_ev_table[] = {
	{ AFW_EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ AFW_EV_GAIN_FOCUS, AppShowView },
	{ UIS_EV_DIALOG_DONE, APP_HandleConsumeEvAndExit },
	{ UIS_EV_DONE, APP_HandleConsumeEvAndExit },
	{ APP_END_EV_TABLE, APP_INVALID_FUNCTION_PTR }
};

static const APP_STATE_TRANSIT_T app_state_trans_table[] = {
	{ APP_STATE_ANY, APP_INVALID_FUNCTION_PTR, APP_INVALID_FUNCTION_PTR, app_state_any_ev_table },
	{ APP_STATE_VIEW, APP_INVALID_FUNCTION_PTR, APP_INVALID_FUNCTION_PTR, app_state_view_ev_table },
};

static const AFW_EVENT_CODE_T app_event_reg_table[] = {
	APP_VIEWER_STARTUP_EVENT
};

static const char app_name_string[APP_NAME_SIZE + 1] = "EP3_Viewer";

void EP3_APP_Viewer_Register(void) {
	D("[EP3 APV]: %s\n", "Enter!");

	SYN_RETURN_STATUS_T status = APP_Register(
		&app_event_reg_table[0],
		ARRAY_SIZE(app_event_reg_table),
		app_state_trans_table,
		APP_STATE_MAX,
		(void *) (UINTPTR) AppMainStart
	);
	if (status != SYN_SUCCESS) {
		D("[EP3 APV]: %s\n", "Cannot register EP3_APP_Viewer application.");
	}

	/* Register EP3_API_APP_View() as API function. */
	SU_RET_STATUS result;
	UINTPTR viewer_function = (UINTPTR) &EP3_API_APP_View;
	suRegisterName(VIEWER_FUNC_NAME, (UINT32) viewer_function, &result);
	if (result != SU_OK) {
		D("[EP3 APV]: Register '%s', '0x%08X' failed, result: '%d'.\n", VIEWER_FUNC_NAME, viewer_function, result);
		return;
	}
	D("[EP3 APV]: Registered API function '0x%08X' as '%s' name.\n", viewer_function, VIEWER_FUNC_NAME);

	D("[EP3 APV]: %s\n", "Exit!");
}

static SYN_RETURN_STATUS_T AppMainStart(AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, void *reg_hdl) {
	D("[EP3 APV]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X\n", "Enter!", p_evg, reg_id, reg_hdl);

	AFW_EVENT_T *event = AFW_GetEv(p_evg);
	if (event->ev_code != APP_VIEWER_STARTUP_EVENT) {
		D("[EP3 APV]: Unknown event ev_code=0x%08X\n", event->ev_code);
		return SYN_FAIL;
	}

	if (AFW_InquireRoutingStackByRegId(reg_id) == SYN_SUCCESS) {
		D("[EP3 APV]: App is already running! ev_code=0x%08X, reg_id=0x%04X\n", event->ev_code, reg_id);
		return SYN_FAIL;
	}

	D("[EP3 APV]: Size of buffer event aux data, ev_bsize=%d\n", event->ev_bsize);
	if (event->ev_bsize == 0) {
		D("[EP3 APV]: %s\n", "No buffer aux data in event found!");
		return SYN_FAIL;
	}

	APP_DATA_T *appd = (APP_DATA_T *) APP_InitAppData(
		(void *) (UINTPTR) APP_HandleEvent,
		sizeof(APP_DATA_T),
		reg_id,
		APP_NO_HISTORY, APP_ONE_LEVEL_HISTORY, USERINTER_TOKEN_PRIORITY, AFW_APP_CENTRICITY_SECONDARY,
		AFW_FOCUS, AFW_POSITION_TOP
	);
	if (appd == SYN_NULL) {
		D("[EP3 APV]: %s\n", "Cannot initialize app instance with data!");
		return SYN_FAIL;
	}

	/* Init other fields of Application Data Instance struct (APP_INSTANCE_T) here. */
	PORTABLE_u_atou((const char *) (event->ev_auxd), appd->msg);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	status |= APP_Start(
		p_evg,
		&(appd->apd),
		APP_STATE_VIEW,
		app_state_trans_table,
		AppMainExit,
		app_name_string,
		SYN_NULL
	);
	if (status != SYN_SUCCESS) {
		D("[EP3 APV]: %s\n", "Cannot start app, cleaning all data structs!");
		status = APP_HandleFailedAppStart(p_evg, &(appd->apd), SYN_NULL);
		/* Clean other fields of Application Data Instance struct (APP_INSTANCE_T) here. */
		memset(appd->msg, 0, sizeof(WCHAR) * VIEWER_MESSAGE_MAX_LENGTH);
	}

	D("[EP3 APV]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X, ret=%d\n", "Exit!", p_evg, reg_id, reg_hdl, status);
	return status;
}

static SYN_RETURN_STATUS_T AppMainExit(AFW_EVENT_GROUP_T *p_evg, void *p_apd) {
	D("[EP3 APV]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	// TODO: Should I consume it?
	// status |= APP_ConsumeEv(p_evg, p_apd);

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;
	status |= APP_UtilUISDialogDelete(&(appd->apd.dialog_hdl));

	status |= APP_Exit(p_evg, p_apd, SYN_NULL);

	D("[EP3 APV]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}

static SYN_RETURN_STATUS_T AppShowView(AFW_EVENT_GROUP_T *p_evg, void *p_apd) {
	D("[EP3 APV]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	UIS_APP_ENV_T app_env;
	UIS_CONTENT_T content;

	if (appd->apd.dialog_hdl == UIS_NULL_HANDLE) {
		app_env.routeId = appd->apd.port_id;

		memset(&content, 0, sizeof(UIS_CONTENT_T));
		status |= UIS_MakeContentFromString("q0Nq1", &content, L"EP3 Log Viewer", appd->msg);

		appd->apd.dialog_hdl = UIS_CreateViewer(&app_env, &content, SYN_NULL);
		status |= (appd->apd.dialog_hdl == UIS_NULL_HANDLE);
	}

	D("[EP3 APV]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}

extern BOOL EP3_API_APP_View(const char *format, ...) {
	/* Skip NULL and empty strings. */
	if (!format || !format[0]) {
		return FALSE;
	}

	va_list args;
	va_start(args, format);

	char buffer[VIEWER_MESSAGE_MAX_LENGTH];
	/*
	 * Early M-CORE phones on Patriot and Rainbow SoC have no "vsnprintf" function, so use "vsprintf" instead.
	 * Some early phones like C330 have "visprintf" instead of "vsprintf", but we can alias it.
	 */
#if defined(FTR_NO_VSNPRINTF)
	vsprintf(buffer, format, args);
#else
	vsnprintf(buffer, VIEWER_MESSAGE_MAX_LENGTH, format, args);
#endif /* FTR_NO_VSNPRINTF */
	buffer[VIEWER_MESSAGE_MAX_LENGTH - 1] = ASCII_NULL;

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	INT32 buffer_size = PORTABLE_strlen(buffer);

	status |= AFW_CreateInternalQueuedEvAux(
		APP_VIEWER_STARTUP_EVENT,
		AFW_BUF_FLAG_READ_ONLY,
		buffer_size,
		(AFW_EV_BUF_T *) buffer
	);

	return status;
}
