#ifndef P2K_EP3_BASE_H
#define P2K_EP3_BASE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EP3_BIN_LOADER_FILE_NAME_WCHAR                     L"ep3.bin"
#define EP3_ELF_LOADER_FILE_NAME_WCHAR                     L"ep3.ldr"
#define EP3_ELF_LIBRARY_FILE_NAME_WCHAR                    L"ep3.so"
#define EP3_ELF_LOADER_CFG_FILE_NAME_WCHAR                 L"ep3.ini"

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
