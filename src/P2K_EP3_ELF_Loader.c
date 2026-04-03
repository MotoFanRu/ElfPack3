#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_SUAPI.h>

#include <P2K_EP3_APP_Viewer.h>
#include <P2K_EP3_Task_Reactor.h>

static BOOL Send_Message(const char *title, const char *msg, UINT32 arg) {
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

	return V(title, "MotoFan.Ru \nHello Moto! \nArgs: %d %d\n0x%08X \n   %s", arg, arg + 1, arg + 2, msg);
}

static STATUS task_a(UINTPTR *args) {
	SU_RET_STATUS status;

	D("%s 0x%08X\n", "Enter A!", (UINTPTR) args);

	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
		return RESULT_FAIL;
	}

	for (int i = 0; i < 5000; ++i) {
		L("%s %06d\n", "[TASK A]: Hello Moto!", i + 1);

		suSleep(SU_WAIT_1MS, &status);
	}

	suReleaseSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error releasing binary semaphore!\n");
		return RESULT_FAIL;
	}

	suDeleteSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error deleting binary semaphore!\n");
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

static STATUS task_b(UINTPTR *args) {
	SU_RET_STATUS status;

	D("%s 0x%08X\n", "Enter B!", (UINTPTR) args);

	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
		return RESULT_FAIL;
	}

	for (int i = 0; i < 5000; ++i) {
		L("%s %06d\n", "[TASK B]: Hello Moto!", i + 1);

		suSleep(SU_WAIT_1MS, &status);
	}

	suReleaseSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error releasing binary semaphore!\n");
		return RESULT_FAIL;
	}

	suDeleteSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error deleting binary semaphore!\n");
		return RESULT_FAIL;
	}

	return Send_Message("BTask Moto", "TASK B send it!", (UINTPTR) args);
}

static STATUS task_c(UINTPTR *args) {
	SU_RET_STATUS status;

	D("%s 0x%08X\n", "Enter C!", (UINTPTR) args);

	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
		return RESULT_FAIL;
	}

	for (int i = 0; i < 5000; ++i) {
		L("%s %06d\n", "[TASK C]: Hello Moto!", i + 1);

		suSleep(SU_WAIT_1MS, &status);
	}

	suReleaseSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error releasing binary semaphore!\n");
		return RESULT_FAIL;
	}

	suDeleteSem(binary_semaphore, &status);
	if (status != SU_OK) {
		L("%s\n", "Error deleting binary semaphore!\n");
		return RESULT_FAIL;
	}

	return Send_Message("CTasker MotoFan.Ru", "TASK C Now here!", (UINTPTR) args);
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

	EP3_Reactor_Send_To_Core(REACTOR_PORT_NAME_A, (UINTPTR) &task_a, NULL_ADDR);
	EP3_Reactor_Send_To_Core(REACTOR_PORT_NAME_B, (UINTPTR) &task_b, 0xFACE0000);
	EP3_Reactor_Send_To_Core(REACTOR_PORT_NAME_B, (UINTPTR) &task_c, 0x0000BEEF);

	L("%s\n", "End!");

	return RESULT_OK;
}
