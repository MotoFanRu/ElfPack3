#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_SUAPI.h>
#include <P2K_AFW_Event.h>
#include <P2K_Application.h>
#include <P2K_UIS.h>
#include <P2K_Logger.h>

#include <P2K_EP3_Image_Args.h>

// EXL, 31-Oct-2026: Test ELF for the .so loader. Built with --export-dynamic (exports
//   ui_notify for the SO -> ELF call) and linked against mylib.so (DT_NEEDED auto-load).
//   Also loads mylib.so via dlopen/dlsym to test the explicit dl API.

// EXL, 31-Oct-2026: dl API, resolved from the DEF library table at runtime (see
//   P2K_EP3_SO_Loader.c, EP3_dlopen/EP3_dlsym/EP3_dlclose/EP3_dlerror).
extern void *dlopen(const char *name);
extern void *dlsym(void *handle, const char *name);
extern int dlclose(void *handle);
extern const char *dlerror(void);

// EXL, 31-Oct-2026: mylib.so functions, linked as DT_NEEDED dependency (direct calls).
extern int mylib_add(int a, int b);
extern int mylib_version(void);

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
	0x0000AFEE
};

static const char app_name_string[APP_NAME_SIZE + 1] = "TestSO";

// EXL, 31-Oct-2026: Exported to mylib.so (SO -> ELF call), resolved from the ELF
//   dynamic exports by the loader provider registry.
void ui_notify(int value) {
	EP3_Logger("[TestSO]: ui_notify called from .so library, value=%d\n", value);
}

// EXL, 31-Oct-2026: dlerror() returns NULL when there is no error (POSIX), but the
//   freestanding logger prints garbage for NULL — show it as an empty string.
static const char *dl_err_or_empty(void) {
	const char *err = dlerror();
	return (err != NULL) ? err : "";
}

UINT32 EP3_Main(EP3_ImageArgs_T *args) {
	UNUSED(args);
	D("[TestSO]: %s\n", "Enter!");

	// EXL, 31-Oct-2026: 1. Direct call via DT_NEEDED (mylib.so auto-loaded at ELF load).
	int result = mylib_add(2, 3);
	D("[TestSO]: mylib_add(2, 3) = %d, version=%d\n", result, mylib_version());

	// EXL, 31-Oct-2026: 2. Explicit dlopen + dlsym path.
	void *handle = dlopen("mylib.so");
	D("[TestSO]: dlopen(mylib.so) = 0x%08X, dlerror='%s'\n", handle, dl_err_or_empty());

	if (handle != NULL) {
		typedef int (*mylib_add_t)(int a, int b);
		mylib_add_t add_fn = (mylib_add_t) ((UINTPTR) dlsym(handle, "mylib_add"));
		D("[TestSO]: dlsym(mylib_add) = 0x%08X, dlerror='%s'\n", add_fn, dl_err_or_empty());

		if (add_fn != NULL) {
			result = add_fn(20, 23);
			D("[TestSO]: dlsym mylib_add(20, 23) = %d\n", result);
		}

		void *missing_fn = dlsym(handle, "mylib_missing");
		D("[TestSO]: dlsym(mylib_missing) = 0x%08X, dlerror='%s'\n", missing_fn, dl_err_or_empty());

		// EXL, 31-Oct-2026: dlclose drops the dlopen reference, the library stays
		//   in memory because the DT_NEEDED dependency still holds a refcount.
		dlclose(handle);
	}

	SYN_RETURN_STATUS_T status = APP_Register(
		&app_event_reg_table[0],
		ARRAY_SIZE(app_event_reg_table),
		app_state_trans_table,
		APP_STATE_MAX,
		AppStart
	);
	if (status != SYN_SUCCESS) {
		D("[TestSO]: %s\n", "Cannot register TestSO application.");
	}

	status |= AFW_CreateInternalQueuedEvAux(
		0x0000AFEE,
		AFW_BUF_FLAG_READ_ONLY,
		0,
		NULL
	);

	D("[TestSO]: %s\n", "Exit!");
	// EXL, 02-Nov-2026: RESULT_OK keeps the image loaded.
	return RESULT_OK;

}

static SYN_RETURN_STATUS_T AppStart(
	AFW_EVENT_GROUP_T *p_evg, AFW_APP_REGISTRY_ID_T reg_id, const AFW_EV_DATA_APP_REG_T *reg_hdl
) {
	D("[TestSO]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X\n", "Enter!", p_evg, reg_id, reg_hdl);

	AFW_EVENT_T *event = AFW_GetEv(p_evg);
	if (event->ev_code != 0x0000AFEE) {
		D("[TestSO]: Unknown event ev_code=0x%08X\n", event->ev_code);
		return SYN_FAIL;
	}

	if (AFW_InquireRoutingStackByRegId(reg_id) == SYN_SUCCESS) {
		D("[TestSO]: App is already running! ev_code=0x%08X, reg_id=0x%04X\n", event->ev_code, reg_id);
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
		D("[TestSO]: %s\n", "Cannot initialize app instance with data!");
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
		D("[TestSO]: %s\n", "Cannot start app!");
		status = APP_HandleFailedAppStart(p_evg, &(appd->apd), SYN_NULL);
	}

	D("[TestSO]: %s, p_evg=0x%08X, reg_id=0x%04X, reg_hdl=0x%08X, ret=%d\n", "Exit!", p_evg, reg_id, reg_hdl, status);
	return status;
}

static SYN_RETURN_STATUS_T AppExit(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[TestSO]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;
	status |= APP_UtilUISDialogDelete(&(appd->apd.dialog_hdl));

	status |= APP_Exit(p_evg, p_apd, SYN_NULL);

	D("[TestSO]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}

static SYN_RETURN_STATUS_T AppView(AFW_EVENT_GROUP_T *p_evg, APP_INSTANCE_DATA_T *p_apd) {
	D("[TestSO]: %s, p_evg=0x%08X, p_apd=0x%08X\n", "Enter!", p_evg, p_apd);

	SYN_RETURN_STATUS_T status = SYN_SUCCESS;

	APP_DATA_T *appd = (APP_DATA_T *) p_apd;

	UIS_APP_ENV_T app_env;
	UIS_CONTENT_T content;

	if (appd->apd.dialog_hdl == UIS_NULL_HANDLE) {
		app_env.routeId = appd->apd.port_id;

		memset(&content, 0, sizeof(UIS_CONTENT_T));
		status |= UIS_MakeContentFromString("q0Nq1", &content, L"SO Test!", L"Hello Moto SO ELF!");

		appd->apd.dialog_hdl = UIS_CreateViewer(&app_env, &content, SYN_NULL);
		status |= (appd->apd.dialog_hdl == UIS_NULL_HANDLE);
	}
	D("[TestSO]: %s, p_evg=0x%08X, p_apd=0x%08X, ret=%d\n", "Exit!", p_evg, p_apd, status);

	return status;
}
