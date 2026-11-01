#ifndef P2K_EP3_EVENTS_H
#define P2K_EP3_EVENTS_H

#include "P2K_SDK_Base.h"
#include "P2K_AFW_Event.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// EXL, 02-Nov-2026: Event system, ported from em2 (ldrSendEvent* family). Each image
//   gets an EVENT BASE — a 0x40-wide block of AFW event codes it owns (ep3.cfg autorun
//   may pin one explicitly, otherwise the loader allocates it from a global cursor and
//   passes it through EP3_ImageArgs_T.event). The image builds its own codes as
//   "event + offset" (offset 0 = the start code) and registers them with Synergy via
//   APP_Register; the loader only provides allocation and sending shims.

#define EP3_EVENT_BASE                 (0xA000)  // First auto-allocated base (em2 parity).
#define EP3_EVENT_RESERVE              (0x40)    // Codes per image (em2 EVBASE_RESERVE).

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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_EVENTS_H */
