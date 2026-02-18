/**
* @file P2kBase.h
 * @brief Base type definitions and common macros for the P2K platform SDK.
 * @defgroup P2K_Base P2K Base Definitions
 * @{
 */

#ifndef P2K_BASE_H
#define P2K_BASE_H

#include "P2kFeatures.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* P2K Types (Motorola-style uppercase, fixed-width). */
#if !defined(FTR_DONT_DEFINE_SDK_P2K_TYPES)
	typedef signed char                                    INT8;
	typedef unsigned char                                  UINT8;
	typedef signed short                                   INT16;
	typedef unsigned short                                 UINT16;
	typedef signed long                                    INT32;
	typedef unsigned long                                  UINT32;
	typedef signed long long                               INT64;
	typedef unsigned long long                             UINT64;

	typedef unsigned long                                  SIZE_T;
	typedef signed long                                    SSIZE_T;
	typedef unsigned long                                  UINTPTR;
	typedef signed long                                    INTPTR;
	typedef signed long                                    PTRDIFF;

	typedef float                                          FLOAT32;  /* IEEE 754 single. */
	typedef double                                         FLOAT64;  /* IEEE 754 double. */
	typedef double                                         DOUBLE;   /* IEEE 754 double. */

	typedef void                                           VOID;
#endif /* !FTR_DONT_DEFINE_SDK_P2K_TYPES */

/* C-like Types (stdint.h / stddef.h style). */
#if !defined(FTR_DONT_DEFINE_SDK_CLIKE_TYPES)
	typedef signed char                                    int8_t;
	typedef unsigned char                                  uint8_t;
	typedef signed short                                   int16_t;
	typedef unsigned short                                 uint16_t;
	typedef signed long                                    int32_t;
	typedef unsigned long                                  uint32_t;
	typedef signed long long                               int64_t;
	typedef unsigned long long                             uint64_t;

	typedef unsigned long                                  size_t;
	typedef signed long                                    ssize_t;
	typedef unsigned long                                  uintptr_t;
	typedef signed long                                    intptr_t;
	typedef signed long                                    ptrdiff_t;

	typedef float                                          float32_t;  /* IEEE 754 single. */
	typedef double                                         float64_t;  /* IEEE 754 double. */
	typedef double                                         double64_t; /* IEEE 754 double. */
#endif /* !FTR_DONT_DEFINE_SDK_CLIKE_TYPES */

/* Rust-like Types (modern embedded style). */
#if !defined(FTR_DONT_DEFINE_SDK_RUST_TYPES)
	typedef signed char                                    i8;
	typedef unsigned char                                  u8;
	typedef signed short                                   i16;
	typedef unsigned short                                 u16;
	typedef signed long                                    i32;
	typedef unsigned long                                  u32;
	typedef signed long long                               i64;
	typedef unsigned long long                             u64;

	typedef unsigned long                                  usize;
	typedef signed long                                    isize;

	typedef float                                          f32;  /* IEEE 754 single. */
	typedef double                                         f64;  /* IEEE 754 double. */
	typedef double                                         d64;  /* IEEE 754 double. */
#endif /* !FTR_DONT_DEFINE_SDK_RUST_TYPES */

/* NULL */
#if !defined(FTR_DONT_DEFINE_SDK_NULL)
	#if !defined(__cplusplus)
		#define NULL                                       ((void *) 0)
	#else
		#define NULL                                       (0)
	#endif /* !__cplusplus */
#endif /* !FTR_DONT_DEFINE_SDK_NULL */

/* BOOL */
#if !defined(FTR_DONT_DEFINE_SDK_BOOL)
	typedef unsigned char                                  BOOL;
	typedef unsigned char                                  BOOLEAN;

	#define TRUE                                           ((BOOL) 1)
	#define FALSE                                          ((BOOL) 0)
#endif /* !FTR_DONT_DEFINE_SDK_BOOL */

/* RESULT */
#if !defined(FTR_DONT_DEFINE_SDK_RESULT)
	typedef unsigned long                                  RESULT;
	typedef unsigned long                                  STATUS;

	#define RESULT_OK                                      ((RESULT) 0)
	#define RESULT_FAIL                                    ((RESULT) 1)
#endif /* !FTR_DONT_DEFINE_SDK_RESULT */

/* BYTE / WORD / DWORD / QWORD */
#if !defined(FTR_DONT_DEFINE_SDK_BYTEWORDS)
	typedef unsigned char                                  BYTE;   /* 8-bit.  */
	typedef unsigned short                                 WORD;   /* 16-bit. */
	typedef unsigned long                                  DWORD;  /* 32-bit. */
	typedef unsigned long long                             QWORD;  /* 64-bit. */
#endif /* !FTR_DONT_DEFINE_SDK_BYTEWORDS */

// TODO: Should it moves to some ustr function file?
/* WCHAR */
#if !defined(FTR_DONT_DEFINE_SDK_WCHAR)
	typedef unsigned short                                 WCHAR;
#endif /* !FTR_DONT_DEFINE_SDK_WCHAR */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_BASE_H */
