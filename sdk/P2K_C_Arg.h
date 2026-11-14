#ifndef P2K_C_ARG_H
#define P2K_C_ARG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !defined(FTR_DONT_DEFINE_C_STD_VA_ARGS)

/*
 * EXL, 15-Nov-2026:
 *
 * Прошивки телефонов P2K на платформе Neptune собраны компилятором ARM ADS,
 * у которого был собственный ABI для функций с переменным количеством
 * аргументов, поэтому необходимо переопределить "va_args" в соответствии
 * с таким ABI, отличным от современного GCC. Вероятно, что определённый
 * ниже "va_args" ABI используются на большинстве мобильных телефонов под
 * управлением MCU-ядра на базе архитектуры ARM7TDMI-S, ARMv4T, прошивки
 * которого собирались компилятором ARM ADS.
 */
#if defined(FTR_NEPTUNE)
	typedef __builtin_va_list          va_list[1];

	#define va_start(v, l)             __builtin_va_start(v[0], l)
	#define va_end(v)                  __builtin_va_end(v[0])
	#define va_arg(v, l)               __builtin_va_arg(v[0], l)
	#define va_copy(d, s)              __builtin_va_copy(d[0], s[0])
#endif /* FTR_NEPTUNE */

/*
 * EXL, 15-Nov-2026:
 *
 * Компиляторы ARM RVCT и M-CORE Diab, которыми собирались прошивки других
 * мобильных телефонов, используют ABI для "va_args" совпадающий с таковым
 * у компилятора GCC.
 */
#if !defined(FTR_NEPTUNE)
	typedef __builtin_va_list          va_list;

	#define va_start(v, l)             __builtin_va_start(v, l)
	#define va_end(v)                  __builtin_va_end(v)
	#define va_arg(v, l)               __builtin_va_arg(v, l)
	#define va_copy(d, s)              __builtin_va_copy(d, s)
#endif /* !FTR_NEPTUNE */

#endif /* !FTR_DONT_DEFINE_C_STD_VA_ARGS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_C_ARG_H */
