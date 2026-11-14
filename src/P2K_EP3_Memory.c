#include <P2K_SDK_Base.h>
#include <P2K_SUAPI.h>
#include <P2K_UIS_Memory.h>
#include <P2K_J2ME_System.h>

#include <P2K_Logger.h>

#include <P2K_EP3_Memory.h>

void *EP3_Memory_Alloc(UINT32 size) {
#if defined(FTR_ALLOC_SUAPI)
	SU_RET_STATUS status;
	void *address = suAllocMem(size, &status);
	if (status != SU_OK) {
		L("[EP3 MEM E]: %s\n", "Cannot allocate memory");
		L("[EP3 MEM E]: %s\n", "suAllocMem()");
		L("[EP3 MEM E]:  status=%d\n", status);
		L("[EP3 MEM E]:  size=%d\n", size);
		return NULL;
	}
	return address;
#elif defined(FTR_ALLOC_UIS)
	UIS_ALLOCATION_ERROR_T status;
	void *address = uisAllocateMemory(size, &status);
	if (status != UIS_MEMORY_ALLOCATION_SUCCESS) {
		L("[EP3 MEM E]: %s\n", "Cannot allocate memory");
		L("[EP3 MEM E]: %s\n", "uisAllocateMemory()");
		L("[EP3 MEM E]:  status=%d\n", status);
		L("[EP3 MEM E]:  size=%d\n", size);
		return NULL;
	}
	return address;
#elif defined(FTR_ALLOC_J2ME)
	void *address = AmMemAllocPointer((INT32) size);
	if (address == NULL) {
		L("[EP3 MEM E]: %s\n", "Cannot allocate memory");
		L("[EP3 MEM E]: %s\n", "AmMemAllocPointer()");
		L("[EP3 MEM E]:  size=%d\n", size);
	}
	return address;
#else
	#error "Set FTR_ALLOC_SUAPI or FTR_ALLOC_UIS or FTR_ALLOC_J2ME defines!"
#endif /* FTR_ALLOC_SUAPI, FTR_ALLOC_UIS, FTR_ALLOC_J2ME */
}

void EP3_Memory_Free(void *address) {
#if defined(FTR_ALLOC_SUAPI)
	suFreeMem(address);
#elif defined(FTR_ALLOC_UIS)
	uisFreeMemory(address);
#elif defined(FTR_ALLOC_J2ME)
	AmMemFreePointer(address);
#else
	#error "Set FTR_ALLOC_SUAPI or FTR_ALLOC_UIS or FTR_ALLOC_J2ME defines!"
#endif /* FTR_ALLOC_SUAPI, FTR_ALLOC_UIS, FTR_ALLOC_J2ME */
}
