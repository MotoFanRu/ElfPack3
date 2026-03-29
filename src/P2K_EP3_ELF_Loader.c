#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_SUAPI.h>
#include <P2K_J2ME_System.h>

#include <P2K_EP3_Task_Reactor.h>

typedef struct {
	INT16 x;
	INT16 y;
} POINT_T;

typedef struct {
	POINT_T ulc;
	POINT_T lrc;
} RECT_T;

extern BOOL DL_DdWriteDisplayRegion(
		RECT_T *update_region,
		BYTE *data_ptr,
		UINT8 display_target
);

extern BOOL DAL_WriteDisplayRegion(
		RECT_T *update_region,
		BYTE *data_ptr,
		UINT8 display_target
);

static void task_a(void) {
	SU_RET_STATUS status;
	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
	}

	for (int i = 0; i < 10000; ++i) {
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
}

static void task_c(void) {
	SU_RET_STATUS status;
	SU_SEMA_HANDLE binary_semaphore = suCreateBSem(SU_SEM_LOCKED, &status);
	if (status != SU_OK) {
		L("%s\n", "Error creating binary semaphore!\n");
	}

	for (int i = 0; i < 10000; ++i) {
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
}

__attribute__((used, section(".text.bin.entry_point")))
void EP3_ELF_Loader_MainRegister(void) {
	L("%s\n", "Hello Moto!");

	EP3_Send_To_Reactor((UINTPTR) &task_a, TASK_REACTOR_A);
	EP3_Send_To_Reactor((UINTPTR) &task_b, TASK_REACTOR_B);
	EP3_Send_To_Reactor((UINTPTR) &task_c, TASK_REACTOR_A);

#if 0
	sc_lock();
	some_task_loop();
	sc_unlock();
#endif

#if 0
	int stack_size = 30;
	int priority = 1;

	SU_TAG_ARRAY *array = suAllocMem(sizeof(SU_TAG_ARRAY) * 3, NULL);
	array[0].tag = 3;
	array[0].data = (void *) &stack_size;
	array[1].tag = 4;
	array[1].data = (void *) &priority;
	array[2].tag = 0;
	array[2].data = NULL;

	SU_RET_STATUS res;
	SU_TASK_HANDLE task_handle = suCreateTask(&some_task_loop, array, &res);
	if (res != SU_OK) {
		PFprintf("%s Status: %d %d\n", "Error creating task!", res, task_handle);
	}
#endif

#if 0
#if defined(FTR_C330)
	BYTE *data_ptr = suAllocMem(96 * 64, NULL);
	for (int i = 0; i < 96 * 64; i++) {
		data_ptr[i] = 0xFF;
	}

	RECT_T r;
	r.ulc.x = 0;
	r.ulc.y = 0;
	r.lrc.x = 95;
	r.lrc.y = 63;

	while (1) {
		DL_DdWriteDisplayRegion(&r, data_ptr, 0);
	}
#endif

#if defined(FTR_E1)
	BYTE *data_ptr = suAllocMem(176 * 220 * 2, NULL);
	for (int i = 0; i < 176 * 220 * 2; i++) {
		data_ptr[i] = 0xF0;
	}

	RECT_T r;
	r.ulc.x = 0;
	r.ulc.y = 0;
	r.lrc.x = 175;
	r.lrc.y = 219;

	while (1) {
		DAL_WriteDisplayRegion(&r, data_ptr, 0);
	}
#endif

#if defined(FTR_A830)
	BYTE *data_ptr = suAllocMem(96 * 64 * 2, NULL);
	for (int i = 0; i < 96 * 64 * 2; i++) {
		data_ptr[i] = 0x70;
	}

	RECT_T r;
	r.ulc.x = 0;
	r.ulc.y = 0;
	r.lrc.x = 175;
	r.lrc.y = 219;

	while (1) {
		DAL_WriteDisplayRegion(&r, data_ptr, 0);
	}
#endif
#endif

	L("%s\n", "End!");
}
