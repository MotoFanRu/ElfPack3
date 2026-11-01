#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_Image_Manager.h>
#if !defined(FTR_BIN_LDR)
#include <P2K_EP3_Config.h>
#endif
#include <P2K_EP3_BIN_Loader.h>

/*
* EXL, 01-Nov-2026: Image lifetime rules (RAM builds, FTR_BIN_LDR NOR loader is untouched):
*
*   "free_it"   = FALSE (default for resident apps) — the image STAYS registered after
*                 its entry returns: no restore, no free, no unregister. A binary that
*                 registered itself to events keeps working and can be unloaded later
*                 by address. TRUE — the image is torn down right after the entry
*                 returns (restore/free/unregister), like a one-shot script.
*   "leave_it"  (per-image .cfg at a fixed load_address) — 1: load destructively, never
*                 save/restore the original block. 0 (default): save the original block
*                 and restore it when the image is eventually removed. Saving is skipped
*                 for resident images: the block would never be restored while the
*                 resident code is alive.
*   "load_address" (.cfg) takes precedence over the "addr" argument; missing/zero/
*                 misaligned address falls back to the argument, then the allocator.
*   EP3_IMG_FLAG_FREE_MEM is set exactly when this call heap-allocated the image, so a
*                 later EP3_Image_Unload(addr) frees the memory in every case.
*/

STATUS EP3_API_BIN_Load(const WCHAR *file_path, const UINTPTR *args, UINTPTR addr, BOOL a_bit, BOOL free_it) {
#if defined(FTR_BIN_LDR)
	UNUSED(free_it);
#endif
	UNUSED(args);

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

#if !defined(FTR_BIN_LDR)
	// EXL, 01-Nov-2026: Refuse a second instance of an already registered binary.
	//   Skipped in the NOR loader: it has no image database instance.
	if (EP3_Image_Find_By_Uri(file_path, EP3_IMG_BIN) != NULL) {
		L("[EP3 BIN]: %s\n", "This binary is already running, launch refused.");
		return RESULT_FAIL;
	}
#endif

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
	UINT8 *saved_block = NULL;
	BOOL fixed_load = FALSE;
	BOOL mem_owner = FALSE;               // This call heap-allocated the image.
	BOOL resident = !free_it;             // free_it is the inverse of the cfg "leave_it".

	// EXL, 01-Nov-2026: Per-image config "app.ldr.cfg" takes precedence over "addr".
	EP3_Image_Cfg_T image_cfg;
	if (EP3_Image_Config_Load(file_path, &image_cfg) == RESULT_OK) {
		if ((image_cfg.load_address != 0) && ((image_cfg.load_address & 0x3) == 0)) {
			load_addr = (UINTPTR *) image_cfg.load_address;
			fixed_load = TRUE;

			// EXL, 01-Nov-2026: Save the original block only for images that will be
			//   torn down eventually — a resident image would never restore it.
			if (!image_cfg.leave_it && !resident) {
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
			} else if (image_cfg.leave_it) {
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
		mem_owner = TRUE;
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
	// EXL, 01-Nov-2026: Register the image BEFORE the jump so it is tracked from the
	//   first instruction. EP3_IMG_FLAG_FREE_MEM marks heap-allocated images (freed on
	//   unregister); saved_block rides on the record and is restored when the entry is
	//   removed — immediately for ephemeral images, or later by EP3_Image_Unload(addr).
	EP3_Image *image = EP3_Image_Register(
		EP3_IMG_BIN, (void *) load_addr, (UINT32) file_size, file_path, 0,
		mem_owner ? EP3_IMG_FLAG_FREE_MEM : 0
	);
	if (image != NULL) {
		image->saved_copy = saved_block;
	} else {
		EP3_Memory_Free(saved_block);
	}
#endif

	/* Set Thumb etc. bit if needed (will be ignored on ARM and M-CORE) */
	if (a_bit) {
		load_addr = (UINTPTR *) (((UINT32) load_addr) | 1);
	}

	/* Execute the loaded binary. */
	L("[EP3 BIN]: Jump to '0x%08X' memory address.\n", load_addr);

	// EXL, 02-Nov-2026: Build the unified EP3_ImageArgs_T block (BINs get argv=NULL:
	//   the legacy "args" pointer array has no text form). Heap-owned, freed on
	//   unregister, so a resident binary keeps valid pointers. The NOR loader has no
	//   image database instance, so its boot-time block is simply left allocated.
	EP3_ImageArgs_T img_args_fallback;
	memset(&img_args_fallback, 0, sizeof(EP3_ImageArgs_T));
	EP3_ImageArgs_T *img_args = EP3_ImageArgs_Build(file_path, NULL, 0);
	if (img_args == NULL) {
		L("[EP3 BIN]: %s\n", "Failed to build the image arguments.");
		img_args = &img_args_fallback;
	}

#if !defined(FTR_BIN_LDR)
	if (image != NULL) {
		image->args_block = img_args;
	}
#endif

	EP3_ELF_LDR_ENTRY_POINT_T entry_point = (EP3_ELF_LDR_ENTRY_POINT_T) ((UINTPTR) load_addr);
	STATUS result = entry_point(img_args);
	D("[EP3 BIN]: Returned status: %d\n", result);

	/* Disable on BIN_LDR in NOR. */
#if !defined(FTR_BIN_LDR)
	// EXL, 02-Nov-2026: Tear the image down when "free_it" is set OR the entry failed
	//   (failed start). Resident binaries with a successful start stay registered:
	//   their registered apps keep working, the saved block (if any) is restored later
	//   by EP3_Image_Unload(addr).
	if (!resident || (result != (STATUS) RESULT_OK)) {
		if (image != NULL) {
			EP3_Image_Unregister(image);
		} else if (saved_block != NULL) {
			// EXL, 01-Nov-2026: No registry slot — restore manually.
			UINT8 *orig = (UINT8 *) (((UINTPTR) load_addr) & ~1UL);
			for (UINT32 i = 0; i < file_size; ++i) {
				orig[i] = saved_block[i];
			}
			L("[EP3 BIN]: Restored original block to 0x%08X.\n", orig);
			EP3_Memory_Free(saved_block);
		}
	} else {
		L("[EP3 BIN]: %s\n", "Image stays resident (free_it is not set).");
	}
#endif

	return result;
}
