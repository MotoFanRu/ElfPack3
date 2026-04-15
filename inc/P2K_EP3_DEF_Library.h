#ifndef P2K_EP3_DEF_LIBRARY_H
#define P2K_EP3_DEF_LIBRARY_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DEF_LIB_TOO_SMALL              (32)

#define DEF_LIB_MAX_SYM_NAME_LENGTH    (80)
#define DEF_LIB_MAX_SYM_COUNT          (1024)

#define DEF_LIB_PARSER_HEADER_VAL      (6)
#define DEF_LIB_PARSER_HEADER_CPU_NONE "NONE"
#define DEF_LIB_PARSER_CONTENT_ADDR    (2)
#define DEF_LIB_PARSER_CONTENT_NAME    (13)

#define DEF_LIB_EP3_VERSION_SLUG       "EP3"

typedef struct tagDEF_LIB_Symbol {
	UINT32 hash;
	UINT32 addr;
} DEF_LIB_Symbol;

extern RAW_DATA UINT32 EP3_phone;
extern RAW_DATA UINT32 EP3_firmware;
extern RAW_DATA UINT32 EP3_version;

extern UINT32 EP3_DEF_Library_Load(DEF_LIB_Symbol *def_symbols);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_EP3_DEF_LIBRARY_H */
