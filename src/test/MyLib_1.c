#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_Logger.h>

// EXL, 31-Oct-2026: Test shared library for the EP3 .so loader (build_test.py, -fPIC -shared).
//   Exports mylib_add()/mylib_version(), calls back into the host ELF via ui_notify()
//   (the ELF must be built with --export-dynamic) and into the firmware via EP3_Logger
//   (resolved from the DEF library at runtime).

// EXL, 31-Oct-2026: Host ELF export (SO -> ELF call), resolved at runtime.
extern void ui_notify(int value);

int mylib_add(int a, int b) {
	int result = a + b;

	D("[MyLib]: mylib_add(%d, %d) = %d\n", a, b, result);

	// EXL, 31-Oct-2026: SO -> ELF call, resolved from the host ELF exports.
	ui_notify(result);

	// EXL, 31-Oct-2026: SO -> firmware call, resolved from the DEF library.
	EP3_Logger("[MyLib]: Logger from .so library, result=%d\n", result);

	return result;
}

int mylib_version(void) {
	return 1;
}
