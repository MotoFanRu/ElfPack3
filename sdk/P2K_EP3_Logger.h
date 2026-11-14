#ifndef P2K_EP3_LOGGER_H
#define P2K_EP3_LOGGER_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_BUFFER_SIZE                (128)
#define LOG_SUAPI_MESSAGE_ID           (0x5151)

/*
 * EXL, 15-Nov-2026:
 *
 * Функция вывода логов, поведение которой можно настроить через макросы:
 *     FTR_LOGGER_JAL
 *     FTR_LOGGER_SUAPI
 *     FTR_LOGGER_FILE
 * Может принимать переменное количество аргументов.
 */
extern void EP3_API_Logger(const char *format, ...);

/*
 * EXL, 15-Nov-2026:
 *
 * Функция вывода юникодных строк в лог.
 *
 * Строка формата должна обязательно содержать '%s'.
 */
extern void EP3_API_Logger_Unicode(const char *format, const WCHAR *message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_LOGGER_H */
