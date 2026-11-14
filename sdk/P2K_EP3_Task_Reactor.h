#ifndef P2K_EP3_TASK_REACTOR_H
#define P2K_EP3_TASK_REACTOR_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define REACTOR_PORT_A                 (0xFEED)
#define REACTOR_PORT_B                 (0xFACE)
#define REACTOR_FUNC_NAME              "ep3/tar"
#define REACTOR_PORT_NAME_A            "ep3/tar_a"
#define REACTOR_PORT_NAME_B            "ep3/tar_b"
#define REACTOR_MSG_TYPE_GENERAL       (0)

typedef struct tagREACTOR_MSG_T {
	UINTPTR func_addr;
	UINTPTR args_addr;
} REACTOR_MSG_T;

typedef struct tagREACTOR_ARGS_T {
	const char *port_name;
	UINTPTR func_addr;
	UINTPTR args_addr;
} REACTOR_ARGS_T;

/*
 * EXL, 15-Nov-2026:
 *
 * Эти две функции тасок подменяют патчем существующие в прошивке телефона
 * отладочные таски `logger_config_main_task()` и `logger_flusher_task()`,
 * которые отключаются вместе с функциями `suLogData()` и `suLogMessage()`.
 *
 * Приоритеты тасок могут быть выставлены в специальной таблице VRTXmc внутри
 * самой прошивки телефона.
 */
extern void EP3_Reactor_Task_A(void);
extern void EP3_Reactor_Task_B(void);

/*
 * EXL, 15-Nov-2026:
 *
 * Функция прокинутая в API проекта, с помощью которой можно отправить
 * на исполнение любую функцию внутрь одного из двух доступных в постоянном
 * ожидании тасок.
 */
extern STATUS EP3_API_Send_To_Reactor_Core(const REACTOR_ARGS_T *args);
typedef STATUS (* EP3_API_SEND_TO_REACTOR_CORE_T)(const REACTOR_ARGS_T *args);

/*
 * EXL, 15-Nov-2026:
 *
 * Функция, которая отправляется внутрь таска на исполнение должна
 * иметь следующий вид.
 */
typedef STATUS (* EP3_REACTOR_ROUTINE_T)(const UINTPTR *args);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_TASK_REACTOR_H */
