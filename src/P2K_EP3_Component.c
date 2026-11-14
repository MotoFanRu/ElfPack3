#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_UIS_Ustring.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_DL_File_System.h>

#include <P2K_Logger.h>

#include <P2K_EP3_Component.h>

STATUS EP3_Find_Component(const char *file_name, WCHAR *out_path) {
	if (EMPTY_STR(file_name)) {
		return RESULT_FAIL_WRONG_ARG;
	}
	if (out_path == NULL) {
		return RESULT_FAIL_WRONG_ARG;
	}

	int res;

	char out_path_str[PATH_MAX_COMPONENT];
	out_path[0] = UNICODE_NULL;
	out_path_str[0] = ASCII_NULL;

#if defined(FTR_DISKS_A)   || \
	defined(FTR_DISKS_AB)  || \
	defined(FTR_DISKS_ABC) || \
	defined(FTR_DISKS_ALL)
	/*
	 * EXL, 15-Nov-2026:
	 *
	 * NONE (нет определений)
	 *     Телефоны в которых плоская файловая система не имеют дисков.
	 *     Примеры: C330, C350, V60, V60i, T720i и др.
	 * FTR_DISKS_A
	 *     Телефоны в которых есть лишь один диск /a/ и нет карты памяти.
	 *     Примеры: C380, C650, V600 и др.
	 * FTR_DISKS_AB
	 *     Телефоны в которых есть диск /a/ и /b/ карта памяти.
	 *     Примеры: E398 и др.
	 * FTR_DISKS_ABC
	 *     Телефоны в которых есть диск /a/, /c/ и /b/ карта памяти.
	 *     Примеры: ROKR E1, SLVR L7 и др.
	 * FTR_DISKS_ALL
	 *     Телефоны в которых плоская файловая система не имеют дисков.
	 *     Примеры: RAZR V3x, KRZR K3 и др.
	 */
	#if defined(FTR_DISKS_A)
		const char *disks[] = { "a" };
	#elif defined(FTR_DISKS_AB)
		const char *disks[] = { "b", "a" };
	#elif defined(FTR_DISKS_ABC)
		const char *disks[] = { "b", "c", "a" };
	#elif defined(FTR_DISKS_ALL)
		const char *disks[] = { "b", "c", "e", "g", "a" };
	#endif

		for (UINT32 i = 0; i < ARRAY_SIZE(disks); ++i) {
			/*
			 * EXL, 15-Nov-2026:
			 *
			 * Шаг 1: Проверка /disk/ + file_name в цикле.
			 */
			res = snprintf(
				out_path_str,
				PATH_MAX_COMPONENT,
				"/%s/%s",
				disks[i],
				file_name
			);
			if (res < PATH_MAX_COMPONENT) {
				D("[EP3 CMP D]: [1]: Trying: %s\n", out_path_str);
				PORTABLE_u_atou(out_path_str, out_path);
				if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
					D("[EP3 CMP D]: [1]:  Found: %s\n", out_path_str);
					return RESULT_OK;
				}
			}

			/*
			 * EXL, 15-Nov-2026:
			 *
			 * Шаг 2: Проверка /disk/elf/ + file_name в цикле.
			 */
			res = snprintf(
				out_path_str,
				PATH_MAX_COMPONENT,
				"/%s/elf/%s",
				disks[i],
				file_name
			);
			if (res < PATH_MAX_COMPONENT) {
				D("[EP3 CMP D]: [2]: Trying: %s\n", out_path_str);
				PORTABLE_u_atou(out_path_str, out_path);
				if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
					D("[EP3 CMP D]: [2]:  Found: %s\n", out_path_str);
					return RESULT_OK;
				}
			}
		}
#endif /* FTR_DISKS_A || FTR_DISKS_AB || FTR_DISKS_ABC || FTR_DISKS_ALL */

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Шаг 3: Проверка file_name напрямую.
	 */
	res = snprintf(out_path_str, PATH_MAX_COMPONENT, "%s", file_name);
	if (res < PATH_MAX_COMPONENT) {
		D("[EP3 CMP D]: [3]: Trying: %s\n", out_path_str);
		PORTABLE_u_atou(out_path_str, out_path);
		if (DL_FsSGetFileSize(out_path, DL_FS_OWNER_RESERVED) > 0) {
			D("[EP3 CMP D]: [3]:  Found: %s\n", out_path_str);
			return RESULT_OK;
		}
	}

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Шаг 4: Файл не найден. Очистка аргументов на выходе.
	 */
	out_path[0] = UNICODE_NULL;
	D("[EP3 CMP D]: [4]: Not found: %s\n", file_name);
	return RESULT_FAIL_FILE_NOT_FOUND;
}
