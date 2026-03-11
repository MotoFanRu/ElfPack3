#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_SUAPI.h>
#include <P2K_J2ME_System.h>
#include <P2K_DL_Keypad.h>

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
		UINT8 display_target);

extern BOOL DAL_WriteDisplayRegion(
		RECT_T *update_region,
		BYTE *data_ptr,
		UINT8 display_target);

__attribute__((used, section(".text.entry_point")))
void EP3_ELF_Loader_MainRegister(void) {
	PFprintf("%s\n", "Hello Moto!");

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
		data_ptr[i] = 0xF0;
	}

	RECT_T r;
	r.ulc.x = 0;
	r.ulc.y = 0;
	r.lrc.x = 95;
	r.lrc.y = 63;

	while (1) {
		DAL_WriteDisplayRegion(&r, data_ptr, 0);
	}
#endif

	PFprintf("%s\n", "End!");
}
