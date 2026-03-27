#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_SUAPI.h>
#include <P2K_UIS_Memory.h>
#include <P2K_J2ME_System.h>

#include <P2K_EP3_Memory.h>

void *EP3_Memory_Alloc(UINT32 size) {
#if defined(FTR_ALLOC_SUAPI)
	{
		SU_RET_STATUS status;
		void *address = suAllocMem(size, &status);
		if (status != SU_OK) {
			L("Cannot allocate %d bytes using suAllocMem, status=%d!\n", size, status);
			return NULL;
		}
		return address;
	}
#elif defined(FTR_ALLOC_UIS)
	{
		UIS_ALLOCATION_ERROR_T status;
		void *address = uisAllocateMemory(size, &status);
		if (status != UIS_MEMORY_ALLOCATION_SUCCESS) {
			L("Cannot allocate %d bytes using uisAllocateMemory, status=%d!\n", size, status);
			return NULL;
		}
		return address;
	}
#elif defined(FTR_ALLOC_J2ME)
	{
		void *address = AmMemAllocPointer((INT32) size);
		if (address == NULL) {
			L("Cannot allocate %d bytes using AmMemAllocPointer!\n", size);
		}
		return address;
	}
#else
	#error "Unknown memory manager! Please set FTR_ALLOC_SUAPI, FTR_ALLOC_UIS, FTR_ALLOC_J2ME, etc."
#endif /* FTR_ALLOC_SUAPI, FTR_ALLOC_UIS, FTR_ALLOC_J2ME */
}

void EP3_Memory_Free(void *address) {
#if defined(FTR_ALLOC_SUAPI)
	{
		suFreeMem(address);
	}
#elif defined(FTR_ALLOC_UIS)
	{
		uisFreeMemory(address);
	}
#elif defined(FTR_ALLOC_J2ME)
	{
		AmMemFreePointer(address);
	}
#else
	#error "Unknown memory manager! Please set FTR_ALLOC_SUAPI, FTR_ALLOC_UIS, FTR_ALLOC_J2ME, etc."
#endif /* FTR_ALLOC_SUAPI, FTR_ALLOC_UIS, FTR_ALLOC_J2ME */
}
