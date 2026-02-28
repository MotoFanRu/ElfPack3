#include <P2K_Logger.h>

#include <P2K_EP3_Base.h>
#include <P2K_J2ME_System.h>

void EP3_Logger(const char *format, ...) {
#if defined(FTR_LOGGER_JAL)
	// How to call EP3_JAL_Logger()?
#endif
#if defined(FTR_LOGGER_SUAPI)
	// How to call EP3_SUAPI_Logger()?
#endif
#if defined(FTR_LOGGER_FILE)
	// How to call EP3_FILE_Logger()?
#endif
}

static void EP3_JAL_Logger(const char *format, ...) {
	/* Skip NULLed strings and empty strings. */
	if (format && *format) {
		char buffer[LOG_BUFFER_SIZE];
		va_list args;
		va_start(args, format);
		vsprintf(buffer, format, args);
		va_end(args);

		PFprintf("%s", buffer);
	}
}

void EP3_SUAPI_Logger(const char *format, ...) {

}

void EP3_FILE_Logger(const char WCHAR *file_path, const char *format, ...) {

}
