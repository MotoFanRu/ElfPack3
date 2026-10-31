#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_Logger.h>

// EXL, 31-Oct-2026: Test shared library #2 for the EP3 .so loader: has a constructor
//   (DT_INIT_ARRAY) and reads the firmware data symbol GLOBAL_VARIABLE (imported from
//   the DEF library, "0xA0400000 D GLOBAL_VARIABLE" in ep3.def, resolved as GLOB_DAT).

// EXL, 31-Oct-2026: Firmware data symbol, resolved from the DEF library at runtime.
extern UINT32 GLOBAL_VARIABLE;

static UINT32 lib2_value_at_init = 0;

// EXL, 31-Oct-2026: Constructor runs after relocation, so the import is already resolved.
static void __attribute__((constructor)) lib2_init(void) {
	lib2_value_at_init = GET_DATA_ADDR(GLOBAL_VARIABLE);

	EP3_Logger("[Lib2]: Constructor: GLOBAL_VARIABLE = 0x%08X\n", lib2_value_at_init);
}

UINT32 lib2_get_value(void) {
	EP3_Logger("[Lib2]: lib2_get_value: GLOBAL_VARIABLE = 0x%08X\n", GET_DATA_ADDR(GLOBAL_VARIABLE));

	return GET_DATA_ADDR(GLOBAL_VARIABLE);
}

UINT32 lib2_get_value_at_init(void) {
	return lib2_value_at_init;
}
