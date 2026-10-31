#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_SUAPI.h>
#include <P2K_AFW_Event.h>
#include <P2K_Application.h>
#include <P2K_UIS.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_Logger.h>

// EXL, 31-Oct-2026: Complex .so test ELF: depends on lib1.so (MyLib clone),
//   lib2.so (constructor + firmware data symbol GLOBAL_VARIABLE) and lib3.so
//   (SO -> SO call into lib1.so). The GLOBAL_VARIABLE value is displayed
//   on the second line of the UI dialog.

// EXL, 31-Oct-2026: lib1.so exports (DT_NEEDED, direct calls), same sources as mylib.so.
extern int mylib_add(int a, int b);
extern int mylib_version(void);

// EXL, 31-Oct-2026: lib2.so exports.
extern UINT32 lib2_get_value(void);
extern UINT32 lib2_get_value_at_init(void);

// EXL, 31-Oct-2026: lib3.so export (SO -> SO -> firmware chain).
extern int lib3_call(int a, int b);

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
	0x0000AFE8
};

static const char app_name_string[APP_NAME_SIZE + 1] = "TestSO2";

// EXL, 31-Oct-2026: Exported to lib1.so (SO -> ELF call), resolved from the ELF exports.
void ui_notify(int value) {
	EP3_Logger("[TestSO2]: ui_notify called from .so library, value=%d\n", value);
}

void Register(void) {
	D("[TestSO2]: %s\n", "Enter!");

	// EXL, 31-Oct-2026: 1. lib1.so direct call (DT_NEEDED), also triggers ui_notify.
	int result = mylib_add(2, 3);
	D("[TestSO2]: mylib_add(2, 3) = %d, version=%d\n", result, mylib_version());

	// EXL, 31-Oct-2026: 2. lib2.so: firmware data symbol via GLOB_DAT import.
	UINT32 global_var = lib2_get_value();
	UINT32 global_var_at_init = lib2_get_value_at_init();
	D("[TestSO2]: GLOBAL_VARIABLE = 0x%08X (at init: 0x%08X)\n", global_var, global_var_at_init);

	// EXL, 31-Oct-2026: 3. lib3.so -> lib1.so (SO -> SO call chain).
	result = lib3_call(7, 35);
	D("[TestSO2]: lib3_call(7, 35) = %d\n", result);

	SYN_RETURN_STATUS_T status = APP_Register(
		&app_event_reg_table[0],
		ARRAY_SIZE(app_event_reg_table),
		app_state_trans_table,
		APP_STATE_MAX,
		AppStart
	);
	if (status != SYN_SUCCESS) {
		D("[TestSO2]: %s\n", "Cannot register TestSO2 application.");
	}

	status |= AFW_CreateInternalQueuedEvAux(
		0x0000AFE8,
		AFW_BUF_FLAG_READ_ONLY,
		0,
		NULL
	);

	D("[TestSO2]: %s\n", "Exit!");
}

static SYN_RETURN_STATUS_T AppStart(
	AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, const AFW_EV_DATA_APP_REG_T *reg_hdl
) {
	D("[TestSO2]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X\n", "Enter!", p_evg, reg_id, reg_hdl);

	AFW_EVENT_T *event = AFW_GetEv(p_evg);
	if (event->ev_code != 0x0000AFE8) {
		D("[TestSO2]: Unknown event ev_code=0x%08X\n", event->ev_code);
		return SYN_FAIL;
	}

	if (AFW_InquireRoutingStackByRegId(reg_id) == SYN_SUCCESS) {
		D("[TestSO2]: App is already running! ev_code=0x%08X, reg_id=0x%04X\n", event->ev_code, reg_id);
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
		D("[TestSO2]: %s\n", "Cannot initialize app instance with data!");
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
		D("[TestSO2]: %s\n", "Cannot start app!");
		status = APP_HandleFailedAppStart(p_evg, &(appd->apd), SYN_NULL);
	}

	D("[TestSO2]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X, ret=%d\n", "Exit!", p_evg, reg_id, reg_hdl, status);
	return status;
}

static SYN_RETURN_STATUS_T AppExit(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[TestSO2]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;
	status |= APP_UtilUISDialogDelete(&(appd->apd.dialog_hdl));

	status |= APP_Exit(p_evg, p_apd, SYN_NULL);

	D("[TestSO2]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}

static SYN_RETURN_STATUS_T AppView(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[TestSO2]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	UIS_APP_ENV_T app_env;
	UIS_CONTENT_T content;

	if (appd->apd.dialog_hdl == UIS_NULL_HANDLE) {
		app_env.routeId = appd->apd.port_id;

		// EXL, 31-Oct-2026: Second line of the UI shows the GLOBAL_VARIABLE value
		//   read by lib2.so (the "q" item is the title, "N" is the newline).
		char second_line[32] = { ASCII_NULL };
		snprintf(second_line, sizeof(second_line), "GLOBAL_VAR=0x%08X", lib2_get_value());

		WCHAR second_line_w[32];
		PORTABLE_u_atou(second_line, second_line_w);

		memset(&content, 0, sizeof(UIS_CONTENT_T));

		status |= UIS_MakeContentFromString("q0Nq1", &content, L"SO Test 2!", second_line_w);

		appd->apd.dialog_hdl = UIS_CreateViewer(&app_env, &content, SYN_NULL);
		status |= (appd->apd.dialog_hdl == UIS_NULL_HANDLE);
	}

	D("[TestSO2]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}
