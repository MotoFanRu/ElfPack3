#include <P2K_SDK_Base.h>
#include <P2K_C_Arg.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_SUAPI.h>
#include <P2K_Application.h>

#include <P2K_Logger.h>

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

static SYN_RETURN_STATUS_T AppStart(
	AFW_EVENT_GROUP_T *p_evg,
	AFW_APP_REGISTRY_ID_T reg_id,
	const AFW_EV_DATA_APP_REG_T *reg_hdl
);
static SYN_RETURN_STATUS_T AppExit(
	AFW_EVENT_GROUP_T *p_evg,
	APP_INSTANCE_DATA_T *p_apd
);
static SYN_RETURN_STATUS_T AppView(
	AFW_EVENT_GROUP_T *p_evg,
	APP_INSTANCE_DATA_T *p_apd
);

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
	{
		APP_STATE_ANY,
		APP_INVALID_FUNCTION_PTR,
		APP_INVALID_FUNCTION_PTR,
		app_state_any_ev_table
	},
	{
		APP_STATE_VIEW,
		APP_INVALID_FUNCTION_PTR,
		APP_INVALID_FUNCTION_PTR,
		app_state_view_ev_table
	},
};

static const AFW_EVENT_CODE_T app_event_reg_table[] = {
	APP_VIEWER_STARTUP_EVENT
};

static const char app_name_string[APP_NAME_SIZE + 1] = "EP3_Viewer";

void EP3_APP_Viewer_Register(void) {
	P();

	const SYN_RETURN_STATUS_T status = APP_Register(
		&app_event_reg_table[0],
		ARRAY_SIZE(app_event_reg_table),
		app_state_trans_table,
		APP_STATE_MAX,
		AppStart
	);
	if (status != SYN_SUCCESS) {
		D("[EP3 APV E]: Cannot register: %s\n", app_name_string);
		D("[EP3 APV E]: %s\n", "APP_Register()");
		D("[EP3 APV E]:  status=%d\n", status);
		D("[EP3 APV E]:  AppStart=0x%08X\n", (UINTPTR) AppStart);
		return;
	}
	D("[EP3 APV D]: Registered %s\n", app_name_string);
	D("[EP3 APV D]: %s\n", "APP_Register()");
	D("[EP3 APV D]:  status=%d\n", status);
	D("[EP3 APV D]:  AppStart=0x%08X\n", (UINTPTR) AppStart);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Регистрируем в SUAPI главную функцию вызывающую окно как API.
	 * Это позволит использовать её глобально без внесения в библиотеки.
	 */
	SU_RET_STATUS result;
	const UINTPTR func_addr = (UINTPTR) &EP3_API_View;
	suRegisterName(APP_VIEWER_FUNC_NAME, (UINT32) func_addr, &result);
	if (result != SU_OK) {
		D("[EP3 APV E]: %s\n", "Cannot register " APP_VIEWER_FUNC_NAME);
		D("[EP3 APV E]: %s\n", "suRegisterName()");
		D("[EP3 APV E]:  status=%d\n", status);
		D("[EP3 APV E]:  func_addr=0x%08X\n", (UINTPTR) func_addr);
		return;
	}
	D("[EP3 APV D]: %s\n", "Registered " APP_VIEWER_FUNC_NAME);
	D("[EP3 APV D]: %s\n", "suRegisterName()");
	D("[EP3 APV D]:  status=%d\n", status);
	D("[EP3 APV D]:  func_addr=0x%08X\n", (UINTPTR) func_addr);
}

static SYN_RETURN_STATUS_T AppStart(
	AFW_EVENT_GROUP_T *p_evg,
	AFW_APP_REGISTRY_ID_T reg_id,
	const AFW_EV_DATA_APP_REG_T *reg_hdl
) {
	UNUSED(reg_hdl);

	P();
	D("[EP3 APV D]: %s\n", "AppStart()");
	D("[EP3 APV D]:  p_evg=0x%08X\n", p_evg);
	D("[EP3 APV D]:  reg_id=0x%04X\n", reg_id);
	D("[EP3 APV D]:  reg_hdl=0x%08X\n", reg_hdl);

	if (AFW_InquireRoutingStackByRegId(reg_id) == SYN_SUCCESS) {
		D("[EP3 APV E]: %s\n", "The app is already running");
		D("[EP3 APV E]: %s\n", "AFW_InquireRoutingStackByRegId()");
		D("[EP3 APV E]:  reg_id=0x%04X\n", reg_id);
		return SYN_FAIL;
	}

	AFW_EVENT_T *event = AFW_GetEv(p_evg);
	if (event->ev_code != APP_VIEWER_STARTUP_EVENT) {
		D("[EP3 APV E]: %s\n", "Unknown event");
		D("[EP3 APV E]: %s\n", "AFW_GetEv()");
		D("[EP3 APV E]:  ev_code=0x%08X\n", event->ev_code);
		return SYN_FAIL;
	}
	if (event->ev_bsize == 0) {
		D("[EP3 APV E]: %s\n", "No event data");
		D("[EP3 APV E]: %s\n", "AFW_GetEv()");
		D("[EP3 APV E]:  ev_bsize=0x%08X\n", event->ev_bsize);
		return SYN_FAIL;
	}
	D("[EP3 APV D]: %s\n", "Event with data");
	D("[EP3 APV D]: %s\n", "AFW_GetEv()");
	D("[EP3 APV D]:  ev_bsize=0x%08X\n", event->ev_bsize);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Инициализируем и создаём структуру приложения с данными.
	 */
	APP_DATA_T *appd = (APP_DATA_T *) APP_InitAppData(
		APP_HandleEvent,
		sizeof(APP_DATA_T),
		reg_id,
		APP_NO_HISTORY, APP_ONE_LEVEL_HISTORY, USERINTER_TOKEN_PRIORITY,
		AFW_APP_CENTRICITY_SECONDARY, AFW_FOCUS, AFW_POSITION_TOP
	);
	if (appd == SYN_NULL) {
		D("[EP3 APV E]: %s\n", "Cannot init app struct and data");
		D("[EP3 APV E]: %s\n", "APP_InitAppData()");
		D("[EP3 APV E]:  appd=%s\n", "SYN_NULL");
		return SYN_FAIL;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Инициализируем поля структуры приложения данными, которые по итогу
	 * прилетели в отправленном ивенте.
	 *
	 * Разделителем заголовка и текста является обычный символ 'ASCII_NULL'.
	 */
	PORTABLE_u_atou((const char *) (event->ev_auxd), appd->viewer_title);
	PORTABLE_u_atou(
		(const char *) ((event->ev_auxd) + VIEWER_TITLE_MAX_LENGTH),
		appd->viewer_msg
	);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Запускаем наше приложение.
	 */
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
		D("[EP3 APV E]: %s\n", "Cannot start app");
		D("[EP3 APV E]: %s\n", "APP_Start()");
		D("[EP3 APV E]:  status=%d\n", status);

		status = APP_HandleFailedAppStart(p_evg, &(appd->apd), SYN_NULL);

		/* Clean or free additional fields of `APP_INSTANCE_T` struct here. */
		appd->viewer_title[0] = UNICODE_NULL;
		appd->viewer_msg[0] = UNICODE_NULL;
	}

	return status;
}

static SYN_RETURN_STATUS_T AppExit(
	AFW_EVENT_GROUP_T *p_evg,
	APP_INSTANCE_DATA_T *p_apd
) {
	P();
	D("[EP3 APV D]: %s\n", "AppExit()");
	D("[EP3 APV D]:  p_evg=0x%08X\n", p_evg);
	D("[EP3 APV D]:  p_apd=0x%08X\n", p_apd);

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;
	status |= APP_UtilUISDialogDelete(&(appd->apd.dialog_hdl));
	status |= APP_Exit(p_evg, p_apd, SYN_NULL);

	return status;
}

static SYN_RETURN_STATUS_T AppView(
	AFW_EVENT_GROUP_T *p_evg,
	APP_INSTANCE_DATA_T *p_apd
) {
	UNUSED(p_evg);

	P();
	D("[EP3 APV D]: %s\n", "AppView()");
	D("[EP3 APV D]:  p_evg=0x%08X\n", p_evg);
	D("[EP3 APV D]:  p_apd=0x%08X\n", p_apd);

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;
	if (appd->apd.dialog_hdl == UIS_NULL_HANDLE) {
		UIS_APP_ENV_T app_env;
		UIS_CONTENT_T content;

		app_env.routeId = appd->apd.port_id;

		memset(&content, 0, sizeof(UIS_CONTENT_T));
		status |= UIS_MakeContentFromString(
			"q0Nq1", &content, appd->viewer_title, appd->viewer_msg
		);

		appd->apd.dialog_hdl = UIS_CreateViewer(&app_env, &content, SYN_NULL);
		status |= (appd->apd.dialog_hdl == UIS_NULL_HANDLE);
	}

	return status;
}

extern STATUS EP3_API_View(const char *title, const char *format, ...) {
	P();

	if (EMPTY_STR(format) || EMPTY_STR(title)) {
		return RESULT_FAIL_WRONG_ARG;
	}

	/*
	 * EXL, 15-Nov-2026
	 *
	 * Мы совместим заголовок окна и его содержимое (текст) в одном буфере,
	 * который отправим через систему ивентов. Разделителем заголовка и
	 * текста будет являться обычный символ 'ASCII_NULL'.
	 */
	char buf[VIEWER_TITLE_MAX_LENGTH + VIEWER_MESSAGE_MAX_LENGTH];
	PORTABLE_strncpy(buf, title, VIEWER_TITLE_MAX_LENGTH);
	buf[VIEWER_TITLE_MAX_LENGTH - 1] = ASCII_NULL;

	va_list args;
	va_start(args, format);

	PORTABLE_TINY_vsnprintf(
		buf + VIEWER_TITLE_MAX_LENGTH,
		VIEWER_MESSAGE_MAX_LENGTH,
		format,
		args
	);
	buf[VIEWER_TITLE_MAX_LENGTH + VIEWER_MESSAGE_MAX_LENGTH - 1] = ASCII_NULL;

	va_end(args);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Отправляем ивент запуска приложения с сопутствующей нагрузкой в виде
	 * буфера в котором имеется текст, разделённый на заголовок и тело
	 * сообщения. Испущенный ивент запустит функцию `AppStart()` в приложении.
	 */
	const INT32 msg_size = PORTABLE_strlen(buf + VIEWER_TITLE_MAX_LENGTH);
	const SYN_RETURN_STATUS_T status = AFW_CreateInternalQueuedEvAux(
		APP_VIEWER_STARTUP_EVENT,
		AFW_BUF_FLAG_READ_ONLY,
		VIEWER_TITLE_MAX_LENGTH + msg_size + 1,
		(AFW_EV_BUF_T *) buf
	);
	if (status != SYN_SUCCESS) {
		D("[EP3 APV E]: %s\n", "Cannot send event with data");
		D("[EP3 APV E]: %s\n", "AFW_CreateInternalQueuedEvAux()");
		D("[EP3 APV E]:  status=%d\n", status);
		return RESULT_FAIL;
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Отправим сообщение в лог на всякий случай тоже.
	 */
	L("[EP3 APV I]: %s - %s\n", buf, buf + VIEWER_TITLE_MAX_LENGTH);

	return RESULT_OK;
}
