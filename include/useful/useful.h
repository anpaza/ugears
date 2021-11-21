/*
    Useful function for embedded systems
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __USEFUL_H__
#define __USEFUL_H__

/**
 * @file useful.h
 *      The 'useful' library provides a number of useful functions,
 *      balancedly optimized for speed/size. Some functions are
 *      like their ANSI C counterparts, but they aren't meant
 *      to be 100% compliant with the standard.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

// Optimize by speed (1) or size (0) where appropiate
#define USEFUL_OPTIMIZE			1

#if !defined __BYTE_ORDER__
#  define __ORDER_LITTLE_ENDIAN__	3412
#  define __ORDER_BIG_ENDIAN__		1234

#if defined ARCH_X86 || defined ARCH_X86_64 || defined ARCH_ARM
# define __BYTE_ORDER__			__ORDER_LITTLE_ENDIAN__
#endif
#endif // __BYTE_ORDER__

/// Get the number of elements in a static array
#define ARRAY_LEN(x)		(sizeof (x) / sizeof (x [0]))

/// Structure attribute to avoid aligning structure members
#define PACKED			__attribute__ ((packed))

/// Use for inline functions in header files
#define INLINE_ALWAYS           inline __attribute__((always_inline))

/// Get the offset of member m in compound type t
#define OFFSETOF(t, m)		__builtin_offsetof (t, m)

/// Get a pointer to the container structure from an offset of a field
#define CONTAINER_OF(ptr, type, member) \
	((type *)((char *)ptr - offsetof (type, member)))

#define _STRINGIFY(x)		#x
/// Convert x -> "x"
#define STRINGIFY(x)		_STRINGIFY(x)

/// Return the absolute value of a integer
#define ABS(x)			({ typeof (x) __z = x; if (__z < 0) __z = -__z; __z; })

/// Exchange two values of the same type
#define XCHG(x, y)		{ typeof (x) __z = x; x = y; y = __z; }

/// Bit Value: convert bit number to bit mask
#define BV(x)			(1 << (x))

/**
 * Return the largest of two values
 */
#define MAX(x,y) \
    ({ \
        __typeof__ (x) _x = x; \
        __typeof__ (y) _y = y; \
        _x > _y ? _x : _y; \
    })

/**
 * Return the smallest of two values
 */
#define MIN(x,y) \
    ({ \
        __typeof__ (x) _x = x; \
        __typeof__ (y) _y = y; \
        _x < _y ? _x : _y; \
    })

#define _JOIN2(a,b)		a##b
/// Join two tokens together and interpret the result as a new token
#define JOIN2(a,b)		_JOIN2(a, b)

#define _JOIN3(a,b,c)		a##b##c
/// Join three tokens together and interpret the result as a new token
#define JOIN3(a,b,c)		_JOIN3(a,b,c)

#define _JOIN4(a,b,c,d)		a##b##c##d
/// Join four tokens together and interpret the result as a new token
#define JOIN4(a,b,c,d)		_JOIN4(a,b,c,d)

#define _JOIN5(a,b,c,d,e)	a##b##c##d##e
/// Join five tokens together and interpret the result as a new token
#define JOIN5(a,b,c,d,e)	_JOIN5(a,b,c,d,e)

#define _JOIN6(a,b,c,d,e,f)	a##b##c##d##e##f
/// Join six tokens together and interpret the result as a new token
#define JOIN6(a,b,c,d,e,f)	_JOIN6(a,b,c,d,e,f)

#if defined __DEBUG__
#  if defined ARCH_ARM && (defined CORTEX_M3 || defined CORTEX_M4)
/// Break into debugger
#    define DEBUG_BREAK		__asm__ __volatile__ ("bkpt #0")
/// Break into debugger if condition @a c is true
#    define DEBUG_BREAK_IF(c)	if (c) DEBUG_BREAK
#  endif
#endif

#if !defined DEBUG_BREAK
#  define DEBUG_BREAK
#  define DEBUG_BREAK_IF(c)
#endif

// Make headers compatible with both C and C++
#ifdef __cplusplus
#  define EXTERN_C extern "C"
#else
#  define EXTERN_C extern
#endif

// Arch-dependent useful inline functions
#if defined ARCH_X86 && defined __GNUC__
#  include "useful-x86.h"
#endif
#if defined ARCH_ARM && defined __GNUC__
#  include "useful-arm.h"
#endif

// Generic implementations for missing arch-dependent inline functions
#include "useful-generic.h"

typedef union
{
    uint16_t u16;
    uint32_t u32;
}
#ifdef __GNUC__
    __attribute__((may_alias))
#endif
    __aliasing_through;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
/// Получить little-endian uint32_t из массива data со смещением ofs байт.
#  define GET_UINT32_LE(data,ofs)	((__aliasing_through *)((char *)(data) + (ofs)))->u32
/// Получить big-endian uint32_t из массива data со смещением ofs байт.
#  define GET_UINT32_BE(data,ofs)	bswap32 (GET_UINT32_LE ((data), (ofs)))
/// Записать little-endian uint32_t в массива data по смещению ofs байт.
#  define PUT_UINT32_LE(data,ofs,x)	(((__aliasing_through *)((char *)(data) + (ofs)))->u32 = (x))
/// Записать big-endian uint32_t в массива data по смещению ofs байт.
#  define PUT_UINT32_BE(data,ofs,x)	PUT_UINT32_LE (data, ofs, bswap32 (x))
/// Получить little-endian uint16_t из массива data со смещением ofs байт.
#  define GET_UINT16_LE(data,ofs)	((__aliasing_through *)((char *)(data) + (ofs)))->u16
/// Получить big-endian uint16_t из массива data со смещением ofs байт.
#  define GET_UINT16_BE(data,ofs)	bswap16 (GET_UINT16_LE ((data), (ofs)))
/// Преобразовать 32-х битное число в формат little-endian.
#  define UINT32_LE(x)			(x)
/// Преобразовать 32-х битное число в формат big-endian.
#  define UINT32_BE(x)			bswap32 (x)
/// Преобразовать 16-х битное число в формат little-endian.
#  define UINT16_LE(x)			(x)
/// Преобразовать 16-х битное число в формат big-endian.
#  define UINT16_BE(x)			bswap16 (x)
#else
#  define GET_UINT32_LE(data,ofs)	bswap32 (GET_UINT32_BE ((data), (ofs)))
#  define GET_UINT32_BE(data,ofs)	((__aliasing_through *)((char *)(data) + (ofs)))->u32
#  define PUT_UINT32_LE(data,ofs,x)	PUT_UINT32_BE (data, ofs, bswap32 (x))
#  define PUT_UINT32_BE(data,ofs,x)	(((__aliasing_through *)((char *)(data) + (ofs)))->u32 = (x))
#  define GET_UINT16_LE(data,ofs)	bswap16 (GET_UINT16_BE (data, ofs))
#  define GET_UINT16_BE(data,ofs)	((__aliasing_through *)((char *)(data) + (ofs)))->u16
#  define UINT32_LE(x)			bswap32 (x)
#  define UINT32_BE(x)			(x)
#  define UINT16_LE(x)			bswap16 (x)
#  define UINT16_BE(x)			(x)
#endif

#endif // __USEFUL_H__
