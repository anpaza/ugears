/*
    libC-like functions for embedded systems
    Copyright (C) 2014-2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _CLIKE_H
#define _CLIKE_H

#include "useful.h"

/**
 * @file clike.h
 *      libC-like functions for embedded systems.
 *
 * This file provides prototypes for various libuseful functions that
 * are much like the functions from libC (there's no guarantee they
 * fully follow the standards, but enough for most uses).
 *
 * The functions can be declared underscored (e.g. _memcpy, _printf etc),
 * if they conflict with some other library you use (e.g. newlib etc).
 * The following macros can be used to influence this behavior:
 *
 * * USE_CLIKE - all functions are declared with their native libc-like
 *      names (e.g. memset(), strlen(), printf etc).
 * * USE_LIBC (conflicts with USE_CLIKE) - all functions are declared
 *      with an underscore prefix (e.g. _memset(), _strlen(), _printf()).
 *
 *      If USE_LIBC is defined, including <useful/clike.h> will include
 *      the respective libc header files so that both underscored and
 *      libc's non-underscored functions will be available for use.
 *      This is especially useful e.g. when testing function
 *      implementations in a "real" OS like Linux.
 */

#include "clike-defs.h"

// -------------------------------------------------------------------------- //

#ifdef USE_LIBC
#   include <stdio.h>
#endif

#include "printf.h"

// -------------------------------------------------------------------------- //

#ifdef USE_LIBC
#   include <stdlib.h>
#endif

/**
 * Set the starting point for rand().
 * This uses a global xs_rng_t.
 *
 * @arg seed The number that determines the pseudo-random sequence.
 */
EXTERN_C void CLIKE_P (srand) (unsigned seed);

/**
 * Get a pseudo-random number.
 * This uses a global xs_rng_t.
 *
 * @return A new random number in the range 0..MAX_UNSIGNED_INT
 */
EXTERN_C unsigned CLIKE_P (rand) ();

// -------------------------------------------------------------------------- //

#ifdef USE_LIBC
// Don't forget to define _GNU_SOURCE on compiler command line
// if you need memrchr()! This is not a standard function.
#   include <string.h>
#endif

/**
 * Return the length of a zero-terminated string.
 * This is an underscored function because strlen() is an optimized macro
 * that may call _strlen() in some cases.
 *
 * @param str A pointer to a string
 * @return String length, bytes
 */
EXTERN_C size_t _strlen (const char *str);

/**
 * Copy string from @a src to @a dest.
 *
 * @param dest Destination string
 * @param src Source string
 * @return destination string
 */
EXTERN_C char *CLIKE_P (strcpy) (char *dest, const char *src);

/**
 * Copy at most @a destlen characters from src to dest.
 * The resulting string may be non-zero-terminated if src doesn't contain
 * a zero amongst first @a destlen bytes.
 *
 * @param dest Destination string
 * @param src Source string
 * @param destlen Maximum capacity of dest, in bytes
 * @return destination string
 */
EXTERN_C char *CLIKE_P (strncpy) (char *dest, const char *src, size_t destlen);

/**
 * Fill the first @a len bytes of the memory area pointed to by
 * @a dest with the constant byte @a c.
 * @arg dest A pointer of memory to fill
 * @arg c The value to fill with
 * @arg len Number of bytes to fill
 */
EXTERN_C void *CLIKE_P (memset) (void *dest, int c, size_t len);

/**
 * Fill the first @a len bytes of the memory area pointed to by
 * @a dest with the zero constant.
 * @arg dest A pointer of memory to fill
 * @arg len Number of bytes to fill
 */
EXTERN_C void CLIKE_P (memclr) (void *dest, unsigned len);

/**
 * Optimized traditional memcpy().
 * @arg dest The destination pointer
 * @arg src Source pointer
 * @arg len Number of bytes to copy
 */
EXTERN_C void *CLIKE_P (memcpy) (void *dest, const void *src, size_t len);

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
EXTERN_C int CLIKE_P (memcmp) (const void *s1, const void *s2, size_t n);

/**
 * Scan given memory area, bottom-up, looking for given byte.
 * Finds the first occurence of the value in given memory area.
 * @param mem A pointer to memory area to search.
 * @param val The value to look for.
 * @param len The size of memory area pointed by @a mem.
 * @return A pointer to found value, or NULL if there's no byte with this
 *      value in searched memory area.
 */
EXTERN_C void *CLIKE_P (memchr) (const void *mem, int c, size_t size);

/**
 * Scan given memory area, top-down, looking for given byte.
 * Finds the last occurence of the value in given memory area.
 * @param mem A pointer to memory area to search.
 * @param val The value to look for.
 * @param len The size of memory area pointed by @a mem.
 * @return A pointer to found value, or NULL if there's no byte with this
 *      value in searched memory area.
 */
EXTERN_C void *CLIKE_P (memrchr) (const void *mem, int c, size_t size);

#ifdef USE_CLIKE
#   define strlen(s)    (__builtin_constant_p (*s) ? __builtin_strlen (s) : _strlen (s))
#endif

// -------------------------------------------------------------------------- //

#ifdef USE_LIBC
#   include <assert.h>
#endif

#ifdef USE_CLIKE
#   define assert(cond) do { if (!(cond)) _assert_abort (#cond); } while (0)
#endif

/**
 * This is what assert calls if condition is false.
 * On embedded systems this transforms into a debug breakpoint
 * in a endless loop.
 * @param msg A message (usually the expression that caused trouble)
 */
EXTERN_C void _assert_abort (const char *msg) __attribute__((noreturn));

#endif // _CLIKE_H
