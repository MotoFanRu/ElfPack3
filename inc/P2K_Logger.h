#ifndef P2K_SDK_LOGGER_H
#define P2K_SDK_LOGGER_H

#include "P2K_EP3_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define P2K_LOG(format, ...) \
	do { \
		EP3_Logger("%s:%d: " format, __func__, __LINE__, ##__VA_ARGS__); \
	} while (0)

#define L(format, ...)                                     P2K_LOG(format, ##__VA_ARGS__)

#if defined(DEBUG)
	#define D(format, ...)                                 P2K_LOG(format, ##__VA_ARGS__)
	#define P()                                            D("Debug Line!\n")
#else
	#define D(format, ...)                                 do { } while (0)
	#define P()                                            do { } while (0)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_LOGGER_H */
