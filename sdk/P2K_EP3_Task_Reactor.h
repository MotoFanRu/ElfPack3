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

typedef STATUS (* EP3_REACTOR_ROUTINE_T)(UINTPTR *args);

extern void EP3_Reactor_Task_A(void);
extern void EP3_Reactor_Task_B(void);

typedef BOOL (* EP3_REACTOR_SEND_TO_CORE_T)(const char *port_name, UINTPTR func_addr, UINTPTR args_addr);
extern BOOL EP3_API_Reactor_Send_To_Core(const char *port_name, UINTPTR func_addr, UINTPTR args_addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_TASK_REACTOR_H */
