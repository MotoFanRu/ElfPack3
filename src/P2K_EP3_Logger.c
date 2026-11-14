#include <P2K_SDK_Base.h>
#include <P2K_C_Arg.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_SUAPI.h>
#include <P2K_J2ME_System.h>
#include <P2K_DL_File_System.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Component.h>
#include <P2K_EP3_Logger.h>

void EP3_API_Logger(const char *format, ...) {
	if (EMPTY_STR(format)) {
		return;
	}

	va_list args;
	va_start(args, format);

	char buffer[LOG_BUFFER_SIZE];
	PORTABLE_TINY_vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
	buffer[LOG_BUFFER_SIZE - 1] = ASCII_NULL;

	va_end(args);

#if defined(FTR_LOGGER_JAL)
	{
		PFprintf("%s", buffer);
	}
#endif /* FTR_LOGGER_JAL */

#if defined(FTR_LOGGER_SUAPI)
	{
		const UINT32 size = PORTABLE_strlen(buffer);
		suLogData(SU_PORT_LOG_ALWAYS, LOG_SUAPI_MESSAGE_ID, 1, size, buffer);
	}
#endif /* FTR_LOGGER_SUAPI */

#if defined(FTR_LOGGER_FILE)
	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Осторожнее с записью файла лога на ранних этапах загрузки телефона.
	 * К примеру, при инициализации тасок Task_A и Task_B файловая система
	 * телефона ещё не готова к использованию. Поэтому лучше использовать лог
	 * в файл только для части содержащей загрузчик ELF, когда телефон уже
	 * проинициализировался и работает.
	 */
	{
		const char *configured_path = "" FTR_LOGGER_FILE "";
		const char *file_log_path = (EMPTY_STR(configured_path)) ? EP3_ELF_LDR_LOG_NAME : configured_path;

		WCHAR file_path[PATH_MAX_COMPONENT];
		PORTABLE_u_atou(file_log_path, file_path);
		DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_APPEND_MODE, DL_FS_OWNER_RESERVED);
		if (file_handle != DL_FS_HANDLE_INVALID) {
			DL_FS_COUNT_T elements_written;
			const DL_FS_SIZE_T size = PORTABLE_strlen(buffer);
			DL_FsWriteFile((void *) buffer, size, 1, file_handle, &elements_written);
			DL_FsCloseFile(file_handle);
		}
		/*
		 * EXL, 15-Nov-2026:
		 *
		 * Мы не можем использовать другие логгеры в случае если в файл ничего
		 * не было записано, поскольку они требуют наличия функций, которых
		 * может не быть, поэтому оставляем это как есть.
		 */
	}
#endif /* FTR_LOGGER_FILE */
}

void EP3_API_Logger_Unicode(const WCHAR *message) {
	char buffer[LOG_BUFFER_SIZE];
	PORTABLE_u_utoa(message, buffer);
	EP3_API_Logger("%s\n", buffer);
}
