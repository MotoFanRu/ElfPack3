#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_DL_Keypad.h>
#include <P2K_DL_File_System.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_File_System.h>
#include <P2K_EP3_BIN_Loader.h>

__attribute__((used, section(".text.bin.entry_point")))
void EP3_BIN_Loader_MainRegister(void) {
	/*
	 * There should be a call to `APP_CALC_MainRegister()` inside this function so that the
	 * calculator application can also register itself in the system.
	 */
	APP_CALC_MainRegister();

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

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		L("[EP3 BIN]: Failed to open '%s' file.\n", EP3_ELF_LDR_NAME);
		return;
	}

	/* Use `DL_FsSGetFileSize()` instead `DL_FsGetFileSize()` to reduce the number of functions. */
	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(file_path, DL_FS_OWNER_RESERVED);
	if (file_size < FILE_SIZE_TOO_SMALL) {
		L("[EP3 BIN]: File '%s' too small (%d bytes).\n", EP3_ELF_LDR_NAME, file_size);
		DL_FsCloseFile(file_handle);
		return;
	}

	/* Add additional bytes to align the address space % 4 bytes.*/
	BYTE *load_addr = (BYTE *) EP3_Memory_Alloc(file_size + 4);
	if (load_addr == NULL) {
		L("[EP3 BIN]: Failed to allocate %d bytes of memory.\n", file_size);
		DL_FsCloseFile(file_handle);
		return;
	}

	/* Align load address to 4 bytes. */
	load_addr = (BYTE *) (((UINTPTR) load_addr + 3) & ~3UL);

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(load_addr, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		L("[EP3 BIN]: Failed to read '%s' into memory at '0x%08X' address.\n", EP3_ELF_LDR_NAME, (UINT32) load_addr);
		DL_FsCloseFile(file_handle);
		return;
	}

	DL_FsCloseFile(file_handle);

	/* Execute the loaded binary, set Thumb bit if needed (will be ignored on ARM and M-CORE). */
	L("[EP3 BIN]: Loaded %d bytes to 0x%08X memory address, now jump to it.\n", file_size, (UINTPTR) load_addr);
#if defined(FTR_THUMB_MODE)
	load_addr += 1;
#endif /* FTR_THUMB_MODE */
	EP3_ELF_LDR_ENTRY_POINT_T entry_point = ((EP3_ELF_LDR_ENTRY_POINT_T) (UINTPTR) load_addr);
	entry_point();
}
