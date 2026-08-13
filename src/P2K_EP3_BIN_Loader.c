#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_BIN_Loader.h>

STATUS EP3_API_BIN_Load(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL a_bit, BOOL free_it) {
	if ((file_path == NULL) || (file_path[0] == UNICODE_NULL)) {
		D("[EP3 BIN]: %s\n", "Argument file_path is NULL or empty.");
		return RESULT_FAIL;
	}

	/* Use `DL_FsSGetFileSize()` instead `DL_FsGetFileSize()` to reduce the number of functions. */
	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(file_path, DL_FS_OWNER_RESERVED);
	if (file_size < FILE_SIZE_TOO_SMALL) {
		L("[EP3 BIN]: File '%s' is not exist or too small (%d bytes).\n", EP3_ELF_LDR_NAME, file_size);
		return RESULT_FAIL;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		L("[EP3 BIN]: Failed to open '%s' file.\n", EP3_ELF_LDR_NAME);
		return RESULT_FAIL;
	}

	/*
	* Since the M-CORE GCC cross-compiler cannot generate position-independent binaries (-fPIC, -fPIE + -pie),
	* we cannot use the random memory address that the system allocator gives us.
	* Therefore, to make things simpler, it is best to use a fixed load address.
	*/
	UINTPTR *load_addr = (UINTPTR *) addr;
	if (addr == NULL_ADDR) {
		/* Add additional 4 bytes to align the address space. */
		load_addr = (UINTPTR *) EP3_Memory_Alloc(file_size + 4);
		if (load_addr == NULL) {
			L("[EP3 BIN]: Failed to allocate %d bytes of memory.\n", file_size + 4);
			DL_FsCloseFile(file_handle);
			return RESULT_FAIL;
		}

		/* Align load address to 4 bytes. */
		load_addr = (UINTPTR *) (((UINTPTR) load_addr + 3) & ~3UL);
	}

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(load_addr, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		L("[EP3 BIN]: Failed to read '%s' into memory at '0x%08X' address.\n", EP3_ELF_LDR_NAME, load_addr);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	if (elements_read != 1) {
		L("[EP3 BIN]: elements_read=%d, should be 1.\n", elements_read);
		DL_FsCloseFile(file_handle);
		return RESULT_FAIL;
	}
	L("[EP3 BIN]: Loaded %d bytes to '0x%08X' memory address.\n", file_size, load_addr);

	DL_FsCloseFile(file_handle);

	/* Set Thumb etc. bit if needed (will be ignored on ARM and M-CORE) */
	if (a_bit) {
		load_addr = (UINTPTR *) (((UINT32) load_addr) | 1);
	}

	/* Execute the loaded binary. */
	L("[EP3 BIN]: Jump to '0x%08X' memory address.\n", load_addr);
	EP3_ELF_LDR_ENTRY_POINT_T entry_point = (EP3_ELF_LDR_ENTRY_POINT_T) ((UINTPTR) load_addr);
	STATUS result = entry_point(args);
	D("[EP3 BIN]: Returned status: %d\n", result);

	if (addr == NULL_ADDR) {
		if (free_it) {
			EP3_Memory_Free(load_addr);
		}
	}

	return result;
}
