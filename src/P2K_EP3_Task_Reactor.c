#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>
#include <P2K_Logger.h>

#include <P2K_EP3_BIN_Loader.h>
#include <P2K_EP3_Task_Reactor.h>

__attribute__((used, section(".text.task.a.entry_point")))
void EP3_Reactor_Task_A(void) {
	SU_RET_STATUS status;
	SU_QUEUE_HANDLE queue_handle = suCreateQueue(&status);
	if (status != SU_OK) {
		L("[EP3 TAR A]: Failed to create queue handle, status: '%d'.\n", status);
		return;
	}

	SU_PORT_HANDLE port_handle = suCreatePortFromQueue(queue_handle, REACTOR_PORT_A, &status);
	if (status != SU_OK) {
		L("[EP3 TAR A]: Failed to create port handle, status: '%d'.\n", status);
		return;
	}

	suRegisterName(REACTOR_PORT_NAME_A, (UINT32) port_handle, &status);
	if (status != SU_OK) {
		L("[EP3 TAR A]: Failed to register port, status: '%d'.\n", status);
		return;
	}

	/* The "0x???????? D reactor_core_port_a" global value should be declarated. */
	UINTPTR *global_port_addr = (UINTPTR *) GET_DATA_ADDR(reactor_core_port_a);
	if (global_port_addr == NULL) {
		L("[EP3 TAR A]: %s.\n", "Reactor core port address is NULL");
	}

	L("[EP3 TAR A]: Save reactor port handle '0x%08X' to '0x%08X' address.\n", (UINTPTR) port_handle, global_port_addr);
	*global_port_addr = (UINTPTR) port_handle;

#if defined(FTR_USE_TASK_REACTOR)
	EP3_BIN_Loader_MainRegister(void);
#endif

	while (TRUE) {
		/* Endless task loop for EP3_Reactor_Task_A. */
		UINTPTR *msg = (UINTPTR *) suReceiveMessageFromQueue(queue_handle, SU_WAIT_FOREVER, &status);
		if (msg != NULL) {
			UINTPTR msg_addr = (UINTPTR) *msg;
			D("[EP3 TAR A]: Get message '0x%08X' with addr '0x%08X'.\n", (UINTPTR) msg, msg_addr);
			suDeleteMessage(msg, &status);

			EP3_REACTOR_ROUTINE_T task_routine = ((EP3_REACTOR_ROUTINE_T) msg_addr);
			task_routine();
		}
	}
}

__attribute__((used, section(".text.task.b.entry_point")))
void EP3_Reactor_Task_B(void) {
	SU_RET_STATUS status;
	SU_QUEUE_HANDLE queue_handle = suCreateQueue(&status);
	if (status != SU_OK) {
		L("[EP3 TAR B]: Failed to create queue handle, status: '%d'.\n", status);
		return;
	}

	SU_PORT_HANDLE port_handle = suCreatePortFromQueue(queue_handle, REACTOR_PORT_B, &status);
	if (status != SU_OK) {
		L("[EP3 TAR B]: Failed to create port handle, status: '%d'.\n", status);
		return;
	}

	suRegisterName(REACTOR_PORT_NAME_B, (UINT32) port_handle, &status);
	if (status != SU_OK) {
		L("[EP3 TAR B]: Failed to register port, status: '%d'.\n", status);
		return;
	}

	/* The "0x???????? D reactor_core_port_b" global value should be declarated. */
	UINTPTR *global_port_addr = (UINTPTR *) GET_DATA_ADDR(reactor_core_port_b);
	if (global_port_addr == NULL) {
		L("[EP3 TAR B]: %s.\n", "Reactor core port address is NULL");
	}

	L("[EP3 TAR B]: Save reactor port handle '0x%08X' to '0x%08X' address.\n", (UINTPTR) port_handle, global_port_addr);
	*global_port_addr = (UINTPTR) port_handle;

	while (TRUE) {
		/* Endless task loop for EP3_Reactor_Task_B. */
		UINTPTR *msg = (UINTPTR *) suReceiveMessageFromQueue(queue_handle, SU_WAIT_FOREVER, &status);
		if (msg != NULL) {
			UINTPTR msg_addr = (UINTPTR) *msg;
			D("[EP3 TAR B]: Get message '0x%08X' with addr '0x%08X'.\n", (UINTPTR) msg, msg_addr);
			suDeleteMessage(msg, &status);

			EP3_REACTOR_ROUTINE_T task_routine = ((EP3_REACTOR_ROUTINE_T) msg_addr);
			task_routine();
		}
	}
}

__attribute__((used, section(".text.tasks.reactor")))
BOOL EP3_Send_To_Reactor(UINTPTR function_routine_address, TASK_REACTOR_T reactor) {
	SU_RET_STATUS status;

	SU_PORT_HANDLE port_handle = NULL;
	switch (reactor) {
		case TASK_REACTOR_A:
			port_handle = (SU_PORT_HANDLE) GET_DATA(reactor_core_port_a);
			break;
		case TASK_REACTOR_B:
			port_handle = (SU_PORT_HANDLE) GET_DATA(reactor_core_port_b);
			break;
		default:
			L("[EP3 TAR]: Unknown reactor task: '0x%02X'\n", reactor);
			break;
	}
	if (port_handle == NULL) {
		L("[EP3 TAR]: Port handle of '0x%02X' reactor task is NULL!\n", reactor);
		return FALSE;
	}

	D(
		"[EP3 TAR]: Reactor ID, Port Handle, Routine Addr: '0x%02X', '0x%08X', '0x%08X'.\n",
		reactor, port_handle, function_routine_address
	);

	UINTPTR *msg = (UINTPTR *) suCreateMessage(sizeof(UINTPTR), REACTOR_MSG_TYPE_GENERAL, port_handle, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to create message, status: '%d'.\n", status);
		return FALSE;
	}

	*msg = function_routine_address;

	suSendMessage(msg, port_handle, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to send message, status: '%d'.\n", status);
		return FALSE;
	}

	return TRUE;
}
