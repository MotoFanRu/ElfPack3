#ifndef P2K_EP3_APP_VIEWER_H
#define P2K_EP3_APP_VIEWER_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_VIEWER_FUNC_NAME           "ep3/apv"
#define APP_VIEWER_STARTUP_EVENT       ((AFW_EVENT_CODE_T) 0xA000ACED)
#define VIEWER_TITLE_MAX_LENGTH        (32)
#define VIEWER_MESSAGE_MAX_LENGTH      (128)

/*
 * EXL, 15-Nov-2026:
 *
 * Функция регистрации приложения в P2K/Synergy фреймворке AFW.
 */
extern void EP3_APP_Viewer_Register(void);

/*
 * EXL, 15-Nov-2026:
 *
 * Функция вызова визуального окна с информацией с переменным количеством
 * аргументов, полезная для показа различных сообщений пользователю.
 *
 * Принимает аргументами заголовок окна, формат сообщения и
 * последующие аргументы.
 *
 * Возвращает:
 *     RESULT_OK
 *     RESULT_FAIL           (не удалось отправить ивент)
 *     RESULT_FAIL_WRONG_ARG
 */
extern STATUS EP3_API_View(const char *title, const char *format, ...);
typedef STATUS (* EP3_API_VIEW_T)(const char *title, const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_APP_VIEWER_H */
