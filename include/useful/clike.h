/*
    libC-like functions for embedded systems
    Copyright (C) 2014-2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _CLIKE_H
#define _CLIKE_H

#include <useful/useful.h>

/**
 * @file clike.h
 *      libC-like functions for embedded systems.
 *
 * This file provides prototypes for various libuseful functions that
 * are much like the functions from libC (there's no guarantee they
 * fully follow the standards, but enough for most uses).
 *
 * The functions are declared underscored (e.g. _memcpy, _printf etc),
 * and macros are used to give them the normally expected names.
 * There are a few macros that influence this behavior:
 *
 * * CLIKE_ORIG - declare only original function names (e.g. underscored).
 *      Use this if macros interfere with something else you're using
 *      (perhaps a real libc or whatever).
 * * USING_LIBC (implies CLIKE_ORIG) - include the standard libc header
 *      files implementing functions declared here. This way, you can use
 *      libc when compiling for a "real" OS, and use these simplified
 *      implementations when compiling for embedded systems.
 */

#if USING_LIBC
#   if !CLIKE_ORIG
#       define CLIKE_ORIG 1
#   endif
#   define _GNU_SOURCE
#endif

// -------------------------------------------------------------------------- //

#if USING_LIBC
#   include <stdio.h>
#endif

#include "printf.h"

#if !CLIKE_ORIG
#  define printf    _printf
#  define snprintf  _snprintf
#  define putchar   _putchar
#  define puts      _puts
#  define fflush    _fflush
#endif

// -------------------------------------------------------------------------- //

#if USING_LIBC
#   include <stdlib.h>
#endif

/**
 * Set the starting point for rand().
 * This uses a global xs_rng_t.
 *
 * @arg seed The number that determines the pseudo-random sequence.
 */
extern void _srand (unsigned seed);

/**
 * Get a pseudo-random number.
 * This uses a global xs_rng_t.
 *
 * @return A new random number in the range 0..MAX_UNSIGNED_INT
 */
extern unsigned _rand ();

#if !CLIKE_ORIG
#  define srand         _srand
#  define rand          _rand
#endif

// -------------------------------------------------------------------------- //

#if USING_LIBC
#   include <string.h>
#endif

/**
 * Return the length of a zero-terminated string
 *
 * @param str A pointer to a string
 * @return String length, bytes
 */
extern size_t _strlen (const char *str);

/**
 * Fill the first @a len bytes of the memory area pointed to by
 * @a dest with the constant byte @a c.
 * @arg dest A pointer of memory to fill
 * @arg c The value to fill with
 * @arg len Number of bytes to fill
 */
extern void _memset (void *dest, char c, unsigned len);

/**
 * Fill the first @a len bytes of the memory area pointed to by
 * @a dest with the zero constant.
 * @arg dest A pointer of memory to fill
 * @arg len Number of bytes to fill
 */
extern void memclr (void *dest, unsigned len);

/**
 * Optimized traditional memcpy().
 * @arg dest The destination pointer
 * @arg src Source pointer
 * @arg len Number of bytes to copy
 */
extern void _memcpy (void *dest, const void *src, unsigned len);

/**
 * Optimized traditional memcmp().
 * Compare two memory areas.
 *
 * @param s1 A pointer to memory area 1
 * @param s2 A pointer to memory area 2
 * @param n The size of both memory areas
 * @return 0 if contents of memory is equal, negative number if first
 *      different byte in s1 is less than corresponding byte from s2, or
 *      a positive number if first different byte in s1 is greater than
 *      corresponding byte from s2.
 */
extern int _memcmp (const void *s1, const void *s2, size_t n);

/**
 * Scan given memory area, bottom-up, looking for given byte.
 * Finds the first occurence of the value in given memory area.
 * @param mem A pointer to memory area to search.
 * @param val The value to look for.
 * @param len The size of memory area pointed by @a mem.
 * @return A pointer to found value, or NULL if there's no byte with this
 *      value in searched memory area.
 */
extern void *_memchr (const void *mem, uint8_t val, size_t size);

/**
 * Scan given memory area, top-down, looking for given byte.
 * Finds the last occurence of the value in given memory area.
 * @param mem A pointer to memory area to search.
 * @param val The value to look for.
 * @param len The size of memory area pointed by @a mem.
 * @return A pointer to found value, or NULL if there's no byte with this
 *      value in searched memory area.
 */
extern void *_memrchr (const void *mem, uint8_t val, size_t size);

#if !CLIKE_ORIG
#  define strlen(s)     (__builtin_constant_p (*s) ? __builtin_strlen (s) : _strlen (s))
#  define memset        _memset
#  define memcpy        _memcpy
#  define memcmp        _memcmp
#  define memchr        _memchr
#  define memrchr       _memrchr
#endif

// -------------------------------------------------------------------------- //

#if USING_LIBC
#   include <assert.h>
#endif

#define _assert(cond)   do { if (!(cond)) _assert_abort (#cond); } while (0)

/**
 * This is what assert calls if condition is false.
 * On embedded systems this transforms into a debug breakpoint
 * in a endless loop.
 * @param msg A message (usually the expression that caused trouble)
 */
extern void _assert_abort (const char *msg) __attribute__((noreturn));

#if !CLIKE_ORIG
#   define assert(c)    _assert(c)
#endif

#endif // _CLIKE_H
