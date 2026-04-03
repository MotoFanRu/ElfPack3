#ifndef P2K_SDK_LOGGER_H
#define P2K_SDK_LOGGER_H

#include "P2K_EP3_Logger.h"
#include "P2K_EP3_APP_Viewer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define P2K_LOG(format, ...) \
	do { \
		EP3_Logger("%s:%d: " format, __func__, __LINE__, ##__VA_ARGS__); \
	} while (0)

#define P2K_LOG_VIEW(title, format, ...) \
	do { \
		EP3_APP_View(title, "%s:%d: " format, __func__, __LINE__, ##__VA_ARGS__); \
	} while (0)

#define L(format, ...)                 EP3_Logger(format, ##__VA_ARGS__)
#define V(title, format, ...)          EP3_APP_View(title, format, ##__VA_ARGS__)
#define LU(message)                    EP3_Logger_Unicode(message)

#if defined(FTR_DEBUG)
	#define D(format, ...)             P2K_LOG(format, ##__VA_ARGS__)
	#define DU(message)                EP3_Logger_Unicode(message)
	#define DV(title, format, ...)     P2K_LOG_VIEW(title, format, ##__VA_ARGS__)
	#define P()                        D("%s\n", "Debug!")
#else
	#define D(format, ...)             do { } while (0)
	#define DU(message)                do { } while (0)
	#define DV(title, format, ...)     do { } while (0)
	#define P()                        do { } while (0)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_LOGGER_H */
