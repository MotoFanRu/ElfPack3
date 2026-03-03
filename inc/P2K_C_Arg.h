#ifndef P2K_C_ARG_H
#define P2K_C_ARG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* TODO: RECHECK THIS!!! */
#if defined(FTR_ARM7) || defined(FTR_NEPTUNE)
	typedef struct {
		__builtin_va_list args;
	} va_list;

	#define va_start(v, l)                                 __builtin_va_start((v).args, l)
	#define va_arg(v, type)                                __builtin_va_arg((v).args, type)
	#define va_end(v)                                      __builtin_va_end((v).args)
	#define va_copy(d, s)                                  __builtin_va_copy((d).args, (s).args)
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
