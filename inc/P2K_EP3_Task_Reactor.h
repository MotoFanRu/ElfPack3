#ifndef P2K_EP3_TASK_REACTOR_H
#define P2K_EP3_TASK_REACTOR_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define REACTOR_PORT_A                 (0xFEED)
#define REACTOR_PORT_B                 (0xFACE)
#define REACTOR_MSG_TYPE_GENERAL       (0)
#define REACTOR_PORT_NAME_A            "p/tar_a"
#define REACTOR_PORT_NAME_B            "p/tar_b"

typedef enum tagTASK_REACTOR_T {
	TASK_REACTOR_A = 0,
	TASK_REACTOR_B,
} TASK_REACTOR_T;

typedef void (*EP3_REACTOR_ROUTINE_T)(void);

extern void EP3_Reactor_Task_A(void);
extern void EP3_Reactor_Task_B(void);

extern BOOL EP3_Send_To_Reactor(UINTPTR function_routine_address, TASK_REACTOR_T reactor);

extern RAW_DATA UINTPTR *reactor_core_port_a;
extern RAW_DATA UINTPTR *reactor_core_port_b;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_TASK_REACTOR_H */
