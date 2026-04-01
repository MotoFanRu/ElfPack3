#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_SUAPI.h>

#include <P2K_EP3_APP_Viewer.h>
#include <P2K_EP3_Task_Reactor.h>

static BOOL Send_Message(const char *msg, int arg) {
	SU_RET_STATUS status;

	EP3_APP_VIEW_T EP3_APP_View = (EP3_APP_VIEW_T) suFindName(VIEWER_FUNC_NAME, SU_NOWAIT, &status);
	if (status != SU_OK) {
		D("[EP3 ELF]: Failed to find func '%s', status: '%d'.\n", VIEWER_FUNC_NAME, status);
		return RESULT_FAIL;
	}
	if (EP3_APP_View == NULL) {
		D("[EP3 ELF]: %s\n", "Function pointer 'EP3_APP_View' is NULL!");
		return RESULT_FAIL;
	}

	return EP3_APP_View("MotoFan.Ru \nHello Moto! \nArgs: %d %d\n0x%08X \n   %s", arg, arg + 1, arg + 2, msg);
}

static void task_a(void) {
	SU_RET_STATUS status;
	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
	}

	for (int i = 0; i < 1000; ++i) {
		L("%s %06d\n", "[TASK A]: Hello Moto!", i + 1);

		suSleep(SU_WAIT_1MS, &status);
	}

	suReleaseSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error releasing binary semaphore!\n");
	}

	suDeleteSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error deleting binary semaphore!\n");
	}
}

static void task_b(void) {
	SU_RET_STATUS status;
	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
	}

	for (int i = 0; i < 10000; ++i) {
		L("%s %06d\n", "[TASK B]: Hello Moto!", i + 1);

		suSleep(SU_WAIT_1MS, &status);
	}

	suReleaseSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error releasing binary semaphore!\n");
	}

	suDeleteSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error deleting binary semaphore!\n");
	}

	Send_Message("TASK B send it!", 4000);
}

static void task_c(void) {
	SU_RET_STATUS status;
	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
	}

	for (int i = 0; i < 1000; ++i) {
		L("%s %06d\n", "[TASK C]: Hello Moto!", i + 1);

		suSleep(SU_WAIT_1MS, &status);
	}

	suReleaseSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error releasing binary semaphore!\n");
	}

	suDeleteSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error deleting binary semaphore!\n");
	}

	Send_Message("TASK C Now here!", 3);
}

__attribute__((used, section(".text.bin.entry_point")))
STATUS EP3_ELF_Loader_MainRegister(const UINTPTR *args) {
	UNUSED(args);

	SU_RET_STATUS status;

	L("%s\n", "Hello Moto!");

	EP3_REACTOR_SEND_TO_CORE_T EP3_Reactor_Send_To_Core;
	EP3_Reactor_Send_To_Core = (EP3_REACTOR_SEND_TO_CORE_T) suFindName(REACTOR_FUNC_NAME, SU_NOWAIT, &status);
	if (status != SU_OK) {
		D("[EP3 ELF]: Failed to find func '%s', status: '%d'.\n", REACTOR_FUNC_NAME, status);
		return RESULT_FAIL;
	}
	if (EP3_Reactor_Send_To_Core == NULL) {
		D("[EP3 ELF]: %s\n", "Function pointer 'EP3_Reactor_Send_To_Core' is NULL!");
		return RESULT_FAIL;
	}

	EP3_Reactor_Send_To_Core(REACTOR_PORT_NAME_A, (UINTPTR) &task_a);
	EP3_Reactor_Send_To_Core(REACTOR_PORT_NAME_B, (UINTPTR) &task_b);
	EP3_Reactor_Send_To_Core(REACTOR_PORT_NAME_B, (UINTPTR) &task_c);

	L("%s\n", "End!");

	return RESULT_OK;
}
