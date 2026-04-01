#ifndef P2K_EP3_APP_VIEWER_H
#define P2K_EP3_APP_VIEWER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define VIEWER_FUNC_NAME               "ep3/apv"
#define VIEWER_MESSAGE_MAX_LENGTH      (256)
#define APP_VIEWER_STARTUP_EVENT       ((AFW_EVENT_CODE_T) 0x0000ACED)

extern void EP3_APP_Viewer_Register(void);

typedef BOOL (*EP3_APP_VIEW_T)(const char *format, ...);
extern BOOL EP3_API_APP_View(const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_APP_VIEWER_H */
