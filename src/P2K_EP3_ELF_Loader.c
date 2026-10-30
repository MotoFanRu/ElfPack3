#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>

#include <P2K_EP3_Memory.h>
#include <P2K_EP3_DEF_Library.h>
#include <P2K_EP3_ELF_Loader.h>

__attribute__((used, section(".text.bin.entry_point")))
STATUS EP3_ELF_Loader_MainRegister(const UINTPTR *args) {
	UNUSED(args);

	L("%s\n", "Hello Moto!");

	DEF_LIB_Symbol *def_library = (DEF_LIB_Symbol *) EP3_Memory_Alloc(sizeof(DEF_LIB_Symbol) * DEF_LIB_MAX_SYM_COUNT);
	if (def_library == NULL) {
		L("Cannot allocate bytes: %d\n", sizeof(DEF_LIB_Symbol) * DEF_LIB_MAX_SYM_COUNT);
		return RESULT_FAIL;
	}

	UINT32 def_cnt = EP3_DEF_Library_Load(def_library);
	// EP3_DEF_Library_Insert(def_library, &def_cnt, FNV1A32("EP3_API_Viewer"), (UINTPTR) EP3_APP_View);
	L("[EP3 ELF]: Symbols: %d\n", def_cnt);

	EP3_ELF_Load(L"h.elf", NULL, def_library, def_cnt, NULL);

	L("%s\n", "End!");

	return RESULT_OK;
}
