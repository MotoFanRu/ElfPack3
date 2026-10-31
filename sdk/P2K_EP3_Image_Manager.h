#ifndef P2K_EP3_IMAGE_MANAGER_H
#define P2K_EP3_IMAGE_MANAGER_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 31-Oct-2026: Unified registry of running images (ELFs, BINs and future .so libraries).
//   Tracks load address + size so any image can be found and unloaded by address,
//   similar to "elfList" in EP2/EM2 loaders, but as a fixed-slot array (no AFW dependency).

#define EP3_IMG_MAX_SLOTS              (16)
#define EP3_IMG_DEFAULT_SLOTS          (8)

#define EP3_IMG_FLAG_FREE_MEM          (1 << 0)  // Free "mem_base" on unregister.

typedef enum tagEP3_Image_Type {
	EP3_IMG_ELF = 0,
	EP3_IMG_BIN,
	EP3_IMG_SO                            // Reserved for future .so shared libraries.
} EP3_Image_Type_T;

typedef struct tagEP3_Image {
	EP3_Image_Type_T type;
	void *mem_base;                       // Physical base of loaded image.
	UINT32 mem_size;                      // Size of loaded image.
	WCHAR *uri;                           // Heap copy of the source file path.
	UINT32 event;                         // Event passed to the ELF entry ("reserve" slot).
	UINT32 flags;
} EP3_Image;

// EXL, 31-Oct-2026: Allocate the image registry from the heap with "max_slots" entries
//   (clamped to EP3_IMG_MAX_SLOTS). Must be called once from the RAM-resident owner
//   (the ELF loader) before any registration. Without Init the manager stays passive:
//   Register() just logs and returns NULL, so the module is safe to link into RO binaries.
extern void EP3_Image_Init(UINT32 max_slots);

// EXL, 31-Oct-2026: Register a loaded image, returns slot descriptor or NULL when full.
//   "uri" is copied to the heap. Flags control whether "mem_base" is freed on unregister.
extern EP3_Image *EP3_Image_Register(
	EP3_Image_Type_T type, void *mem_base, UINT32 mem_size, const WCHAR *uri, UINT32 event, UINT32 flags
);

// EXL, 31-Oct-2026: Remove the image from the registry, frees "uri" and (per flag) "mem_base".
extern STATUS EP3_Image_Unregister(EP3_Image *image);

// EXL, 31-Oct-2026: Find image containing "addr" (addr >= mem_base && addr < mem_base + mem_size)
//   and unregister it. Does not stop a running image.
extern STATUS EP3_Image_Unload(void *addr);

extern EP3_Image *EP3_Image_Find(void *addr);
extern UINT32 EP3_Image_Count(void);
extern UINT32 EP3_Image_Capacity(void);
extern EP3_Image *EP3_Image_At(UINT32 index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_IMAGE_MANAGER_H */
