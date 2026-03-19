#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>
#include <P2K_Logger.h>

#include <P2K_EP3_BIN_Loader.h>
#include <P2K_EP3_Task_Reactor.h>

__attribute__((used, section(".text.task.entry_point")))
void EP3_Reactor_Task(void) {
	SU_RET_STATUS status;
	SU_QUEUE_HANDLE queue_handle = suCreateQueue(&status);
	if (status != SU_OK) {
		L("[EP3 TAR]: Failed to create queue handle, status: '%d'.\n", status);
		return;
	}

	SU_PORT_HANDLE port_handle = suCreatePortFromQueue(queue_handle, REACTOR_PORT, &status);
	if (status != SU_OK) {
		L("[EP3 TAR]: Failed to create port handle, status: '%d'.\n", status);
		return;
	}

	suRegisterName(REACTOR_PORT_NAME, (UINT32) port_handle, &status);
	if (status != SU_OK) {
		L("[EP3 TAR]: Failed to register port, status: '%d'.\n", status);
		return;
	}

	/* The "0xDEADBEEF D reactor_core_port" global value should be declarated. */
	UINTPTR *global_port_addr = (UINTPTR *) GET_DATA_ADDR(reactor_core_port);
	if (global_port_addr == NULL) {
		L("[EP3 TAR]: %s.\n", "Reactor core port address is NULL");
	}

	L("[EP3 TAR]: Save reactor port handle '0x%08X' to '0x%08X' address.\n", (UINTPTR) port_handle, global_port_addr);
	*global_port_addr = (UINTPTR) port_handle;

#if defined(FTR_USE_TASK_REACTOR)
	EP3_BIN_Loader_MainRegister(void);
#endif

	while (TRUE) {
		/* Endless task loop. */
		UINTPTR *msg = (UINTPTR *) suReceiveMessageFromQueue(queue_handle, SU_WAIT_FOREVER, &status);
		if (msg != NULL) {
			UINTPTR msg_addr = (UINTPTR) *msg;

			D("[EP3 TAR]: Get message '0x%08X' with addr '0x%08X'.\n", (UINTPTR) msg, msg_addr);

			suDeleteMessage(msg, &status);
			EP3_REACTOR_ROUTINE_T task_routine = ((EP3_REACTOR_ROUTINE_T) msg_addr);
			task_routine();
		}
	}
}

__attribute__((used, section(".text.task.reactor")))
void EP3_Send_To_Reactor(UINTPTR function_routine_address) {
	SU_RET_STATUS status;

	SU_PORT_HANDLE port_handle = (SU_PORT_HANDLE) GET_DATA(reactor_core_port);
	D("[EP3 TAR]: Port handle is '0x%08X', routine address is '0x%08X'.\n", port_handle, function_routine_address);

	UINTPTR *msg = (UINTPTR *) suCreateMessage(sizeof(UINTPTR), REACTOR_MSG_TYPE_GENERAL, port_handle, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to create message, status: '%d'.\n", status);
		return;
	}

	*msg = function_routine_address;

	suSendMessage(msg, port_handle, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to send message, status: '%d'.\n", status);
		return;
	}
}
