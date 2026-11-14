#include <P2K_SDK_Base.h>
#include <P2K_C_Arg.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_SUAPI.h>
#include <P2K_Logger.h>
#include <P2K_Application.h>

#include <P2K_EP3_APP_Viewer.h>

typedef enum {
	APP_STATE_ANY = 0x00,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATES_T;

typedef struct {
	APP_INSTANCE_DATA_T apd;
	WCHAR viewer_title[VIEWER_TITLE_MAX_LENGTH];
	WCHAR viewer_msg[VIEWER_MESSAGE_MAX_LENGTH];
} APP_DATA_T;

static SYN_RETURN_STATUS_T AppStart(AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, const AFW_EV_DATA_APP_REG_T *reg_hdl);
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
	APP_VIEWER_STARTUP_EVENT
};

static const char app_name_string[APP_NAME_SIZE + 1] = "EP3_Viewer";

void EP3_APP_Viewer_Register(void) {
	D("[EP3_APV]: %s\n", "Enter!");

	const SYN_RETURN_STATUS_T status = APP_Register(
		&app_event_reg_table[0],
		ARRAY_SIZE(app_event_reg_table),
		app_state_trans_table,
		APP_STATE_MAX,
		AppStart
	);
	if (status != SYN_SUCCESS) {
		D("[EP3_APV]: APP_Register() Err, res=%d\n", status);
		return;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Регистрируем в SUAPI главную функцию приложения, вызывающую окно.
	 * Это позволит использовать её глобально без внесения в библиотеки.
	 */
	SU_RET_STATUS result;
	const UINTPTR viewer_function = (UINTPTR) &EP3_API_View;
	suRegisterName(VIEWER_FUNC_NAME, (UINT32) viewer_function, &result);
	if (result != SU_OK) {
		D("[EP3_APV]: suRegisterName() Err, res=%d, name=%s, func=0x%08X\n", result, VIEWER_FUNC_NAME, viewer_function);
		return;
	}
	D("[EP3_APV]: suRegisterName() OK, name=%s, func=0x%08X\n", VIEWER_FUNC_NAME, viewer_function);

	D("[EP3_APV]: %s\n", "Exit!");
}

static SYN_RETURN_STATUS_T AppStart(AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, const AFW_EV_DATA_APP_REG_T *reg_hdl) {
	D("[EP3_APV]: Enter, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X\n", p_evg, reg_id, reg_hdl);
	UNUSED(reg_hdl);

	AFW_EVENT_T *event = AFW_GetEv(p_evg);
	if (event->ev_code != APP_VIEWER_STARTUP_EVENT) {
		D("[EP3_APV]: Unk evt, ev_code=0x%08X\n", event->ev_code);
		return SYN_FAIL;
	}

	if (AFW_InquireRoutingStackByRegId(reg_id) == SYN_SUCCESS) {
		D("[EP3_APV]: Already running! ev_code=0x%08X, reg_id=0x%04X\n", event->ev_code, reg_id);
		return SYN_FAIL;
	}

	D("[EP3_APV]: ev_bsize=%d\n", event->ev_bsize);
	if (event->ev_bsize == 0) {
		D("[EP3_APV]: %s\n", "Err, ev_bsize=0");
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
		D("[EP3_APV]: %s\n", "APP_InitAppData() Err!");
		return SYN_FAIL;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Инициализируем поля структуры приложения данными которые прилетели в ивенте.
	 * Разделителем заголовка и текста является 'ASCII_NULL'.
	 */
	PORTABLE_u_atou((const char *) (event->ev_auxd), appd->viewer_title);
	PORTABLE_u_atou((const char *) (event->ev_auxd) + VIEWER_TITLE_MAX_LENGTH, appd->viewer_msg);

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
		D("[EP3_APV]: %s\n", "APP_Start() Err! Cleaning.");
		status = APP_HandleFailedAppStart(p_evg, &(appd->apd), SYN_NULL);
		/* Clean other fields of Application Data Instance struct (APP_INSTANCE_T) here. */
		memset(appd->viewer_title, 0, sizeof(WCHAR) * VIEWER_TITLE_MAX_LENGTH);
		memset(appd->viewer_msg, 0, sizeof(WCHAR) * VIEWER_MESSAGE_MAX_LENGTH);
	}

	D("[EP3_APV]: Exit! p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X, res=%d\n", p_evg, reg_id, reg_hdl, status);
	return status;
}

static SYN_RETURN_STATUS_T AppExit(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[EP3_APV]: Enter! p_evg=0x%08X, p_apd=0x%08X\n", p_evg, p_apd);

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;
	status |= APP_UtilUISDialogDelete(&(appd->apd.dialog_hdl));
	status |= APP_Exit(p_evg, p_apd, SYN_NULL);

	D("[EP3_APV]: Exit! p_evg=0x%08X, p_apd=0x%08X, res=%d\n", p_evg, p_apd, status);
	return status;
}

static SYN_RETURN_STATUS_T AppView(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[EP3_APV]: Enter! p_evg=0x%08X, p_apd=0x%08X\n", p_evg, p_apd);
	UNUSED(p_evg);

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	UIS_APP_ENV_T app_env;
	UIS_CONTENT_T content;
	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	if (appd->apd.dialog_hdl == UIS_NULL_HANDLE) {
		app_env.routeId = appd->apd.port_id;

		memset(&content, 0, sizeof(UIS_CONTENT_T));
		status |= UIS_MakeContentFromString("q0Nq1", &content, appd->viewer_title, appd->viewer_msg);

		appd->apd.dialog_hdl = UIS_CreateViewer(&app_env, &content, SYN_NULL);
		status |= (appd->apd.dialog_hdl == UIS_NULL_HANDLE);
	}

	D("[EP3_APV]: Exit! p_evg=0x%08X, p_apd=0x%08X, res=%d\n", p_evg, p_apd, status);
	return status;
}

extern STATUS EP3_API_View(const char *title, const char *format, ...) {
	D("[EP3_APV]: %s\n", "Enter!");

	if (EMPTY_STR(format)) {
		return RESULT_FAIL_WRONG_ARG;
	}
	if (EMPTY_STR(title)) {
		return RESULT_FAIL_WRONG_ARG;
	}

	/*
	 * EXL, 15-Nov-2026
	 *
	 * Мы совместим заголовок окна и его содержимое (текст) в одном буфере,
	 * который отправим через систему ивентов. Разделителем заголовка и
	 * текста будет являться 'ASCII_NULL'.
	 */
	char buffer[VIEWER_TITLE_MAX_LENGTH + VIEWER_MESSAGE_MAX_LENGTH];
	PORTABLE_strncpy(buffer, title, VIEWER_TITLE_MAX_LENGTH);
	buffer[VIEWER_TITLE_MAX_LENGTH - 1] = ASCII_NULL;

	va_list args;
	va_start(args, format);
	PORTABLE_TINY_vsnprintf(buffer + VIEWER_TITLE_MAX_LENGTH, VIEWER_MESSAGE_MAX_LENGTH, format, args);
	buffer[VIEWER_TITLE_MAX_LENGTH + VIEWER_MESSAGE_MAX_LENGTH - 1] = ASCII_NULL;
	va_end(args);

	L("[EP3_APV]: %s - %s\n", buffer, buffer + VIEWER_TITLE_MAX_LENGTH);

	const INT32 msg_size = PORTABLE_strlen(buffer + VIEWER_TITLE_MAX_LENGTH);

	const SYN_RETURN_STATUS_T status = AFW_CreateInternalQueuedEvAux(
		APP_VIEWER_STARTUP_EVENT,
		AFW_BUF_FLAG_READ_ONLY,
		VIEWER_TITLE_MAX_LENGTH + msg_size + 1,
		(AFW_EV_BUF_T *) buffer
	);
	if (status != SYN_SUCCESS) {
		D("[EP3_APV]: AFW_CreateInternalQueuedEvAux() Err, res=%d\n", status);
		return RESULT_FAIL;
	}

	D("[EP3_APV]: %s\n", "Exit!");
	return RESULT_OK;
}
