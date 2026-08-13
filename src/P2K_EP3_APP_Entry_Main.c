#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_SUAPI.h>
#include <P2K_DL_Keypad.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_File_System.h>
#include <P2K_EP3_BIN_Loader.h>
#include <P2K_EP3_APP_Viewer.h>
#include <P2K_EP3_APP_Entry_Main.h>

__attribute__((used, section(".text.bin.entry_point")))
void EP3_BIN_Loader_MainRegister(void) {
	/*
	 * There should be a call to `APP_SOME_MainRegister()` inside this function so that the
	 * some application can also register itself in the system.
	 */
	APP_CALC_MainRegister();

	/*
	 * Register viewer application for informing user.
	 */
	EP3_APP_Viewer_Register();

	/*
	 * We need to check keyboard state, and if it is pressed any key during power-on phone, disable loading and running.
	 */
	if (DL_KeyQueryKeypadActivity() > 0) {
		L("[EP3 BIN]: Keypress detected during registration; loading of '%s' disabled.\n", EP3_ELF_LDR_NAME);
		return;
	}

	WCHAR file_path[PATH_MAX_SHORT];
	if (!EP3_Find_Internal_System_Component(EP3_ELF_LDR_NAME, file_path)) {
		L("[EP3 BIN]: Failed to find '%s' system component.\n", EP3_ELF_LDR_NAME);
		return;
	}

	/* Register EP3_API_BIN_Loader_Load_Program() as API function. */
	SU_RET_STATUS status;
	UINTPTR binloader_func = (UINTPTR) &EP3_API_BIN_Load;
	suRegisterName(BINLOADER_FUNC_NAME, (UINT32) binloader_func, &status);
	if (status != SU_OK) {
		D("[EP3 BIN]: Register '%s', '0x%08X' failed, status: '%d'.\n", BINLOADER_FUNC_NAME, binloader_func, status);
		return;
	}
	D("[EP3 BIN]: Registered API function '0x%08X' as '%s' name.\n", binloader_func, BINLOADER_FUNC_NAME);

	EP3_BIN_LOAD_T EP3_BIN_Load = (EP3_BIN_LOAD_T) suFindName(BINLOADER_FUNC_NAME, SU_NOWAIT, &status);
	if (status != SU_OK) {
		D("[EP3 BIN]: Failed to find func '%s', status: '%d'.\n", BINLOADER_FUNC_NAME, status);
		return;
	}
	if (EP3_BIN_Load == NULL) {
		D("[EP3 BIN]: %s\n", "Function pointer 'EP3_BIN_Loader' is NULL!");
		return;
	}

	UINTPTR load_bin_addr = NULL_ADDR; /* 0x00000000 */
#if defined(FTR_LOAD_TO_ADDR)
	load_bin_addr = (UINTPTR) FTR_LOAD_TO_ADDR;
#endif /* FTR_LOAD_TO_ADDR */

	BOOL set_bit = FALSE;
#if defined(FTR_THUMB_MODE)
	set_bit = TRUE;
#endif /* FTR_THUMB_MODE */

	EP3_BIN_Load(file_path, NULL, load_bin_addr, set_bit, FALSE);
}
