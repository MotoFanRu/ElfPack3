#include <P2kBase.h>
#include <P2kSuApi.h>
#include <P2kFileSystem.h>

void EP3_BIN_Loader_Register(void) {
	CALC_Register();

	if (IsNoLoadKeyPressed()) {
		Load_ELF_Loader();
	} else {
		LOG("[EP3 BIN Loader]: NoLoad Key was pressed. Skip loading ELF Loader!\n");
	}
}

static void Load_ELF_Loader(void) {
	FILE *elf_loader = Find_ELF_Loader();
	if (elf_loader != NULL) {
		RESULT result;
		SIZE_T fsize = DL_FsGetFileSize(elf_loader);
		// ALLOC using:
		// FIX_ADDRES IRAM, ETC?
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

static BOOL IsNoLoadKeyPressed(void) {
	// Call DL_KeyKjavaGetKeyState()
	// If phone have no Java implement key port reading here.
}
