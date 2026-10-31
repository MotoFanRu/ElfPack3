#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_Image_Manager.h>
#include <P2K_EP3_Config.h>
#include <P2K_EP3_BIN_Loader.h>

STATUS EP3_API_BIN_Load(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL a_bit, BOOL free_it) {
#if defined(FTR_BIN_LDR)
	UNUSED(free_it);
#endif

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

#if !defined(FTR_BIN_LDR)
	/*
	* EXL, 31-Oct-2026: RAM bin loader only — there is no point in the fixed load_address
	*   (IRAM) functionality for the NOR RO flash loader (FTR_BIN_LDR). The per-image
	*   config "app.ldr.cfg" takes precedence over the "addr" argument, if such argument
	*   is present. A missing/zero/misaligned address or a failed save allocation falls
	*   back to the argument, then to the system allocator.
	*/
	UINT8 *saved_block = NULL;
	BOOL fixed_load = FALSE;

	EP3_Image_Cfg_T image_cfg;
	if (EP3_Image_Config_Load(file_path, &image_cfg) == RESULT_OK) {
		if ((image_cfg.load_address != 0) && ((image_cfg.load_address & 0x3) == 0)) {
			load_addr = (UINTPTR *) image_cfg.load_address;
			fixed_load = TRUE;

			if (!image_cfg.leave_it) {
				// EXL, 31-Oct-2026: Save the original block of the image size to the
				//   heap, restored after the binary returns (BIN runs synchronously).
				saved_block = (UINT8 *) EP3_Memory_Alloc(file_size);
				if (saved_block == NULL) {
					L("[EP3 BIN]: Failed to allocate %d bytes for the original block copy.\n", file_size);
					L("[EP3 BIN]: %s\n", "Falling back to the system allocator.");
					load_addr = (UINTPTR *) addr;
					fixed_load = FALSE;
				} else {
					UINT8 *orig = (UINT8 *) load_addr;
					for (UINT32 i = 0; i < file_size; ++i) {
						saved_block[i] = orig[i];
					}
					L("[EP3 BIN]: Saved original block 0x%08X (%d bytes) to 0x%08X.\n", load_addr, file_size, saved_block);
				}
			} else {
				L("[EP3 BIN]: %s\n", "leave_it is set, the original block is not saved.");
			}
		}
	}

	if (!fixed_load && (addr == NULL_ADDR)) {
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
#endif

	DL_FS_COUNT_T elements_read;
	if (DL_FsReadFile(load_addr, file_size, 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		L("[EP3 BIN]: Failed to read '%s' into memory at '0x%08X' address.\n", EP3_ELF_LDR_NAME, load_addr);
		DL_FsCloseFile(file_handle);
#if !defined(FTR_BIN_LDR)
		EP3_Memory_Free(saved_block);
#endif
		return RESULT_FAIL;
	}
	if (elements_read != 1) {
		L("[EP3 BIN]: elements_read=%d, should be 1.\n", elements_read);
		DL_FsCloseFile(file_handle);
#if !defined(FTR_BIN_LDR)
		EP3_Memory_Free(saved_block);
#endif
		return RESULT_FAIL;
	}
	L("[EP3 BIN]: Loaded %d bytes to '0x%08X' memory address.\n", file_size, load_addr);

	DL_FsCloseFile(file_handle);

	/* Disable on BIN_LDR in NOR. */
#if !defined(FTR_BIN_LDR)
	// EXL, 31-Oct-2026: Register the loaded binary in the unified image database before the
	//   jump, so it can be tracked and unloaded by address. "register_it" must be FALSE when
	//   this code runs from a read-only (NOR) context like P2K_EP3_APP_Entry_Main.c — the
	//   database lives in the RAM-resident ELF loader, which owns the registry instance.
	//   Memory is freed on unregister only for allocated images with "free_it" set.
	EP3_Image *image = NULL;
	UINT32 img_flags = (!fixed_load && (addr == NULL_ADDR) && free_it) ? EP3_IMG_FLAG_FREE_MEM : 0;
	image = EP3_Image_Register(
		EP3_IMG_BIN, (void *) load_addr, (UINT32) file_size, file_path, 0, img_flags
	);
#endif

	/* Set Thumb etc. bit if needed (will be ignored on ARM and M-CORE) */
	if (a_bit) {
		load_addr = (UINTPTR *) (((UINT32) load_addr) | 1);
	}

	/* Execute the loaded binary. */
	L("[EP3 BIN]: Jump to '0x%08X' memory address.\n", load_addr);
	EP3_ELF_LDR_ENTRY_POINT_T entry_point = (EP3_ELF_LDR_ENTRY_POINT_T) ((UINTPTR) load_addr);
	STATUS result = entry_point(args);
	D("[EP3 BIN]: Returned status: %d\n", result);

#if !defined(FTR_BIN_LDR)
	// EXL, 31-Oct-2026: A fixed load_address block is not owned by the loader, restore
	//   the saved original content after the synchronous binary returns.
	if (saved_block != NULL) {
		UINT8 *orig = (UINT8 *) (((UINTPTR) load_addr) & ~1UL);
		for (UINT32 i = 0; i < file_size; ++i) {
			orig[i] = saved_block[i];
		}
		L("[EP3 BIN]: Restored original block to 0x%08X.\n", orig);
		EP3_Memory_Free(saved_block);
	}

	/* Disable on BIN_LDR in NOR. */
	// EXL, 31-Oct-2026: Drop the registry entry (and memory, per flags) after return.
	if (image != NULL) {
		EP3_Image_Unregister(image);
	}
#endif

	return result;
}
