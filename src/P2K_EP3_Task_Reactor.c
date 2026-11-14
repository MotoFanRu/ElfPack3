#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>
#include <P2K_Logger.h>

#include <P2K_EP3_Task_Reactor.h>

static SU_QUEUE_HANDLE Create_Task_Port_Queue(
	const UINT16 port_id,
	const char *port_name
) {
	SU_RET_STATUS status;

	SU_QUEUE_HANDLE queue_hdl =
		suCreateQueue(&status);
	if (status != SU_OK) {
		D("[EP3 TAR E]: %s\n", "Failed to create queue");
		D("[EP3 TAR E]: %s\n", "suCreateQueue()");
		D("[EP3 TAR E]:  status=%d\n", status);
		return NULL;
	}

	SU_PORT_HANDLE port_hdl =
		suCreatePortFromQueue(queue_hdl, port_id, &status);
	if (status != SU_OK) {
		D("[EP3 TAR E]: %s\n", "Failed to create port from queue");
		D("[EP3 TAR E]: %s\n", "suCreatePortFromQueue()");
		D("[EP3 TAR E]:  status=%d\n", status);
		D("[EP3 TAR E]:  port_id=0x%04X\n", port_id);
		D("[EP3 TAR E]:  queue_hdl=0x%04X\n", (UINTPTR) queue_hdl);
		return NULL;
	}

	suRegisterName(port_name, (UINT32) port_hdl, &status);
	if (status != SU_OK) {
		D("[EP3 APV E]: %s\n", "Failed to register port name");
		D("[EP3 APV E]: %s\n", "suRegisterName()");
		D("[EP3 TAR E]:  status=%d\n", status);
		D("[EP3 TAR E]:  port_name=%s\n", port_name);
		D("[EP3 TAR E]:  port_hdl=0x%08X\n", (UINTPTR) port_hdl);
		return NULL;
	}
	D("[EP3 TAR D]: %s\n", "Task registered");
	D("[EP3 TAR D]: %s\n", "suRegisterName()");
	D("[EP3 TAR D]:  port_id=0x%04X\n", port_id);
	D("[EP3 TAR D]:  port_name=%s\n", port_name);
	D("[EP3 TAR D]:  port_hdl=0x%08X\n", (UINTPTR) port_hdl);
	D("[EP3 TAR D]:  queue_hdl=0x%08X\n", (UINTPTR) queue_hdl);

	return queue_hdl;
}

__attribute__((used, section(".text.ep3.task_a_entry_point")))
void EP3_Reactor_Task_A(void) {
	SU_RET_STATUS status;

	SU_QUEUE_HANDLE queue_hdl =
		Create_Task_Port_Queue(REACTOR_PORT_A, REACTOR_PORT_NAME_A);
	if (queue_hdl == NULL) {
		D("[EP3 TAR E]: [A]: %s\n", "Failed to create task port queue");
		D("[EP3 TAR E]: [A]: %s\n", "Create_Task_Port_Queue()");
		D("[EP3 TAR E]: [A]:  queue_hdl=%s\n", "NULL");
		return;
	}

	L("[EP3 TAR I]: [A]: %s\n", "Task registered " REACTOR_PORT_NAME_A);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Регистрируем в SUAPI функцию отправки функций в таски на
	 * исполнение как API. Это позволит использовать её глобально
	 * без внесения в библиотеки.
	 */
	const UINTPTR func_addr = (UINTPTR) &EP3_API_Send_To_Reactor_Core;
	suRegisterName(REACTOR_FUNC_NAME, (UINT32) func_addr, &status);
	if (status != SU_OK) {
		D("[EP3 TAR E]: [A]: %s\n", "Cannot register " REACTOR_FUNC_NAME);
		D("[EP3 TAR E]: [A]: %s\n", "suRegisterName()");
		D("[EP3 TAR E]: [A]:  status=%d\n", status);
		D("[EP3 TAR E]: [A]:  func_addr=0x%08X\n", (UINTPTR) func_addr);
		return;
	}
	L("[EP3 TAR I]: [A]: %s\n", "Registered " REACTOR_FUNC_NAME);
	D("[EP3 TAR D]: [A]: %s\n", "suRegisterName()");
	D("[EP3 TAR D]: [A]:  status=%d\n", status);
	D("[EP3 TAR D]: [A]:  func_addr=0x%08X\n", (UINTPTR) func_addr);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Бесконечный цикл для `EP3_Reactor_Task_A()` таска.
	 */
	while (TRUE) {
		REACTOR_MSG_T *msg =
			(REACTOR_MSG_T *) suReceiveMessageFromQueue(
				queue_hdl, SU_WAIT_FOREVER, &status
			);
		if ((status == SU_OK) && (msg != NULL)) {
			const UINTPTR f_addr = msg->func_addr;
			const UINTPTR a_addr = msg->args_addr;

			D("[EP3 TAR D]: [A]: %s\n", "Received message from queue");
			D("[EP3 TAR D]: [A]: %s\n", "suReceiveMessageFromQueue()");
			D("[EP3 TAR D]: [A]:  msg=0x%08X\n", (UINTPTR) msg);
			D("[EP3 TAR D]: [A]:  f_addr=0x%08X\n", (UINTPTR) f_addr);
			D("[EP3 TAR D]: [A]:  a_addr=0x%08X\n", (UINTPTR) a_addr);

			suDeleteMessage(msg, &status);
			if (status != SU_OK) {
				D("[EP3 TAR E]: [A]: %s\n", "Cannot delete message");
				D("[EP3 TAR E]: [A]: %s\n", "suDeleteMessage()");
				D("[EP3 TAR E]: [A]:  status=%d\n", status);
				D("[EP3 TAR E]: [A]:  msg=0x%08X\n", (UINTPTR) msg);
			}

			const EP3_REACTOR_ROUTINE_T task_routine =
				((EP3_REACTOR_ROUTINE_T) f_addr);

			const STATUS res = task_routine((const UINTPTR *) a_addr);
			UNUSED(res);

			D("[EP3 TAR D]: [A]: %s\n", "Returned task routine value");
			D("[EP3 TAR D]: [A]: %s\n", "task_routine()");
			D("[EP3 TAR D]: [A]:  res=%d\n", res);
			D("[EP3 TAR D]: [A]:  f_addr=0x%08X\n", (UINTPTR) f_addr);
		}
	}
}

__attribute__((used, section(".text.ep3.task_b_entry_point")))
void EP3_Reactor_Task_B(void) {
	SU_RET_STATUS status;

	SU_QUEUE_HANDLE queue_hdl =
		Create_Task_Port_Queue(REACTOR_PORT_B, REACTOR_PORT_NAME_B);
	if (queue_hdl == NULL) {
		D("[EP3 TAR E]: [B]: %s\n", "Failed to create task port queue");
		D("[EP3 TAR E]: [B]: %s\n", "Create_Task_Port_Queue()");
		D("[EP3 TAR E]: [B]:  queue_hdl=%s\n", "NULL");
		return;
	}

	L("[EP3 TAR I]: [A]: %s\n", "Task registered " REACTOR_PORT_NAME_B);

	/*
	 * EXL, 15-Nov-2026:
	 *
	 * Бесконечный цикл для `EP3_Reactor_Task_B()` таска.
	 */
	while (TRUE) {
		REACTOR_MSG_T *msg =
			(REACTOR_MSG_T *) suReceiveMessageFromQueue(
				queue_hdl, SU_WAIT_FOREVER, &status
			);
		if ((status == SU_OK) && (msg != NULL)) {
			const UINTPTR f_addr = msg->func_addr;
			const UINTPTR a_addr = msg->args_addr;

			D("[EP3 TAR D]: [B]: %s\n", "Received message from queue");
			D("[EP3 TAR D]: [B]: %s\n", "suReceiveMessageFromQueue()");
			D("[EP3 TAR D]: [B]:  msg=0x%08X\n", (UINTPTR) msg);
			D("[EP3 TAR D]: [B]:  f_addr=0x%08X\n", (UINTPTR) f_addr);
			D("[EP3 TAR D]: [B]:  a_addr=0x%08X\n", (UINTPTR) a_addr);

			suDeleteMessage(msg, &status);
			if (status != SU_OK) {
				D("[EP3 TAR E]: [B]: %s\n", "Cannot delete message");
				D("[EP3 TAR E]: [B]: %s\n", "suDeleteMessage()");
				D("[EP3 TAR E]: [B]:  status=%d\n", status);
				D("[EP3 TAR E]: [B]:  msg=0x%08X\n", (UINTPTR) msg);
			}

			const EP3_REACTOR_ROUTINE_T task_routine =
				((EP3_REACTOR_ROUTINE_T) f_addr);
			const STATUS res = task_routine((const UINTPTR *) a_addr);
			UNUSED(res);

			D("[EP3 TAR D]: [B]: %s\n", "Returned task routine value");
			D("[EP3 TAR D]: [B]: %s\n", "task_routine()");
			D("[EP3 TAR D]: [B]:  res=%d\n", res);
			D("[EP3 TAR D]: [B]:  msg_func_addr=0x%08X\n", (UINTPTR) f_addr);
		}
	}
}

STATUS EP3_API_Send_To_Reactor_Core(const REACTOR_ARGS_T *args) {
	if (EMPTY_STR(args->port_name)) {
		return RESULT_FAIL_WRONG_ARG;
	}
	if (args->func_addr == NULL_ADDR) {
		return RESULT_FAIL_WRONG_ARG;
	}

	SU_RET_STATUS status;

	SU_PORT_HANDLE port_hdl =
		(SU_PORT_HANDLE) suFindName(args->port_name, SU_NOWAIT, &status);
	if (status != SU_OK) {
		D("[EP3 TAR E]: %s\n", "Cannot find port");
		D("[EP3 TAR E]: %s\n", "suFindName()");
		D("[EP3 TAR E]:  status=%d\n", status);
		D("[EP3 TAR E]:  port_name=%s\n", args->port_name);
		return RESULT_FAIL_NOT_FOUND;
	}
	if (port_hdl == NULL) {
		D("[EP3 TAR E]: port_hdl=%s\n", "NULL");
		return RESULT_FAIL_NULL;
	}

	REACTOR_MSG_T *msg = (REACTOR_MSG_T *) suCreateMessage(
		sizeof(REACTOR_MSG_T), REACTOR_MSG_TYPE_GENERAL, port_hdl, &status
	);
	if (status != SU_OK) {
		D("[EP3 TAR E]: %s\n", "Failed to create message");
		D("[EP3 TAR E]: %s\n", "suCreateMessage()");
		D("[EP3 TAR E]:  status=%d\n", status);
		return RESULT_FAIL_CANT_CREATE;
	}

	msg->func_addr = args->func_addr;
	msg->args_addr = args->args_addr;

	suSendMessage(msg, port_hdl, &status);
	if (status != SU_OK) {
		D("[EP3 TAR E]: %s\n", "Failed to send message");
		D("[EP3 TAR E]: %s\n", "suSendMessage()");
		D("[EP3 TAR E]:  status=%d\n", status);
		D("[EP3 TAR E]:  port_name=%s\n", args->port_name);

		suDeleteMessage(msg, &status);
		if (status != SU_OK) {
			D("[EP3 TAR E]: %s\n", "Cannot delete message");
			D("[EP3 TAR E]: %s\n", "suDeleteMessage()");
			D("[EP3 TAR E]:  status=%d\n", status);
			D("[EP3 TAR E]:  msg=0x%08X\n", (UINTPTR) msg);
		}

		return RESULT_FAIL_CANT_SEND;
	}
	D("[EP3 TAR D]: %s\n", "Send message to port");
	D("[EP3 TAR D]: %s\n", "suSendMessage()");
	D("[EP3 TAR D]:  msg=0x%08X\n", (UINTPTR) msg);
	D("[EP3 TAR D]:  func_addr=0x%08X\n", (UINTPTR) msg->func_addr);
	D("[EP3 TAR D]:  args_addr=0x%08X\n", (UINTPTR) msg->args_addr);
	D("[EP3 TAR D]:  port_name=%s\n", args->port_name);
	D("[EP3 TAR D]:  port_hdl=%s\n", (UINTPTR) port_hdl);

	return RESULT_OK;
}
