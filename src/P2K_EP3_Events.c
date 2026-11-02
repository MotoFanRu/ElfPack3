#include <P2K_SDK_Base.h>
#include <P2K_AFW_Event.h>
#include <P2K_Logger.h>

#include <P2K_Application.h>
#include <P2K_EP3_DEF_Library.h>
#include <P2K_EP3_Events.h>

// EXL, 02-Nov-2026: Event system (em2 ldrSendEvent* port). The base allocator hands
//   every image a private EP3_EVENT_RESERVE-wide block of AFW codes; the send shims
//   wrap the Synergy queued-event calls. NOR-safe: everything is cursor math plus
//   firmware calls, no writable statics beyond the cursor (updated only through the
//   Request/Release API, which NOR contexts do not call).

static UINT32 EP3_Event_Base_Cursor = EP3_EVENT_BASE;

UINT32 EP3_API_Request_Event_Base(void) {
	UINT32 base = EP3_Event_Base_Cursor;
	EP3_Event_Base_Cursor += EP3_EVENT_RESERVE;

	L("[EP3 EVT]: Requested event base 0x%08X (next 0x%08X).\n", base, EP3_Event_Base_Cursor);

	return base;
}

void EP3_API_Release_Event_Base(UINT32 base) {
	// EXL, 02-Nov-2026: Roll the cursor back only when this is the last reservation —
	//   intermediate gaps are kept (0x40-wide blocks are plentiful).
	if (EP3_Event_Base_Cursor == (base + EP3_EVENT_RESERVE)) {
		EP3_Event_Base_Cursor = base;
		D("[EP3 EVT]: Released event base 0x%08X.\n", base);
	}
}

STATUS EP3_API_Send_Event(UINT32 evcode) {
	D("[EP3 EVT]: Send event 0x%08X.\n", evcode);

	return (STATUS) AFW_CreateInternalQueuedEvAux(evcode, AFW_BUF_FLAG_READ_ONLY, 0, NULL);
}

STATUS EP3_API_Send_Event_To_App(
	AFW_APP_REGISTRY_ID_T reg_id, UINT32 evcode, void *attachment, UINT32 att_size, AFW_EV_BUF_FLAG_T flag
) {
	D("[EP3 EVT]: Send event 0x%08X to app 0x%04X (%d bytes).\n", evcode, reg_id, att_size);

	return (STATUS) AFW_CreateInternalQueuedEvPriv(
		(AFW_EVENT_CODE_T) evcode,
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC,
		(AFW_EV_ROUTE_ID_T) reg_id,
		AFW_FRAMEWORK_EV_CATG,
		0,
		NULL,
		flag,
		(AFW_EV_BUF_SIZE_T) att_size,
		(AFW_EV_BUF_T *) attachment,
		0
	);
}

STATUS EP3_API_Send_Event_To_App_Empty(AFW_APP_REGISTRY_ID_T reg_id, UINT32 evcode) {
	D("[EP3 EVT]: Send event 0x%08X to app 0x%04X (empty).\n", evcode, reg_id);

	return (STATUS) AFW_CreateInternalQueuedEvPriv(
		(AFW_EVENT_CODE_T) evcode,
		(AFW_AUF_SEQ_NUM_T) AFW_SEQN_GENERIC,
		(AFW_EV_ROUTE_ID_T) reg_id,
		AFW_FRAMEWORK_EV_CATG,
		0,
		NULL,
		AFW_BUF_FLAG_READ_ONLY,
		0,
		NULL,
		0
	);
}

// EXL, 02-Nov-2026: The ldrInitEventHandlersTbl port (em2 walk + ep2 const
//   resolution through the hash-tagged def constants namespace).
UINT32 EP3_API_Init_Event_Handlers_Tbl(APP_EV_HANDLER_ENTRY_T *tbl, UINT32 base) {
	if (tbl == NULL) {
		return base;
	}

	for (UINT32 i = 0; tbl[i].ev_code != (AFW_EVENT_CODE_T) APP_END_EV_TABLE; ++i) {
		if ((UINT32) tbl[i].ev_code == EP3_EV_RESERVED) {
			tbl[i].ev_code = (AFW_EVENT_CODE_T) base++;
			D("New reserved event 0x%08X.\n", tbl[i].ev_code);
		} else if ((UINT32) tbl[i].ev_code == EP3_EV_PM_API_EXIT) {
			tbl[i].ev_code = (AFW_EVENT_CODE_T) EP3_EVENT_PM_API_EXIT;
		} else if ((tbl[i].ev_code & ~((AFW_EVENT_CODE_T) DEF_LIB_CONST_TAG_MASK)) == (AFW_EVENT_CODE_T) DEF_LIB_CONST_TAG_PREFIX) {
			UINT32 value = 0;
			if (EP3_DEF_Library_Get_Const_By_Hash((UINT32) tbl[i].ev_code, &value) == RESULT_OK) {
				tbl[i].ev_code = (AFW_EVENT_CODE_T) value;
				D("Constant event added 0x%08X.\n", tbl[i].ev_code);
			} else {
				D("I think it 0xC0000000 but not int the constant lib? 0x%08X 0x%08X.\n", tbl[i].ev_code, value);
			}
			// EXL, 02-Nov-2026: Unresolvable tags are left for EP3_API_Const_Fix_Table /
			//   a later def reload; the firmware treats unknown codes as no-op slots.
		} else {
			// EXL, 02-Nov-2026: Plain codes pass through untouched.
			D("Skip 0x%08X.\n", tbl[i].ev_code);
		}
	}

	return base;
}

// EXL, 02-Nov-2026: The ldrFindEventHandlerTbl port.
UINT32 EP3_API_Find_Event_Handler_Tbl(const APP_EV_HANDLER_ENTRY_T *tbl, APP_HANDLER_FUNC_T *hfn) {
	if (tbl == NULL) {
		return 0;
	}

	for (UINT32 i = 0; tbl[i].ev_code != (AFW_EVENT_CODE_T) APP_END_EV_TABLE; ++i) {
		if (tbl[i].p_ev_func == hfn) {
			return (UINT32) tbl[i].ev_code;
		}
	}

	return 0;
}

