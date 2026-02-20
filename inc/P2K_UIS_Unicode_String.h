#ifndef P2K_UIS_UNICODE_STRING_H
#define P2K_UIS_UNICODE_STRING_H

#include "P2K_SDK_Base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef UINT16                                             UIS_STRING_T;

/*
 * Convert a Unicode string to a long.
 *
 * `string` - String to convert.
 *
 * Returns the long value for this string, if valid.
 *
 * Note: The radix for this call is 10 and cannot be set.
 */
extern INT32 u_atol(
	UIS_STRING_T *string
);

/*
 * Convert an ASCII character array to a Unicode string
 *
 * `ascii` - Char string to convert.
 * `unicode` - The converted Unicode string.
 *
 * Returns - The converted Unicode string.
 *
 * Note: The Unicode string value must be pre-initialized to the correct length prior to calling this function.
 *       The results of this function are undefined when given a char array outside of the ASCII range.
 */
extern UIS_STRING_T *u_atou(
	UINT8 *ascii,
	UIS_STRING_T *unicode
);

/*
 * Determines if the given character is a aphabetic character.
 *
 * `c` - The character to check.
 *
 * Returns - TRUE if the character is alphabetic, FALSE if not.
 */
extern BOOL u_isAlpha(
	UIS_STRING_T c
);

/*
 * Determines if the given character is a digit.
 *
 * `c` - The character to check.
 *
 * Returns - TRUE if the character is a digit, FALSE if not.
 */
extern BOOL u_isDigit(
	UIS_STRING_T c
);

/*
 * Determines if the given character is a space character.
 *
 * `c` - The character to check.
 *
 * Returns - TRUE if the character is a space, FALSE if not.
 */
extern BOOL u_isSpace(
	UIS_STRING_T c
);

/*
 * Determines if the given character is an upper case character. This characteristic is defined for Latin.
 *
 * `c` - The character to check.
 *
 * Returns - TRUE if the character is upper case, FALSE if the character is not upper,
 *           or not found in one of the supported character sets.
 */
extern BOOL u_isUpper(
	UIS_STRING_T c
);

/*
 * Determines if the given character is a lower case character. This characteristic is defined for Latin.
 *
 * `c` - The character to check.
 *
 * Returns - TRUE if the character is lower case, FALSE if the character is not lower,
 *           or not found in one of the supported character sets.
 */
extern BOOL u_isLower(
	UIS_STRING_T c
);

/*
 * Convert an integer value to single UIS_STRING_T character.
 *
 * `value` - The integer value to convert.
 * `string` - The result of the function.
 *
 * Note: This method accepts 16-bit integers.
 */
extern UIS_STRING_T *u_itoa(
	INT16 value,
	UIS_STRING_T *string
);

/**
* Convert an unsigned integer value to single UIS_STRING_T character
* value - the unsigned integer value to convert
* string - the result of the function.
* Note: this method accepts unsigned 16-bit integers
*/
extern UIS_STRING_T *u_uitoa(
	UINT16 value,
	UIS_STRING_T *string
);

/*
 * Convert an integer value to a Unicode upper hex case string.
 *
 * `value` - The integer 32 bit value to convert.
 * `string` - The result of the function.
 *
 * Note: Buffer of 12 bytes must be allocated by the caller.
 *       The radix for this function is fixed at 16.
 */
extern UIS_STRING_T *u_itoh(
	INT32 value,
	UIS_STRING_T *buffer
);

/*
 * Convert a signed long integer value to single UIS_STRING_T character.
 *
 * `value` - The long integer value to convert.
 * `string` - The result of the function.
 *
 * Returns - The result of the function.
 *
 * Note: This method accepts signed 32-bit integers
 */
extern UIS_STRING_T *u_ltou(
	INT32 value,
	UIS_STRING_T *unicode
);

/*
 * Concatenate two wide strings. Appends a copy of `src`, including the null terminator, to `dst`.
 * The initial copied character from `src` overwrites the null terminator in `dst`.
 *
 * `dst` - The destination string.
 * `src` - The source string.
 *
 * Returns - A pointer to `dst`.
 */
extern UIS_STRING_T *u_strcat(
	UIS_STRING_T *dst,
	const UIS_STRING_T *src
);

/**
 * Concatenate two ustrings.
 * Appends at most <TT>n</TT> characters from <TT>src</TT> to <TT>dst</TT>.
 * Adds a null terminator.
 * @param dst The destination string.
 * @param src The source string.
 * @param n The maximum number of characters to compare.
 * @return A pointer to <TT>dst</TT>.
 */
extern UIS_STRING_T *u_strncat(
	UIS_STRING_T *dst,
	const UIS_STRING_T *src,
	INT32 n
);

/*
 * Find the first occurrence of a specified character in a wide string.
 *
 * `s` - The string to search.
 * `c` - The character to find.
 *
 * Returns - A pointer to the first occurrence of `c` in `s`, or a null pointer if `s` does not contain `c`.
 */
extern UIS_STRING_T *u_strchr(
	const UIS_STRING_T *s,
	UIS_STRING_T c
);

/*
 * Compare two wide strings for bitwise equality.
 *
 * `s1` - A string to compare.
 * `s2` - A string to compare.
 *
 * Returns 0 if `s1` and `s2` are bitwise equal; a negative value if `s1` is bitwise less than `s2`;
 * a positive value if `s1` is bitwise greater than `s2`.
 */
extern INT32 u_strcmp(
	const UIS_STRING_T *s1,
	const UIS_STRING_T *s2
);

/**
 * Compare two ustrings for bitwise equality.
 * Compares at most <TT>n</TT> characters.
 * @param s1 A string to compare.
 * @param s2 A string to compare.
 * @param n The maximum number of characters to compare.
 * @return 0 if <TT>s1</TT> and <TT>s2</TT> are bitwise equal; a negative
 * value if <TT>s1</TT> is bitwise less than <TT>s2,/TT>; a positive
 * value if <TT>s1</TT> is bitwise greater than <TT>s2,/TT>.
 */
extern INT32 u_strncmp(
	const UIS_STRING_T *ucs1,
	const UIS_STRING_T *ucs2,
	INT32 n
);

/*
 * Compare two not NULL-terminated strings in case insensitive manner
 * stopping at the end of any string ("aab" is equal to "aa")
 */
extern INT32 u_strncmpi(
	const UIS_STRING_T *str1,
	INT32 len1,
	const UIS_STRING_T *str2,
	INT32 len2
);

/*
 * Copy a wide string. Adds a null terminator.
 *
 * `dst` - The destination string.
 * `src` - The source string.
 * @return A pointer to <TT>dst</TT>.
 */
extern UIS_STRING_T *u_strcpy(
	UIS_STRING_T *dst,
	const UIS_STRING_T *src
);

/**
 * Copy a ustring.
 * Copies at most <TT>n</TT> characters.  The result will be null terminated
 * if the length of <TT>src</TT> is less than <TT>n</TT>.
 * @param dst The destination string.
 * @param src The source string.
 * @param n The maximum number of characters to copy.
 * @return A pointer to <TT>dst</TT>.
 */
extern UIS_STRING_T *u_strncpy(
	UIS_STRING_T *dst,
	const UIS_STRING_T *src,
	INT32 n
);

/*
 * This function will delete string segment.
 * The characters to delete specifies a position and the number of characters.
 */
extern INT32 u_strcut(
	UIS_STRING_T *str,
	WORD len,
	WORD pos,
	INT32 num
);

/*
 * This function will search a string for a specified substring in a case insensitive manner.
 */
extern UIS_STRING_T *u_stristr(
	UIS_STRING_T *p_source,
	UIS_STRING_T *p_search
);

/*
 * Determine the length of an array of UIS_STRING_T
 * @param s The array of UIS_STRING_Ts, NULL (U+0000) terminated.
 * @return The number of UIS_STRING_Ts in <TT>chars</TT>, minus the terminator.
 */
extern INT32 u_strlen(
	const UIS_STRING_T *s
);

/**
 * This function will convert each letter in a string to lower case.  The pointer to
 * the string should not be SYN_NULL.
 * @param str The pointer to a string that needs to be converted to lower case.
 * @return A pointer to str.
 */
extern UIS_STRING_T *u_strmakelower(
	UIS_STRING_T *str
);

/**
 * This function will capitalize each letter in a string.  The pointer to the string
 * should not be SYN_NULL.
 * @param str The pointer to a string that needs to be capitalized.
 * @return A pointer to str.
 */
extern UIS_STRING_T *u_strmakeupper(
	UIS_STRING_T *str
);

/**
 * Find the last occurance of a character within a string.
 *  src The source string.
 *  c   The character to search for.
 * return A pointer to c or null if not found.
 */
extern UIS_STRING_T *u_strrchr(
	UIS_STRING_T *src,
	const UIS_STRING_T c
);

/* This function will search a string for a specified substring in a case sensitive manner */
extern UIS_STRING_T *u_strstr(
	UIS_STRING_T *p_source,
	UIS_STRING_T *p_search
);

/**
* Convert a 64-bit unsigned integer value to a UIS_STRING_T string
* value - the 64-bit unsigned integer value to convert
* string - the result of the function.
* returns - the result of the function.
* Note: this method accepts 32-bit integers
*/
extern UIS_STRING_T *u_uint64tou(
	UINT64 value,
	UIS_STRING_T *unicode
);

/** LIBaa06657 - added support for unsigned 32-bit integers
* Convert a unsigned long integer value to single UIS_STRING_T character
* value - the unsigned long 32-bit integer value to convert
* string - the result of the function.
* returns - the result of the function.
* Note: this method accepts unsigned 32-bit integers
*/
extern UIS_STRING_T *u_ultou(
	UINT32 value,
	UIS_STRING_T *unicode
);

/*
 * TODO:
	0946 A u_utoa
	0941 A u_strsize
	0931 A u_strdup
	0925 A u_str_to_a
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !P2K_UIS_UNICODE_STRING_H */
