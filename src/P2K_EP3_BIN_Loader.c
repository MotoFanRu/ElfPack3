#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>
#include <P2K_DL_FileSystem.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_BIN_Loader.h>

static BOOL IsNoLoadKeyPressed(void);

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

	} else {
		LOG("[EP3 BIN Loader]: No Load Key was pressed. Disable EP3 loading!\n")
	}
}

static BOOL IsNoLoadKeyPressed(void) {
	// Call DL_KeyKjavaGetKeyState()
	// If phone have no Java implement key port reading here.
	return FALSE;
}

static BOOL Load_EP3_ELF_Loader_as_BIN(void) {
	const W_CHAR *ep3_elf_ldr_path = NULL;
	if (ep3_elf_ldr_path == NULL) {
		ep3_elf_ldr_path = Find_EP3_ELF_Loader_File("");
	}
	if (ep3_elf_ldr_path == NULL) {
		ep3_elf_ldr_path = Find_EP3_ELF_Loader_File("");
	}
}

static W_CHAR *Find_EP3_ELF_Loader_File(void) {
	const W_CHAR *ep3_elf_ldr_path = Find_EP3_ELF_Loader_File_On_Root();
	if (ep3_elf_ldr_path == NULL) {
		ep3_elf_ldr_path = Find_EP3_ELF_Loader_File_On_Disk("");
	}
}

static W_CHAR *Find_EP3_ELF_Loader_File_On_Root(void) {
	W_CHAR path[MAX_PATH];

}

static W_CHAR *Find_EP3_ELF_Loader_File_On_Disk(const W_CHAR *path) {
	if (path == NULL) {
		return NULL;
	}

}






















static void Load_ELF_Loader(void) {
	FILE *elf_loader = Find_ELF_Loader();
	if (elf_loader != NULL) {
		RESULT result;
		SIZE_T fsize = DL_FsGetFileSize(elf_loader);
		// ALLOC using:
		// FIXED_ADDRESS IN IRAM, ETC?
		// SUAPI
		// uisAllocate
		// JAVA ALLOC?
		allocated_in_ram = suAllocMem(fsize, &result);
		if (allocated_in_ram != NULL && (result == RESULT_OK)) {
			allocated_in_ram();
		} else {
			LOG("[EP3 BIN Loader]: Cannot allocate memory!\n");
		}
	} else {
		LOG("[EP3 BIN Loader]: Cannot find ELF loader file!\n");
	}
}

static FILE* Find_ELF_Loader(void) {
	// Check this:
	//
	// /elf.ldr (FlatFS variant, V60i/C350L)
	// /a/elf.ldr
	// /a/Elf/elf.ldr
	//
	// Disks:
	// /a/
	// /b/
	// /c/
	// /e/
	// /g/
	//
	// Variants w/ file:// scheme.
}

