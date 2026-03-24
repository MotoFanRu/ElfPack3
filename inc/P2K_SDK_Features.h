#ifndef P2K_SDK_FEATURES_H
#define P2K_SDK_FEATURES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* General Settings */
#if (0)
	#define FTR_DONT_DEFINE_SDK_P2K_TYPES                  (0)
	#define FTR_DONT_DEFINE_SDK_CLIKE_TYPES                (0)
	#define FTR_DONT_DEFINE_SDK_RUST_TYPES                 (0)
	#define FTR_DONT_DEFINE_SDK_NULL                       (0)
	#define FTR_DONT_DEFINE_SDK_BOOL                       (0)
	#define FTR_DONT_DEFINE_SDK_RESULT                     (0)
	#define FTR_DONT_DEFINE_SDK_BYTEWORDS                  (0)
	#define FTR_DONT_DEFINE_SDK_WIDECHAR                   (0)
	#define FTR_DONT_DEFINE_ARRAY_SIZE                     (0)
	#define FTR_DONT_DEFINE_LIMITS                         (0)
	#define FTR_DONT_DEFINE_DEPRECATED_MACROS              (0)
	#define FTR_WARN_ON_RAW_DATA_ACCESS                    (0)
	#define FTR_DONT_DEFINE_ACCESS_MACROS                  (0)
#endif

/* EP3 Project */
#if (0)
	#define FTR_BIN_LDR                                    (0)
	#define FTR_ELF_LDR                                    (0)
	#define FTR_SO_LIB                                     (0)
	#define FTR_ALLOC_SUAPI                                (0)
	#define FTR_ALLOC_UIS                                  (0)
	#define FTR_ALLOC_J2ME                                 (0)
	#define FTR_NO_VSNPRINTF                               (0)
	#define FTR_LOAD_TO_ADDR                               (0x00000000)
#endif

/* Compiler & CPU Settings */
#if (0)
	#define FTR_ARM_MODE                                   (0)
	#define FTR_THUMB_MODE                                 (0)
	#define FTR_ARM                                        (0)
	#define FTR_ARM7                                       (0)
	#define FTR_ARM11                                      (0)
	#define FTR_MCORE                                      (0)
	#define FTR_MCORE210                                   (0)
	#define FTR_MCORE340                                   (0)
	#define FTR_NEPTUNE                                    (0)
	#define FTR_NEPTUNE_LT_LCA                             (0)
	#define FTR_NEPTUNE_LTS                                (0)
	#define FTR_NEPTUNE_ULS                                (0)
	#define FTR_NEPTUNE_LTE                                (0)
	#define FTR_NEPTUNE_LTE2                               (0)
	#define FTR_NEPTUNE_LTE2_ROM4                          (0)
	#define FTR_PATRIOT                                    (0)
	#define FTR_RAINBOW                                    (0)
	#define FTR_RAINBOW_POG                                (0)
	#define FTR_ARGON                                      (0)
	#define FTR_ARGON_PLUS                                 (0)
	#define FTR_ARGON_LV                                   (0)
#endif

/* Logging Settings */
#if (0)
	#define FTR_DEBUG                                      (0)
	#define FTR_LOGGER                                     (0)
	#define FTR_LOGGER_NULL                                (0)
	#define FTR_LOGGER_JAL                                 (0)
	#define FTR_LOGGER_SUAPI                               (0)
	#define FTR_LOGGER_FILE                                (0)
	#define FTR_LOGGER_FILE_PREDIFINED                     (0)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_FEATURES_H */
