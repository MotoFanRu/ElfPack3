#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>
#include <P2K_Logger.h>

#include <P2K_EP3_Task_Reactor.h>

static SU_QUEUE_HANDLE EP3_Reactor_Create_Task_Port_Queue(UINT16 port_id, const char *port_name);

__attribute__((used, section(".text.task.a.entry_point")))
void EP3_Reactor_Task_A(void) {
	SU_RET_STATUS status;

	SU_QUEUE_HANDLE queue_handle = EP3_Reactor_Create_Task_Port_Queue(REACTOR_PORT_A, REACTOR_PORT_NAME_A);
	if (queue_handle == NULL) {
		D("[EP3 TAR A]: %s\n", "Failed to create task port queue.");
		return;
	}

	/* Register EP3_API_Reactor_Send_To_Core() as API function. */
	UINTPTR reactor_function = (UINTPTR) &EP3_API_Reactor_Send_To_Core;
	suRegisterName(REACTOR_FUNC_NAME, (UINT32) reactor_function, &status);
	if (status != SU_OK) {
		D("[EP3 TAR A]: Register '%s', '0x%08X' failed, status: '%d'.\n", REACTOR_FUNC_NAME, reactor_function, status);
		return;
	}
	D("[EP3 TAR A]: Registered API function '0x%08X' as '%s' name.\n", reactor_function, REACTOR_FUNC_NAME);

	while (TRUE) {
		/* Endless task loop for EP3_Reactor_Task_A. */
		REACTOR_MSG_T *msg = (REACTOR_MSG_T *) suReceiveMessageFromQueue(queue_handle, SU_WAIT_FOREVER, &status);
		if ((status == SU_OK) && (msg != NULL)) {
			UINTPTR func_addr = msg->func_addr;
			UINTPTR args_addr = msg->args_addr;

			D("[EP3 TAR A]: msg=0x%08X, func_addr=0x%08X, args_addr=0x%08X.\n", (UINTPTR) msg, func_addr, args_addr);

			suDeleteMessage(msg, &status);
			if (status != SU_OK) {
				D("[EP3 TAR A]: Cannot delete message '0x%08X', status: %d.\n", (UINTPTR) msg, status);
			}

			EP3_REACTOR_ROUTINE_T task_routine = ((EP3_REACTOR_ROUTINE_T) func_addr);
			STATUS status = task_routine((UINTPTR *) args_addr);
			D("[EP3 TAR A]: Returned func_addr=0x%08X, status: %d.\n", func_addr, status);
		}
	}
}

__attribute__((used, section(".text.task.b.entry_point")))
void EP3_Reactor_Task_B(void) {
	SU_RET_STATUS status;

	SU_QUEUE_HANDLE queue_handle = EP3_Reactor_Create_Task_Port_Queue(REACTOR_PORT_B, REACTOR_PORT_NAME_B);
	if (queue_handle == NULL) {
		D("[EP3 TAR B]: %s.\n", "Failed to create task port queue");
		return;
	}

	while (TRUE) {
		/* Endless task loop for EP3_Reactor_Task_A. */
		REACTOR_MSG_T *msg = (REACTOR_MSG_T *) suReceiveMessageFromQueue(queue_handle, SU_WAIT_FOREVER, &status);
		if ((status == SU_OK) && (msg != NULL)) {
			UINTPTR func_addr = msg->func_addr;
			UINTPTR args_addr = msg->args_addr;

			D("[EP3 TAR B]: msg=0x%08X, func_addr=0x%08X, args_addr=0x%08X.\n", (UINTPTR) msg, func_addr, args_addr);

			suDeleteMessage(msg, &status);
			if (status != SU_OK) {
				D("[EP3 TAR B]: Cannot delete message '0x%08X', status: %d.\n", (UINTPTR) msg, status);
			}

			EP3_REACTOR_ROUTINE_T task_routine = ((EP3_REACTOR_ROUTINE_T) func_addr);
			STATUS status = task_routine((UINTPTR *) args_addr);
			D("[EP3 TAR B]: Returned func_addr=0x%08X, status: %d.\n", func_addr, status);
		}
	}
}

BOOL EP3_API_Reactor_Send_To_Core(const char *port_name, UINTPTR func_addr, UINTPTR args_addr) {
	if (port_name == NULL) {
		D("[EP3 TAR]: %s\n", "Argument port_name is NULL.");
		return FALSE;
	}
	if (func_addr == NULL_ADDR) {
		D("[EP3 TAR]: %s\n", "Argument func_addr is NULL_ADDR (0x00000000).");
		return FALSE;
	}

	SU_RET_STATUS status;

	SU_PORT_HANDLE port_handle = (SU_PORT_HANDLE) suFindName(port_name, SU_NOWAIT, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to find port '%s', status: '%d'.\n", port_name, status);
		return FALSE;
	}
	if (port_handle == NULL) {
		D("[EP3 TAR]: %s\n", "Variable port_handle is NULL.");
		return FALSE;
	}

	REACTOR_MSG_T *msg = (REACTOR_MSG_T *) suCreateMessage(
		sizeof(REACTOR_MSG_T), REACTOR_MSG_TYPE_GENERAL, port_handle, &status
	);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to create message for '%s' port, status: '%d'.\n", port_name, status);
		return FALSE;
	}

	msg->func_addr = func_addr;
	msg->args_addr = args_addr;

	D(
		"[EP3 TAR]: Send msg to '%s', port=0x%08X, msg=0x%08X, func_addr=0x%08X, args_addr=0x%08X.\n",
		port_name, port_handle, msg, msg->func_addr, msg->args_addr
	);

	suSendMessage(msg, port_handle, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to send message to '%s' port, status: '%d'.\n", port_name, status);

		suDeleteMessage(msg, &status);
		if (status != SU_OK) {
			D("[EP3 TAR]: Cannot delete message '0x%08X', status: %d.\n", (UINTPTR) msg, status);
		}

		return FALSE;
	}

	return TRUE;
}

static SU_QUEUE_HANDLE EP3_Reactor_Create_Task_Port_Queue(UINT16 port_id, const char *port_name) {
	SU_RET_STATUS status;

	SU_QUEUE_HANDLE queue_handle = suCreateQueue(&status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to create queue, status: '%d'.\n", status);
		return NULL;
	}

	SU_PORT_HANDLE port_handle = suCreatePortFromQueue(queue_handle, port_id, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to create port '%d', status: '%d'.\n", port_id, status);
		return NULL;
	}

	suRegisterName(port_name, (UINT32) port_handle, &status);
	if (status != SU_OK) {
		D("[EP3 TAR]: Failed to register port '%s', status: '%d'.\n", port_name, status);
		return NULL;
	}

	D(
		"[EP3 TAR]: Task '0x%04X' '%s' registered, port_handle='0x%08X', queue_handle='0x%08X'.\n",
		port_id, port_name, (UINTPTR) port_handle, (UINTPTR) queue_handle
	);

	return queue_handle;
}
