#ifndef P2K_EP3_TEXT_H
#define P2K_EP3_TEXT_H

#include "P2K_SDK_Base.h"

static inline UINT32 EP3_Text_Hex_To_U32(const char *text) {
	UINT32 value = 0;
	while (*text) {
		char c = *(text++);
		if (c >= '0' && c <= '9') {
			value = (value << 4) | (c - '0');
		} else if (c >= 'a' && c <= 'f') {
			value = (value << 4) | (c - 'a' + 10);
		} else if (c >= 'A' && c <= 'F') {
			value = (value << 4) | (c - 'A' + 10);
		} else {
			break;
		}
	}
	return value;
}

static inline void EP3_Text_Skip_To_Next_Line(const char **text) {
	while (**text && (**text != '\n') && (**text != '\r')) {
		(*text)++;
	}
	while ((**text == '\n') || (**text == '\r')) {
		(*text)++;
	}
}

#endif /* !P2K_EP3_TEXT_H */
