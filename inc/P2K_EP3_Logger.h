#ifndef P2K_EP3_LOGGER_H
#define P2K_EP3_LOGGER_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_BUFFER_SIZE                                    (128)
#define LOG_MESSAGE_ID                                     (0x5151)
#define LOG_FILE_PATH                                      (L"/b/elf/ep3.log")

extern void EP3_Logger(const char *format, ...);
extern void EP3_Logger_Unicode(const WCHAR *message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_LOGGER_H */
