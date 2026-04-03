#ifndef P2K_SDK_BASE_H
#define P2K_SDK_BASE_H

#include "P2K_SDK_Features.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* P2K Types (Motorola-style uppercase, fixed-width). */
#if !defined(FTR_DONT_DEFINE_SDK_P2K_TYPES)
	typedef signed char                INT8;
	typedef unsigned char              UINT8;
	typedef signed short               INT16;
	typedef unsigned short             UINT16;
	typedef signed long                INT32;
	typedef unsigned long              UINT32;
	typedef signed long long           INT64;
	typedef unsigned long long         UINT64;

	typedef unsigned long              SIZE_T;
	typedef signed long                SSIZE_T;
	typedef unsigned long              UINTPTR;
	typedef signed long                INTPTR;
	typedef signed long                PTRDIFF;

	typedef float                      FLOAT32;  /* IEEE 754 single. */
	typedef double                     FLOAT64;  /* IEEE 754 double. */

	typedef void                       VOID;
#endif /* !FTR_DONT_DEFINE_SDK_P2K_TYPES */

/* C-like Types (stdint.h / stddef.h style). */
#if !defined(FTR_DONT_DEFINE_SDK_CLIKE_TYPES)
	typedef signed char                int8_t;
	typedef unsigned char              uint8_t;
	typedef signed short               int16_t;
	typedef unsigned short             uint16_t;
	typedef signed long                int32_t;
	typedef unsigned long              uint32_t;
	typedef signed long long           int64_t;
	typedef unsigned long long         uint64_t;

	typedef unsigned long              size_t;
	typedef signed long                ssize_t;
	typedef unsigned long              uintptr_t;
	typedef signed long                intptr_t;
	typedef signed long                ptrdiff_t;

	typedef float                      float32_t;  /* IEEE 754 single. */
	typedef double                     float64_t;  /* IEEE 754 double. */
#endif /* !FTR_DONT_DEFINE_SDK_CLIKE_TYPES */

/* Rust-like Types (modern embedded style). */
#if !defined(FTR_DONT_DEFINE_SDK_RUST_TYPES)
	typedef signed char                i8;
	typedef unsigned char              u8;
	typedef signed short               i16;
	typedef unsigned short             u16;
	typedef signed long                i32;
	typedef unsigned long              u32;
	typedef signed long long           i64;
	typedef unsigned long long         u64;

	typedef unsigned long              usize;
	typedef signed long                isize;

	typedef float                      f32;  /* IEEE 754 single. */
	typedef double                     f64;  /* IEEE 754 double. */
#endif /* !FTR_DONT_DEFINE_SDK_RUST_TYPES */

/* NULL */
#if !defined(FTR_DONT_DEFINE_SDK_NULL)
	#if !defined(__cplusplus)
		#define NULL                   ((void *) 0)
	#else
		#define NULL                   (0)
	#endif /* !__cplusplus */
#endif /* !FTR_DONT_DEFINE_SDK_NULL */

/* NULL_ADDR */
#if !defined(FTR_DONT_DEFINE_SDK_NULL_ADDR)
	#define NULL_ADDR                  (0x00000000)
#endif /* !FTR_DONT_DEFINE_SDK_NULL_ADDR */

/* BOOL */
#if !defined(FTR_DONT_DEFINE_SDK_BOOL)
	typedef unsigned char              BOOL;

	#define TRUE                       ((BOOL) 1)
	#define FALSE                      ((BOOL) 0)
#endif /* !FTR_DONT_DEFINE_SDK_BOOL */

/* RESULT STATUS*/
#if !defined(FTR_DONT_DEFINE_SDK_RESULT_STATUS)
	typedef signed int                 STATUS;

	#define RESULT_OK                  (0)
	#define RESULT_FAIL                (1)
#endif /* !FTR_DONT_DEFINE_SDK_RESULT_STATUS */

/* BYTE / WORD / DWORD / QWORD */
#if !defined(FTR_DONT_DEFINE_SDK_BYTEWORDS)
	typedef unsigned char              BYTE;   /* 8-bit.  */
	typedef unsigned short             WORD;   /* 16-bit. */
	typedef unsigned long              DWORD;  /* 32-bit. */
	typedef unsigned long long         QWORD;  /* 64-bit. */
#endif /* !FTR_DONT_DEFINE_SDK_BYTEWORDS */

/* WCHAR */
#if !defined(FTR_DONT_DEFINE_SDK_WIDECHAR)
	typedef unsigned short             WCHAR;
#endif /* !FTR_DONT_DEFINE_SDK_WIDECHAR */

/* ARRAY_SIZE */
#if !defined(FTR_DONT_DEFINE_ARRAY_SIZE)
	#define ARRAY_SIZE(array)          (sizeof(array) / sizeof((array)[0]))
#endif /* !FTR_DONT_DEFINE_ARRAY_SIZE */

#if !defined(FTR_DONT_DEFINE_LIMITS)
	#define CHAR_BIT                   (8)
	#define BYTE_BIT                   (8)
	#define BOOL_WIDTH                 (8)
	#define SCHAR_WIDTH                (8)
	#define UCHAR_WIDTH                (8)
	#define SHRT_WIDTH                 (16)
	#define USHRT_WIDTH                (16)
	#define INT_WIDTH                  (32)
	#define UINT_WIDTH                 (32)
	#define LONG_WIDTH                 (32)
	#define ULONG_WIDTH                (32)
	#define LLONG_WIDTH                (64)
	#define ULLONG_WIDTH               (64)
	#define FLOAT_WIDTH                (32)
	#define DOUBLE_WIDTH               (64)

	/* Since we use -funsigned-char on both ARM and M-CORE. */
	#define CHAR_MIN                   (0)
	#define CHAR_MAX                   (255)
	#define SCHAR_MIN                  (-128)
	#define SCHAR_MAX                  (127)
	#define UCHAR_MIN                  (0)
	#define UCHAR_MAX                  (255)
	#define SHRT_MIN                   (-32768)
	#define SHRT_MAX                   (32767)
	#define USHRT_MIN                  (0)
	#define USHRT_MAX                  (65535)
	#define INT_MIN                    (-2147483647 - 1)
	#define INT_MAX                    (2147483647)
	#define UINT_MIN                   (0U)
	#define UINT_MAX                   (4294967295U)
	#define LONG_MIN                   (-2147483647L - 1L)
	#define LONG_MAX                   (2147483647L)
	#define ULONG_MIN                  (0LU)
	#define ULONG_MAX                  (4294967295LU)
	#define LLONG_MIN                  (-9223372036854775807LL - 1LL)
	#define LLONG_MAX                  (9223372036854775807LL)
	#define ULLONG_MIN                 (0LLU)
	#define ULLONG_MAX                 (18446744073709551615LLU)

	#define FLT_MIN                    (1.17549435E-38F)
	#define FLT_MAX                    (3.40282347E+38F)
	#define FLT_EPSILON                (1.19209290E-7F)
	#define DBL_MIN                    (2.2250738585072014E-308)
	#define DBL_MAX                    (1.7976931348623157E+308)
	#define DBL_EPSILON                (2.2204460492503131E-16)

	#define INT8_MIN                   (SCHAR_MIN)
	#define INT8_MAX                   (SCHAR_MAX)
	#define UINT8_MIN                  (UCHAR_MIN)
	#define UINT8_MAX                  (UCHAR_MAX)
	#define INT16_MIN                  (SHRT_MIN)
	#define INT16_MAX                  (SHRT_MAX)
	#define UINT16_MIN                 (USHRT_MIN)
	#define UINT16_MAX                 (USHRT_MAX)
	#define INT32_MIN                  (INT_MIN)
	#define INT32_MAX                  (INT_MAX)
	#define UINT32_MIN                 (UINT_MIN)
	#define UINT32_MAX                 (UINT_MAX)
	#define INT64_MIN                  (LLONG_MIN)
	#define INT64_MAX                  (LLONG_MAX)
	#define UINT64_MIN                 (ULLONG_MIN)
	#define UINT64_MAX                 (ULLONG_MAX)

	#define FLOAT32_MIN                (FLT_MIN)
	#define FLOAT32_MAX                (FLT_MAX)
	#define FLOAT64_MIN                (DBL_MIN)
	#define FLOAT64_MAX                (DBL_MAX)

	#define SIZE_WIDTH                 (UINT_WIDTH)
	#define SIZE_MIN                   (UINT_MIN)
	#define SIZE_MAX                   (UINT_MAX)
	#define SSIZE_WIDTH                (INT_WIDTH)
	#define SSIZE_MIN                  (INT_MIN)
	#define SSIZE_MAX                  (INT_MAX)
	#define UINTPTR_WIDTH              (UINT_WIDTH)
	#define UINTPTR_MIN                (UINT_MIN)
	#define UINTPTR_MAX                (UINT_MAX)
	#define INTPTR_WIDTH               (INT_WIDTH)
	#define INTPTR_MIN                 (INT_MIN)
	#define INTPTR_MAX                 (INT_MAX)
	#define PTRDIFF_WIDTH              (INT_WIDTH)
	#define PTRDIFF_MIN                (INT_MIN)
	#define PTRDIFF_MAX                (INT_MAX)

	#define BYTE_WIDTH                 (BYTE_BIT)
	#define BYTE_MIN                   (UCHAR_MIN)
	#define BYTE_MAX                   (UCHAR_MAX)
	#define WORD_WIDTH                 (USHRT_WIDTH)
	#define WORD_MIN                   (USHRT_MIN)
	#define WORD_MAX                   (USHRT_MAX)
	#define DWORD_WIDTH                (UINT_WIDTH)
	#define DWORD_MIN                  (UINT_MIN)
	#define DWORD_MAX                  (UINT_MAX)
	#define QWORD_WIDTH                (ULLONG_WIDTH)
	#define QWORD_MIN                  (ULLONG_MIN)
	#define QWORD_MAX                  (ULLONG_MAX)

	/* Since we use -fshort-wchar on both ARM and M-CORE. */
	#define WCHAR_WIDTH                (USHRT_WIDTH)
	#define WCHAR_MIN                  (USHRT_MIN)
	#define WCHAR_MAX                  (USHRT_MAX)
#endif /* !FTR_DONT_DEFINE_LIMITS */

#if !defined(FTR_DONT_DEFINE_UNUSED)
	#define UNUSED(x)                  ((void) (x))
#endif /* !FTR_DONT_DEFINE_UNUSED */

#if !defined(FTR_DONT_DEFINE_DEPRECATED_MACROS)
	#define DEPRECATED(x)              __attribute__((deprecated(x)))
#endif /* !FTR_DONT_DEFINE_DEPRECATED_MACROS */

#if defined(FTR_WARN_ON_RAW_DATA_ACCESS)
	#define RAW_DATA                   __attribute__((deprecated("Use GET_DATA(x), GET_CONST(x), GET_DATA_ADDR(x)")))
#else
	#define RAW_DATA
#endif /* FTR_WARN_ON_RAW_DATA_ACCESS */

#if !defined(FTR_DONT_DEFINE_ACCESS_MACROS)
	#define GET_DATA(x)                ((UINTPTR) (x))
	#define GET_CONST(x)               ((UINTPTR) (&x))
	#define GET_DATA_ADDR(x)           ((UINTPTR) (&x))
	#define GET_FUNC_ADDR(x)           ((UINTPTR) (&x))
#endif /* !FTR_DONT_DEFINE_ACCESS_MACROS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_SDK_BASE_H */
