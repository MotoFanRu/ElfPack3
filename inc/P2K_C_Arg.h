#ifndef P2K_C_ARG_H
#define P2K_C_ARG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* TODO: RECHECK THIS!!! */
#if defined(FTR_ARM7) || defined(FTR_NEPTUNE)
	typedef __builtin_va_list                              va_list[1];

	#define va_start(v, l)                                 __builtin_va_start(v[0], l)
	#define va_end(v)                                      __builtin_va_end(v[0])
	#define va_arg(v, l)                                   __builtin_va_arg(v[0], l)
	#define va_copy(d, s)                                  __builtin_va_copy(d[0], s[0])
#else
	typedef __builtin_va_list                              va_list;

	#define va_start(v, l)                                 __builtin_va_start(v, l)
	#define va_end(v)                                      __builtin_va_end(v)
	#define va_arg(v, l)                                   __builtin_va_arg(v, l)
	#define va_copy(d, s)                                  __builtin_va_copy(d, s)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_ARG_H */
