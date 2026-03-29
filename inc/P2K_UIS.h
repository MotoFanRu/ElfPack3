#ifndef P2K_UIS_H
#define P2K_UIS_H

#include "P2K_SDK_Base.h"
#include "P2K_SUAPI.h"
#include "P2K_Synergy.h"
#include "P2K_AFW.h"
#include "P2K_AFW_Event.h"
#include "P2K_DL_Clock.h"
#include "P2K_DL_Call.h"
#include "P2K_DL_File_System.h"
#include "P2K_UIS_Ustring.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define UIS_NULL_HANDLE                (0L)
#define UIS_FORMAT_SIZE                (40)

typedef UINT32                         UIS_NUMBER_T;
typedef UINT32                         UIS_DIALOG_HANDLE_T;
typedef UINT32                         UIS_ANIMATION_T;
typedef UINT32                         UIS_SIZE_T;
typedef UINT32                         UIS_PICTURE_ID_T;
typedef SYN_BOOL                       UIS_VOICE_NAME_ID_T;
typedef INT32                          LIST_ENTRY_TAG_T;
typedef UINT32                         UIS_LONG_MESSAGE_HANDLE_T;

enum tagUIS_CLEANUP_UNITS_T {
	UIS_CLEANUP_DAYS = 0,
	UIS_CLEANUP_MESSAGES,
	UIS_CLEANUP_NONE,
	UIS_MAX_CLEANUP_UNITS
};
typedef UINT8                          UIS_CLEANUP_UNITS_T;

enum tagUIS_DURATION_UNITS_T {
	UIS_SECONDS = 0,
	UIS_MINUTES,
	UIS_HOURS,
	UIS_DAYS,
	UIS_WEEKS,
	UIS_DURATION_NO,
	UIS_SECONDS_NE,
	UIS_MINUTES_NE,
	UIS_HOURS_NE,
	UIS_DAYS_NE,
	UIS_WEEKS_NE,
	UIS_MAX_DURATION_UNITS
};
typedef UINT8                          UIS_DURATION_UNITS_T;

enum tagUIS_SPEED_CALL_TYPE_T {
	UIS_SPEED_CALL_FIXED = 0,
	UIS_SPEED_CALL_PHONEBOOK,
	UIS_MAX_SPEEDCALL_TYPE
};
typedef UINT8                          UIS_SPEED_CALL_TYPE_T;

enum tagUIS_STATUS_ID_T {
	UIS_MIN_STATUS_ID = 0,
	/* TODO: Incomplete enum. */
};
typedef UINT8                          UIS_STATUS_ID_T;

enum tagUIS_PICKER_TYPE_T {
	MMS_SECONDS = 0,
	MMS_MINUTE,
	MMS_HOUR,
	MMS_DAY,
	MMS_MONTH
};
typedef UINT8                          UIS_PICKER_TYPE_T;

enum tagUIS_LIST_IMAGE_TYPE_T {
	UIS_LIST_IMAGE_UIS_MEDIA_PATH_T = 0,
	UIS_LIST_IMAGE_DRM_RESOURCE_ID_T,
	UIS_LIST_IMAGE_DL_FS_MID_T,
	UIS_LIST_IMAGE_UIS_MEDIA_PATH_WITH_OFFSET_T,
	UIS_LIST_IMAGE_POINTER
};
typedef UINT8                          UIS_LIST_IMAGE_TYPE_T;

enum tagUIS_ACTION_OPERATION_T {
	UIS_ADD_ACTION = 0,
	UIS_DELETE_ACTION,
	UIS_MODIFY_ACTION
};
typedef UINT8                          UIS_ACTION_OPERATION_T;

typedef struct tagUIS_CLEANUP_T {
	UINT16                             value;
	UIS_CLEANUP_UNITS_T                units;
} UIS_CLEANUP_T;

typedef struct tagUIS_DURATION_T {
	UIS_SIZE_T                         amount;
	UIS_DURATION_UNITS_T               units;
} UIS_DURATION_T;

typedef struct tagUIS_SPEED_CALL_T {
	UIS_SPEED_CALL_TYPE_T              type;
	UINT16                             number;
} UIS_SPEED_CALL_T;

typedef struct tagUIS_TAG_AND_INDEX_T {
	INT32                              list_item;
	LIST_ENTRY_TAG_T                   tag;
} UIS_TAG_AND_INDEX_T;

typedef struct tagUIS_MMS_PICKER_DATA_T {
	UIS_SIZE_T                         pickerValue;
	UIS_PICKER_TYPE_T                  pickerType;
} UIS_MMS_PICKER_DATA_T;

typedef struct tagUIS_LIST_IMAGE_FILE_DETAILS_T {
	WCHAR *                            file_name;
	UINT32                             offset;
	UINT32                             size;
	UINT8                              mime_type;
} UIS_LIST_IMAGE_FILE_DETAILS_T;

typedef struct tagUIS_LIST_IMAGE_POINTER_DETAILS_T {
	void *                             file_name;          /* image pointer */
	UINT32                             image_size;         /* image size */
} UIS_LIST_IMAGE_POINTER_DETAILS_T;

typedef union tagUIS_LIST_IMAGE_T {
	UIS_LIST_IMAGE_FILE_DETAILS_T      image_file_details;
	UIS_LIST_IMAGE_POINTER_DETAILS_T   image_ptr_details;
	DL_FS_MID_T                        file_id;
	DRM_RESOURCE_ID_T                  drm_id;
} UIS_LIST_IMAGE_T;

typedef struct tagUIS_LIST_IMAGE_ELEMENT_T {
	UIS_LIST_IMAGE_T                   image;
	UINT16                             image_index;
	UIS_LIST_IMAGE_TYPE_T              image_type;
	BYTE                               padding;
} UIS_LIST_IMAGE_ELEMENT_T;

typedef struct tagUIS_EXTENDED_PICKER_T {
	UINT16                             value;
	DRM_RESOURCE_ID_T                  units;
} UIS_EXTENDED_PICKER_T;

typedef union tagUIS_VALUE_T {
	DRM_RESOURCE_ID_T                  name;               /* Named string name (resource ID). */
	DL_CALL_METERS_CALL_TIME_T         elapsed_time;       /* For UIS_ELAPSED_TIME ('h') format. */
	DL_CLK_TIME_T                      time_of_day;
	DL_CLK_DATE_T                      date;
	UIS_ANIMATION_T                    animation;          /* Animation resource ID. */
	UIS_CLEANUP_T                      cleanup;
	UIS_STRING_T *                     currency_value;     /* Currency string includes decimal point (if any) */
	UIS_DURATION_T                     duration;
	UIS_PICTURE_ID_T                   picture;            /* Picture resource ID. */
	UIS_SPEED_CALL_T                   speed_call;
	UIS_SIZE_T                         number;
	UIS_STRING_T *                     data;               /* Pointer to data string. */
	UIS_STRING_T *                     notes;              /* Pointer to musical notes string data. */
	UIS_STRING_T *                     phone_number;
	UIS_VOICE_NAME_ID_T                voice_name_id;
	UIS_TAG_AND_INDEX_T                list_entry_id;
	UIS_STATUS_ID_T                    status_ID;
	UIS_LONG_MESSAGE_HANDLE_T          msg_id;
	UIS_MMS_PICKER_DATA_T              mms_picker_data;
	struct tagUIS_CONTENT_T *          content;
	DL_FS_MID_T                        file_id;
	UIS_LIST_IMAGE_ELEMENT_T *         list_image_elem;
	UIS_EXTENDED_PICKER_T              picker_data;
} UIS_VALUE_T;

typedef struct tagUIS_CONTENT_T {
	UIS_FORMAT_STRING_T                format_string[UIS_FORMAT_SIZE];
	UIS_NUMBER_T                       num_values;
	UIS_VALUE_T                        values[P2K_UIS_MAX_NUM_VALUES];
} UIS_CONTENT_T;

typedef struct tagUIS_APP_ENV_T {
	SU_PORT_HANDLE                     routeId;
} UIS_APP_ENV_T;

typedef struct tagUIS_ACTION_LIST_ELEMENT_T {
	UIS_ACTION_OPERATION_T             operation;
	AFW_EVENT_CODE_T                   event;
	DRM_RESOURCE_ID_T                  resIdActListItem;
} UIS_ACTION_LIST_ELEMENT_T;

typedef struct tagUIS_ACTION_LIST_T {
	UINT8                              num_actions;
	UIS_ACTION_LIST_ELEMENT_T          action[P2K_UIS_MAX_NUM_ACTION_LIST_ITEMS];
} UIS_ACTION_LIST_T;

extern SYN_RETURN_STATUS_T UIS_MakeContentFromString(
	UIS_FORMAT_STRING_T *             format_string,
	UIS_CONTENT_T *                   buffer,
	...
);

extern UIS_DIALOG_HANDLE_T UIS_CreateViewer(
	UIS_APP_ENV_T *                    app_env,
	UIS_CONTENT_T *                    contents,
	UIS_ACTION_LIST_T *                action_list
);

extern SYN_RETURN_STATUS_T UIS_Delete(UIS_DIALOG_HANDLE_T handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_UIS_H */
