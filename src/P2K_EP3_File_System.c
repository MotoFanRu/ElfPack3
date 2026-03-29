#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring_Portable.h>

#include <P2K_EP3_File_System.h>

BOOL EP3_Find_Internal_System_Component(const char *file_name, WCHAR *out_path) {
	if (file_name == NULL || out_path == NULL) {
		return FALSE;
	}

	char out_path_str[PATH_MAX_SHORT];
	const char *disks[] = { "b", "c", "e", "g", "a" };

	out_path_str[0] = ASCII_NULL;
	out_path[0] = UNICODE_NULL;

	for (int i = 0; i < ARRAY_SIZE(disks); ++i) {
		/* Check 1: `/disk/` + `file_name` in loop. */
		if (snprintf(out_path_str, PATH_MAX_SHORT, "/%s/%s", disks[i], file_name) < PATH_MAX_SHORT) {
			D("Will check: %s\n", out_path_str);
			PORTABLE_u_atou(out_path_str, out_path);
			if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
				D("Found: %s\n", out_path_str);
				return TRUE;
			}
		}

		/* Check 2: `/disk/elf/` + `file_name` in loop. */
		if (snprintf(out_path_str, PATH_MAX_SHORT, "/%s/elf/%s", disks[i], file_name) < PATH_MAX_SHORT) {
			D("Will check: %s\n", out_path_str);
			PORTABLE_u_atou(out_path_str, out_path);
			if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
				D("Found: %s\n", out_path_str);
				return TRUE;
			}
		}
	}

	/* Check 3: `file_name` directly. */
	if (snprintf(out_path_str, PATH_MAX_SHORT, "%s", file_name) < PATH_MAX_SHORT) {
		D("Will check: %s\n", out_path_str);
		PORTABLE_u_atou(out_path_str, out_path);
		if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
			D("Found: %s\n", out_path_str);
			return TRUE;
		}
	}

	/* Clear `out_path` on failure. */
	out_path[0] = UNICODE_NULL;
	D("Not found: %s\n", file_name);
	return FALSE;
}
