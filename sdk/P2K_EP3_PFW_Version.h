#ifndef P2K_EP3_PFW_VERSION_H
#define P2K_EP3_PFW_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !defined(FTR_EP3_PHONE)
	#error "Please define the FTR_EP3_PHONE string!"
#endif

#if !defined(FTR_EP3_FIRMWARE)
	#error "Please define the FTR_EP3_FIRMWARE string!"
#endif

#if !defined(FTR_EP3_VERSION)
	#error "Please define the FTR_EP3_VERSION string!"
#endif

static inline const char *EP3_API_Get_Phone(void) {
	return FTR_EP3_PHONE;
}

static inline const char *EP3_API_Get_Firmware(void) {
	return FTR_EP3_FIRMWARE;
}

static inline const char *EP3_API_Get_Version(void) {
	return FTR_EP3_VERSION;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_PFW_VERSION_H */
