#ifndef P2K_EP3_TASK_REACTOR_H
#define P2K_EP3_TASK_REACTOR_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define REACTOR_PORT                                       (0xFEED)
#define REACTOR_MSG_TYPE_GENERAL                           (0)
#define REACTOR_PORT_NAME                                  "p/reactor"

typedef void (*EP3_REACTOR_ROUTINE_T)(void);

extern void EP3_Reactor_Task(void);

extern void EP3_Send_To_Reactor(UINTPTR function_routine_address);

extern RAW_DATA UINTPTR *reactor_core_port;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_TASK_REACTOR_H */
