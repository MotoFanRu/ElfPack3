#include <P2K_C_Lib.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring_Portable.h>

#include <P2K_EP3_File_System.h>

/*
 * Resolves and finds the full path to a file by checking various locations in a specific order.
 * Paths are constructed and checked without and then with the "file://" URI scheme.
 *
 * Checks in this order for each URI mode:
 * 1. Directly: file_name (e.g., "file.bin")
 * 2. With leading /: /file_name (e.g., "/file.bin")
 * 3. On disks (b, c, e, g, a) in the given order: /disk/file_name (e.g., "/b/file.bin")
 * 4. On disks in elf dir: /disk/elf/file_name (e.g., "/b/elf/file.bin")
 *
 * If found, leaves the resolved path in out_path and returns TRUE.
 * Returns FALSE if not found anywhere (out_path cleared to empty string).
 *
 * out_path must be provided by the caller with sufficient size.
 * Uses strncat with manual size tracking for safe string construction.
 *
 * file.bin         # 1st check, V60i, C350L variant?
 * /file.bin        # 2nd check V60i, C350L variant?
 * /a/file.bin      # 3rd check, root of any disk.
 * /a/elf/file.bin  # 4th check, elf directory.
 *
 * b, c, a, e, g    # Disk check order.
 *
 * file://          # 5th check, all above again but with URI scheme.
 */

BOOL EP3_Find_Internal_System_Component(const char *file_name, WCHAR *out_path) {
	if (file_name == NULL || out_path == NULL) {
		return FALSE;
	}

	char out_path_str[PATH_MAX_SHORT];

	const char *uri_prefix = "file://";
	const char *disks[] = { "b", "c", "e", "g", "a" };

	BOOL use_uri = FALSE;
	for (use_uri = FALSE; use_uri <= TRUE; ++use_uri) {
		const char *prefix = use_uri ? uri_prefix : "";

		/* Check 1: `file_name` directly. */
		if (snprintf(out_path_str, PATH_MAX_SHORT, "%s%s", prefix, file_name) < PATH_MAX_SHORT) {
			PORTABLE_u_atou(out_path_str, out_path);
			if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
				return TRUE;
			}
		}

		/* Check 2: `/` + `file_name` directly. */
		if (snprintf(out_path_str, PATH_MAX_SHORT, "%s/%s", prefix, file_name) < PATH_MAX_SHORT) {
			PORTABLE_u_atou(out_path_str, out_path);
			if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
				return TRUE;
			}
		}

		UINT8 i;
		for (i = 0; i < ARRAY_SIZE(disks); ++i) {
			/* Check 3: `/disk/` + `file_name` in loop. */
			if (snprintf(out_path_str, PATH_MAX_SHORT, "%s/%s/%s", prefix, disks[i], file_name) < PATH_MAX_SHORT) {
				PORTABLE_u_atou(out_path_str, out_path);
				if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
					return TRUE;
				}
			}

			/* Check 4: `/disk/elf/` + `file_name` in loop. */
			if (snprintf(out_path_str, PATH_MAX_SHORT, "%s/%s/elf/%s", prefix, disks[i], file_name) < PATH_MAX_SHORT) {
				PORTABLE_u_atou(out_path_str, out_path);
				if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
					return TRUE;
				}
			}
		}
	}

	/* Clear `out_path` on failure. */
	out_path[0] = UNICODE_NULL;
	return FALSE;
}
