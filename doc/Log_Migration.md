# SDK and EP3 Migration/Development Log

## 22-Feb-2026...03-Apr-2026

* Working on Aleph library
* Working on Task API
* Working on Skeleton
* Working on Log View
* Working on va_args
* Migrated functions:

  ```
  sprintf
  snprintf
  vsprintf
  vsnprintf
  memset

  DL_KeyQueryKeypadActivity

  PFprintf
  AmMemAllocPointer
  AmMemFreePointer

  suLogData
  sc_lock
  sc_unlock
  suDisableSched
  suEnableSched
  suDisableInt
  suDisableAllInt
  suSetInt
  suCreateQueue
  suCreatePortFromQueue
  suRegisterName
  suReceiveMessageFromQueue
  suCreateMessage
  suSendMessage
  suDeleteMessage
  suCreateBSem
  suCreateCSem
  suCreateMSem
  suAcquireSem
  suReleaseSem
  suDeleteSem
  suSleep

  uisAllocateMemory
  uisReAllocMemory
  uisFreeMemory

  AFW_InquireRoutingStackByRegId
  AFW_GetEv
  AFW_GetEvCode
  AFW_GetEvSeqn
  APP_Register
  APP_HandleUITokenGranted
  APP_HandleUITokenRevoked
  APP_InitAppData
  APP_HandleEvent
  APP_HandleEventPrepost
  APP_Start
  APP_HandleFailedAppStart
  APP_ConsumeEv
  APP_UtilChangeState
  APP_UtilConsumeEvChangeState
  APP_Exit

  UIS_MakeContentFromString
  UIS_CreateViewer
  UIS_Delete

  AFW_CreateInternalQueuedEv
  AFW_CreateInternalQueuedSvcEv
  AFW_CreateInternalQueuedEvAux
  AFW_CreateInternalQueuedEvAuxD
  AFW_CreateInternalQueuedSvcEvAux
  AFW_CreateInternalQueuedSvcEvAuxD
  AFW_CreateInternalQueuedEvPriv

  APP_UtilUISDialogDelete
  APP_HandleConsumeEvAndExit

  suFindName

  AFW_AddEv
  AFW_AddEvNoD
  AFW_AddEvEvD
  AFW_AddEvAux
  AFW_AddEvAuxD
  AFW_AddSvcEv
  AFW_AddSvcEvNoD
  AFW_AddSvcEvEvD
  AFW_AddSvcEvAux
  AFW_AddSvcEvAuxD
  AFW_AddEvPriv
  AFW_TranslateEv
  AFW_TranslateEvNoD
  AFW_TranslateEvEvD
  AFW_TranslateEvAux
  AFW_TranslateEvAuxD
  ```

* Migrated structs:

  * APP_INSTANCE_DATA_T (APPLICATION_T) + dependencies.
  * DL_DB_SHORTCUT_RECORD_T (SEEM_0002_T) + dependencies.
  * AFW_EVENT_GROUP_T (EVENT_STACK_T) + dependencies.
  * UIS_ACTION_LIST_T + dependencies.
  * UIS_CONTENT_T + dependencies.

* Some general events like `EV_DIALOG_DONE` and `EV_DONE` were migrated.

* Deleted from SDK & Libraries:

  ```
  APP_InitData
  APP_LockInitAppData
  APP_LockViewOnlyInitData
  AFW_GetEvRtid

  EV_GAIN_FOCUS
  EV_GRANT_TOKEN
  EV_LOSE_FOCUS
  EV_POWER_DOWN
  EV_REVOKE_TOKEN
  EV_RENDER
  EV_APP_DONE
  EV_DIALOG_DONE
  EV_DONE

  AFW_CreateInternalQueuedEv
  AFW_CreateInternalQueuedSvcEv
  AFW_CreateInternalQueuedEvAux
  AFW_CreateInternalQueuedEvAuxD
  AFW_CreateInternalQueuedSvcEvAux
  AFW_CreateInternalQueuedSvcEvAuxD
  AFW_AddEv
  AFW_AddEvNoD
  AFW_AddEvEvD
  AFW_AddEvAux
  AFW_AddEvAuxD
  AFW_AddSvcEv
  AFW_AddSvcEvNoD
  AFW_AddSvcEvEvD
  AFW_AddSvcEvAux
  AFW_AddSvcEvAuxD
  ```

## 20-Feb-2026

* E1, V3i, V3x kitchens
* Migrated functions:

  ```
  DL_FsOpenFile
  DL_FsReadFile
  DL_FsCloseFile
  DL_FsFSeekFile
  DL_FsGetFileSize
  DL_FsSGetFileSize

  APP_CALC_MainRegister
  
  suAllocMem
  suFreeMem

  u_atol
  u_atou
  u_isAlpha
  u_isDigit
  u_isLower
  u_isSpace
  u_isUpper
  u_itoa
  u_itoh
  u_ltou
  u_strcat
  u_strchr
  u_strcmp
  u_strcpy
  u_strcut
  u_stristr
  u_strlen
  u_strmakelower
  u_strmakeupper
  u_strncat
  u_strncmp
  u_strncmpi
  u_strncpy
  u_strrchr
  u_strstr
  u_uint64tou
  u_uitoa
  u_ultou
  ```

## 19-Feb-2026

* EM1, EM2 elfs will be dropped.
* ELF ARM Flags that can be used to distinguish ELFs:

  ```
  EP1 ELF     — 0x0200001E
  EP2 ELF, SO — 0x05000002
  EG1 ELF     — 0x05000200
  ```

## 18-Feb-2026

* Core Types and Platform Defines.
