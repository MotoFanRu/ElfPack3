#ifndef P2K_EP3_BASE_H
#define P2K_EP3_BASE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EP3_BIN_LDR_NAME                                   "ep3.bin"
#define EP3_ELF_LDR_NAME                                   "ep3.ldr"
#define EP3_ELF_LDR_LIB_NAME                               "ep3.so"
#define EP3_ELF_LDR_CFG_NAME                               "ep3.ini"

#define LOG_BUFFER_SIZE                                    (255)

typedef enum {
	LOGGER_SUAPI,
	LOGGER_JAL,
	LOGGER_ALL
} EP3_LOGGER_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_BASE_H */
