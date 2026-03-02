#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_BIN_Loader.h>
#include <P2K_EP3_File_System.h>

void EP3_BIN_Loader_MainRegister(void) {
	/*
	 * There should be a call to `APP_CALC_MainRegister()` inside this function so that the
	 * calculator application can also register itself in the system.
	 */
	APP_CALC_MainRegister();

	/*
	 * We need to check the state of the some "No Load" key, and if it is pressed during power-on phone,
	 * just disable BIN Loader loading and running.
	 */
	if (!IsNoLoadKeyPressed()) {
		WCHAR file_path[PATH_MAX_SHORT];
		if (EP3_Find_Internal_System_Component(EP3_ELF_LDR_NAME, file_path)) {
			DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
			if (file_handle != DL_FS_HANDLE_INVALID) {
				DL_FS_SIZE_T file_size = DL_FsGetFileSize(file_handle);
				if (file_size > 32) {
					BYTE *load_addr = EP3_AllocateMemory(file_size);
					if (load_addr != NULL) {
						DL_FS_COUNT_T elements_read;
						DL_FS_RESULT_T result = DL_FsReadFile(load_addr, file_size, 1, file_handle, &elements_read);
						if (result == DL_FS_RESULT_SUCCESS) {
							if (DL_FsCloseFile(file_handle) == DL_FS_HANDLE_INVALID) {
								L("[EP3 BIN]: Cannot close '%s' file!\n", EP3_ELF_LDR_NAME);
							}
							((void (*) (void)) load_addr)();
						} else {
							L("[EP3 BIN]: Cannot read '%s' file to 0x%08X address!\n", EP3_ELF_LDR_NAME, load_addr);
						}
					} else {
						L("[EP3 BIN]: Cannot allocate %d bytes of memory!\n", file_size);
					}
				} else {
					L("[EP3 BIN]: File '%s' < 32 bytes, too small!\n", EP3_ELF_LDR_NAME);
				}
				if (DL_FsCloseFile(file_handle) == DL_FS_HANDLE_INVALID) {
					L("[EP3 BIN]: Cannot close '%s' file!\n", EP3_ELF_LDR_NAME);
				}
			} else {
				L("[EP3 BIN]: Cannot open '%s' file!\n", EP3_ELF_LDR_NAME);
			}
		} else {
			L("[EP3 BIN]: Cannot find '%s' file!\n", EP3_ELF_LDR_NAME);
		}
	} else {
		L("[EP3 BIN]: No Load Key was pressed. Disable '%s' loading!\n", EP3_ELF_LDR_NAME);
	}
}
