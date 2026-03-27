#ifndef P2K_DL_DB_SHORTCUT_H
#define P2K_DL_DB_SHORTCUT_H

#include "P2K_SDK_Base.h"
#include "P2K_Synergy.h"
#include "P2K_DRM.h"
#include "P2K_AFW_Event.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DL_DB_SHORTCUT_MAX_LABEL_LENGTH                    (16)

/* Used in DL_MEDIA_VR_MODEL_T */
#define DL_MEDIA_VR_MODEL_ID_INVALID   (0xFE)
typedef UINT8                          DL_MEDIA_VR_MODEL_T;

enum tagDL_DB_SHORTCUT_TYPE_T {
	DL_DB_SHORTCUT_TYPE_LIST_OBJECT = 0,
	DL_DB_SHORTCUT_TYPE_LIST_SETTING,
	DL_DB_SHORTCUT_TYPE_LIST_COMPOUND_OBJECT,
	DL_DB_SHORTCUT_TYPE_SELECTION_EDITOR_VAL_LIST,
	DL_DB_SHORTCUT_TYPE_SELECTION_EDITOR_VAL_NONLIST,
	DL_DB_SHORTCUT_TYPE_BROWSER_PAGE,
	DL_DB_SHORTCUT_TYPE_APPLICATION,
	DL_DB_SHORTCUT_TYPE_PUSH_MESSAGE,
	DL_DB_SHORTCUT_TYPE_METER_EDITOR_VAL_LIST,
	DL_DB_SHORTCUT_TYPE_METER_EDITOR_VAL_NONLIST,
};
typedef UINT8                          DL_DB_SHORTCUT_TYPE_T;

typedef struct tagDL_DB_LIST_ITEM_T {
	UINT32                             index;
	UINT32                             tag;
} DL_DB_LIST_ITEM_T;

/* size = 0x44, 68 bytes was used on all platforms */
typedef struct tagDL_DB_SHORTCUT_RECORD_T {
	UINT8                              record_id;        /* - the unique id of the shortcut records, begin from 0 */
	DL_DB_SHORTCUT_TYPE_T              type;             /* - the type of dialog displayed */
	UINT8                              shortcut_number;  /* - associated shortcut number used to select, begin from 1 */
	DL_MEDIA_VR_MODEL_T                voice_tag;        /* - associated voice shortcut model id */
	UINT32                             startup_event;    /* - event to queue to invoke shortcut */
	DL_DB_LIST_ITEM_T                  list_item;        /* - if list, highlighted list item value */
	DL_DB_LIST_ITEM_T                  value_list_item;  /* - if sub_dialog list, highlighted list item value */
	INT32                              data;             /* - 32-bit field, can be used as pointer to reference */
	                                                     /*   external data, pointer to URL, database record, etc. */
	                                                     /*   Referenced data is not stored, only the ptr value. */
	DRM_RESOURCE_ID_T                  label_resource;   /* - unicode string name to display in shortcut list */
	WCHAR                              label[DL_DB_SHORTCUT_MAX_LABEL_LENGTH + 1];
	UINT8                              state_tag;        /* - app state to jump to when invoking shortcut */
	SYN_BOOL                           modifiable;
} DL_DB_SHORTCUT_RECORD_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_DL_DB_SHORTCUT_H */
