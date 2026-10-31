#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_Logger.h>

// EXL, 31-Oct-2026: Test shared library #3 for the EP3 .so loader: calls a function
//   from lib1.so (SO -> SO call, DT_NEEDED lib1.so at link time) and logs the result.

// EXL, 31-Oct-2026: lib1.so export, resolved at runtime (DT_NEEDED lib1.so).
extern int mylib_add(int a, int b);

int lib3_call(int a, int b) {
	int result = mylib_add(a, b);

	EP3_Logger("[Lib3]: lib3_call(%d, %d) = %d via lib1.so\n", a, b, result);

	return result;
}
