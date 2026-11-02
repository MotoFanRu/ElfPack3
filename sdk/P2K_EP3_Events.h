#ifndef P2K_EP3_EVENTS_H
#define P2K_EP3_EVENTS_H

#include "P2K_SDK_Base.h"
#include "P2K_AFW_Event.h"
#include "P2K_Application.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 02-Nov-2026: Event system, ported from em2 (ldrSendEvent* family). Each image
//   gets an EVENT BASE — a 0x40-wide block of AFW event codes it owns (ep3.cfg autorun
//   may pin one explicitly, otherwise the loader allocates it from a global cursor and
//
//   Handler tables (APP_EV_HANDLER_ENTRY_T, ended by APP_END_EV_TABLE) support the
//   em2/ep2 sentinels:
//     EP3_EV_RESERVED      ((UINT32)-2)  slots get sequential codes from the base
//     EP3_EV_PM_API_EXIT   ((UINT32)-3)  slot is rewritten to EP3_EVENT_PM_API_EXIT
//   and any other code tagged with the constants prefix (0xC0000000, see
//   P2K_EP3_DEF_Library.h) is resolved through the def constants namespace —
//   the ep2 ldrInitEventHandlersTbl behaviour with EP3 hash-tagged constants.
//
//   Hash precision: the tag prefix fixes the top nibble at 0xC (mask 0x0FFFFFFF),
//   so tags span 0xC0000000..0xCFFFFFFF and 28 bits of FNV1A32 are kept. This
//   costs nothing on ARM7TDMI-S — the fixup is one AND+CMP per word either way.
//   passes it through EP3_ImageArgs_T.event). The image builds its own codes as
//   "event + offset" (offset 0 = the start code) and registers them with Synergy via
//   APP_Register; the loader only provides allocation and sending shims.

#define EP3_EVENT_BASE                 (0xA000)  // First auto-allocated base (em2 parity).
#define EP3_EVENT_RESERVE              (0x40)    // Codes per image (em2 EVBASE_RESERVE).

#define EP3_EV_RESERVED                ((UINT32) -2)  // Handler table slot sentinel.
#define EP3_EV_PM_API_EXIT             ((UINT32) -3)  // Handler table slot sentinel.
#define EP3_EVENT_PM_API_EXIT          (0xA20400)     // Target code (em2 EV_PM_API_EXIT).

// EXL, 02-Nov-2026: Allocate a fresh event base block (cursor += EP3_EVENT_RESERVE).
//   "EP3_API_Release_Event_Base" rolls the cursor back (only when it is the last
//   reservation) — used when an image fails to start.
extern UINT32 EP3_API_Request_Event_Base(void);
extern void EP3_API_Release_Event_Base(UINT32 base);

// EXL, 02-Nov-2026: Send shims over the AFW queued-event calls (em2 ldrSendEvent*).
extern STATUS EP3_API_Send_Event(UINT32 evcode);
extern STATUS EP3_API_Send_Event_To_App(
	AFW_APP_REGISTRY_ID_T reg_id, UINT32 evcode, void *attachment, UINT32 att_size, AFW_EV_BUF_FLAG_T flag
);
extern STATUS EP3_API_Send_Event_To_App_Empty(AFW_APP_REGISTRY_ID_T reg_id, UINT32 evcode);

// EXL, 02-Nov-2026: Handler table helpers (the ldrInitEventHandlersTbl /
//   ldrFindEventHandlerTbl ports, with the ep2 const resolution included).
//   Init: RESERVED slots get sequential codes from "base" (returns the next free
//   code), the PM_API_EXIT sentinel is rewritten to EP3_EVENT_PM_API_EXIT, and
//   0xC0-tagged constants are resolved via the def constants namespace.
//   Find: linear scan by handler pointer, returns the code or 0.
extern UINT32 EP3_API_Init_Event_Handlers_Tbl(APP_EV_HANDLER_ENTRY_T *tbl, UINT32 base);
extern UINT32 EP3_API_Find_Event_Handler_Tbl(const APP_EV_HANDLER_ENTRY_T *tbl, APP_HANDLER_FUNC_T *hfn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_EVENTS_H */
