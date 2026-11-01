#ifndef P2K_EP3_IMAGE_ARGS_H
#define P2K_EP3_IMAGE_ARGS_H

#include "P2K_SDK_Base.h"
#include "P2K_EP3_Image_Manager.h"
#include "P2K_EP3_Environment.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 02-Nov-2026: Entry point arguments for every executable image (ELF, BIN/.ldr).
//   A single EP3_ImageArgs_T is passed to the "EP3_Main" entry, synthesizing the
//   argument styles of the previous loader generations: ep1 (uri, params, reserve),
//   ep2 (WCHAR uri/params + elf->dir), em2 (self struct passed in), sie (switab API).
//
//   The whole block (this struct, argv/envp arrays and all strings) lives in one heap
//   allocation owned by the Image Manager record of the image — it is freed when the
//   image is unregistered, so pointers stay valid for the whole image lifetime
//   (including asynchronous handlers started from EP3_Main).
//
//   argc/argv: argv[0] is the image path, argv[1..] are the tokens of the "args"
//   string from ep3.cfg autorun, argv[argc] is NULL.
//   envp: NULL-terminated WCHAR array of "NAME=value" strings (a snapshot of the
//   shared environment at load time; live access goes through api->getenv).
//
//   Entry result: RESULT_OK keeps the image; anything else makes the loader unload
//   the image and free it immediately (failed start).

typedef struct tagEP3_ImageArgs {
	const WCHAR *path;                    // Source file URI of this image.
	UINT32 argc;                          // argv[0] = path, argv[argc] = NULL.
	const WCHAR **argv;
	const WCHAR **envp;                   // NULL-terminated "NAME=value" snapshot.
	UINT32 event;                         // Event from the autorun entry ("reserve" slot).
	const WCHAR *dir;                     // Directory part of "path", with trailing '/'.
	const EP3_Image_Manager_T *img_man;   // Dispatch table into the Image Manager.
	const EP3_Loader_Api_T *api;          // ElfPack API functions (setenv/getenv/...).
} EP3_ImageArgs_T;

// EXL, 02-Nov-2026: The image entry point ("start" symbol, e.g. -Wl,-eEP3_Main).
typedef UINT32 (*EP3_Image_Entry_T)(EP3_ImageArgs_T *args);

// EXL, 02-Nov-2026: Build the heap argument block (ImageArgs + arrays + strings).
//   "params" is the ASCII autorun args string (may be NULL). Returns NULL on
//   allocation failure. The block is freed by the Image Manager on unregister.
extern EP3_ImageArgs_T *EP3_ImageArgs_Build(const WCHAR *path, const char *params, UINT32 event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_IMAGE_ARGS_H */
