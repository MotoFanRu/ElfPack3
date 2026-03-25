#include <P2K_SDK_Base.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_SUAPI.h>
#include <P2K_J2ME_System.h>
#include <P2K_DL_File_System.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Logger.h>
#include <P2K_EP3_File_System.h>

void EP3_Logger(const char *format, ...) {
#if defined(FTR_LOGGER)
	/* Skip NULL and empty strings. */
	if (!format || !format[0]) {
		return;
	}

	va_list args;
	va_start(args, format);

#if !defined(FTR_LOGGER_NULL)
	char buffer[LOG_BUFFER_SIZE];

	/* Early M-CORE phones on Patriot and Rainbow SoC have no "vsnprintf" function, so use "vsnprintf" instead. */
#if defined(FTR_NO_VSNPRINTF)
	vsprintf(buffer, format, args);
#else
	vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
#endif /* FTR_NO_VSNPRINTF */

#endif /* !FTR_LOGGER_NULL */

	va_end(args);

#if defined(FTR_LOGGER_JAL)
	{
		PFprintf("%s", buffer);
	}
#endif /* FTR_LOGGER_JAL */

#if defined(FTR_LOGGER_SUAPI)
	{
		UINT32 size = PORTABLE_strlen(buffer);
		suLogData(SU_PORT_LOG_ALWAYS, LOG_MESSAGE_ID, 1, size, buffer);
	}
#endif /* FTR_LOGGER_SUAPI */

#if defined(FTR_LOGGER_FILE)
	{
		const WCHAR *final_path;

#if defined(FTR_LOGGER_FILE_PREDIFINED)
		final_path = LOG_FILE_PATH;
#else
		WCHAR file_path[PATH_MAX_SHORT];
		if (EP3_Find_Internal_System_Component(EP3_ELF_LDR_NAME, file_path)) {
			INT32 length = PORTABLE_u_strlen(file_path);
			file_path[length - 3] = L'l';
			file_path[length - 2] = L'o';
			file_path[length - 1] = L'g';
			final_path = file_path;
		} else {
			final_path = LOG_FILE_PATH;
		}
#endif /* FTR_LOGGER_FILE_PREDIFINED */

		DL_FS_HANDLE_T file_handle = DL_FsOpenFile(final_path, DL_FS_APPEND_MODE, DL_FS_OWNER_RESERVED);
		if (file_handle != DL_FS_HANDLE_INVALID) {
			DL_FS_COUNT_T elements_written;
			DL_FS_SIZE_T size = PORTABLE_strlen(buffer);
			DL_FsWriteFile((void *) buffer, size, 1, file_handle, &elements_written);
			DL_FsCloseFile(file_handle);
		}
	}
#endif /* FTR_LOGGER_FILE */

#endif /* FTR_LOGGER */
}

void EP3_Logger_Unicode(const WCHAR *message) {
	char buffer[LOG_BUFFER_SIZE];
	PORTABLE_u_utoa(message, buffer);
	EP3_Logger("%s\n", buffer);
}
