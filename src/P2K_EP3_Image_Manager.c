#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_UIS_Ustring.h>
#include <P2K_UIS_Ustring_Portable.h>
#include <P2K_Logger.h>

#include <P2K_EP3_Memory.h>
#include <P2K_EP3_Image_Manager.h>

// EXL, 31-Oct-2026: Unified registry of running images. The database itself is allocated
//   from the heap by EP3_Image_Init() with the configured slot count (like the DEF library
//   capacity), so no static writable arrays are needed. The two scalars below are only
//   written by Init, therefore the module stays safe to link into read-only (NOR) binaries:
//   there Register() is a no-op which logs and returns NULL.

static EP3_Image *EP3_Images = NULL;
static UINT32 EP3_Images_Slot_Count = 0;

void EP3_Image_Init(UINT32 max_slots) {
	if (max_slots == 0) {
		max_slots = EP3_IMG_DEFAULT_SLOTS;
	}
	if (max_slots > EP3_IMG_MAX_SLOTS) {
		L("[EP3 IMG]: Slots clamped to %d (requested %d).\n", EP3_IMG_MAX_SLOTS, max_slots);
		max_slots = EP3_IMG_MAX_SLOTS;
	}

	// EXL, 31-Oct-2026: Re-init drops the previous table.
	if (EP3_Images != NULL) {
		EP3_Memory_Free(EP3_Images);
		EP3_Images = NULL;
	}

	EP3_Images = (EP3_Image *) EP3_Memory_Alloc(sizeof(EP3_Image) * max_slots);
	if (EP3_Images == NULL) {
		L("[EP3 IMG]: Failed to allocate %d bytes for %d slots.\n", sizeof(EP3_Image) * max_slots, max_slots);
		EP3_Images_Slot_Count = 0;
		return;
	}

	memset(EP3_Images, 0, sizeof(EP3_Image) * max_slots);
	EP3_Images_Slot_Count = max_slots;

	L("[EP3 IMG]: Initialized with %d slots at 0x%08X.\n", EP3_Images_Slot_Count, EP3_Images);
}

EP3_Image *EP3_Image_Register(
	EP3_Image_Type_T type, void *mem_base, UINT32 mem_size, const WCHAR *uri, UINT32 event, UINT32 flags
) {
	if (mem_base == NULL) {
		L("[EP3 IMG]: %s\n", "Argument mem_base is NULL.");
		return NULL;
	}

	// EXL, 31-Oct-2026: No registry (RO context without Init) — registration is impossible.
	if (EP3_Images == NULL) {
		L("[EP3 IMG]: %s\n", "Registry is not initialized, image not registered.");
		return NULL;
	}

	for (UINT32 i = 0; i < EP3_Images_Slot_Count; ++i) {
		EP3_Image *image = &EP3_Images[i];
		if (image->mem_base != NULL) {
			continue;
		}

		image->type = type;
		image->mem_base = mem_base;
		image->mem_size = mem_size;
		image->event = event;
		image->flags = flags;

		if (uri != NULL) {
			UINT32 uri_len = (UINT32) PORTABLE_u_strlen(uri) + 1;
			image->uri = (WCHAR *) EP3_Memory_Alloc(uri_len * sizeof(WCHAR));
			if (image->uri == NULL) {
				L("[EP3 IMG]: Failed to allocate %d bytes for URI.\n", uri_len * sizeof(WCHAR));
				memset(image, 0, sizeof(EP3_Image));
				return NULL;
			}
			for (UINT32 j = 0; j < uri_len; ++j) {
				image->uri[j] = uri[j];
			}
		}

		L("[EP3 IMG]: Registered image #%d type=%d at 0x%08X (%d bytes).\n", i, type, mem_base, mem_size);

		return image;
	}

	L("[EP3 IMG]: Too many loaded images, max %d.\n", EP3_Images_Slot_Count);

	return NULL;
}

STATUS EP3_Image_Unregister(EP3_Image *image) {
	if (image == NULL) {
		L("[EP3 IMG]: %s\n", "Argument image is NULL.");
		return RESULT_FAIL;
	}

	if ((EP3_Images == NULL) || (image < &EP3_Images[0]) || (image >= &EP3_Images[EP3_Images_Slot_Count])) {
		L("[EP3 IMG]: Foreign image pointer: 0x%08X\n", (UINTPTR) image);
		return RESULT_FAIL;
	}
	if (image->mem_base == NULL) {
		L("[EP3 IMG]: %s\n", "Image slot is already empty.");
		return RESULT_FAIL;
	}

	L("[EP3 IMG]: Unregistering image type=%d at 0x%08X (%d bytes).\n", image->type, image->mem_base, image->mem_size);

	if ((image->flags & EP3_IMG_FLAG_FREE_MEM) && (image->mem_base != NULL)) {
		EP3_Memory_Free(image->mem_base);
	}
	if (image->uri != NULL) {
		EP3_Memory_Free(image->uri);
	}

	memset(image, 0, sizeof(EP3_Image));

	return RESULT_OK;
}

EP3_Image *EP3_Image_Find(void *addr) {
	if (addr == NULL) {
		return NULL;
	}

	for (UINT32 i = 0; i < EP3_Images_Slot_Count; ++i) {
		EP3_Image *image = &EP3_Images[i];
		if (image->mem_base == NULL) {
			continue;
		}
		if (((UINTPTR) addr >= (UINTPTR) image->mem_base) &&
			((UINTPTR) addr < ((UINTPTR) image->mem_base + image->mem_size))
		) {
			return image;
		}
	}

	return NULL;
}

STATUS EP3_Image_Unload(void *addr) {
	EP3_Image *image = EP3_Image_Find(addr);
	if (image == NULL) {
		L("[EP3 IMG]: Unknown image address: 0x%08X\n", (UINTPTR) addr);
		return RESULT_FAIL;
	}

	return EP3_Image_Unregister(image);
}

UINT32 EP3_Image_Count(void) {
	UINT32 count = 0;

	for (UINT32 i = 0; i < EP3_Images_Slot_Count; ++i) {
		if (EP3_Images[i].mem_base != NULL) {
			count++;
		}
	}

	return count;
}

EP3_Image *EP3_Image_At(UINT32 index) {
	if (index >= EP3_Images_Slot_Count) {
		return NULL;
	}

	return &EP3_Images[index];
}
