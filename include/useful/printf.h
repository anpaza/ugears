/*
    Simple printf implementation for microcontrollers
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _PRINTF_H
#define _PRINTF_H

#include "useful.h"
#include <stdarg.h>

/**
 * @file printf.h
 *      A very basic s/printf implementation, supporting the most useful
 *      and a few non-standard conversion specifications.
 *
 * * %d print next arg as a signed integer
 * * %u print next arg as a unsigned integer
 * * %s print next arg as a zero-terminated string
 * * %c print next arg as a char
 * * %f print next arg as a signed fixed-point value
 * * %F print next arg as a unsigned fixed-point value
 * * l this modifier, if used before d or u, will interpret next arg as
 *      a long type (signed or unsigned)
 * * h this modifier, if used before d or u, will interpret next arg as
 *      a short type (signed or unsigned)
 * * hh this modifier, if used before d or u, will interpret next arg as
 *      a byte type (signed or unsigned)
 * * [width] if a number is specified before d, u, s, f, F modifiers,
 *      sets the width of the output field
 * * [.fracdigits] if a .number is specified before the f and F modifiers,
 *      sets the number of digits to print after the decimal point
 * * [.fracbits] if a ..number is specified before the f and F modifiers,
 *      sets the number of bits in the fractional part of the number
 *
 * The following macros, which can be set to 0 or 1 in your HARDWARE_H,
 * or can be left undefined to use standard settings (all enabled by default),
 * can be used to enable or disable specific functionality:
 *
 * USING_LIBC - define this to avoid definition of non-underscored macros
 *      for printf and other standard C functions. Define this to avoid
 *      conflicts with libc stdio.h, if you use it. You still can access
 *      this variant of printf by using respective underscored identifiers.
 * PRINTF_LONG_SUPPORT - enable support for the 'l' modifiers.
 *      'l' interprets next arg as (unsigned) long.
 * PRINTF_SHORT_SUPPORT - enable support for the 'h' and 'hh' modifiers.
 *      'h' interprets next arg as (unsigned) short, 'hh' as (unsigned) byte.
 * PRINTF_FP_SUPPORT - enable support for the 'f' and 'F' conversions.
 *
 * Fixed-point format is useful on MCUs without FPUs, when you split the 32
 * bits of an integer value into a integer and fractional part, each using
 * a fixed number of bits. For example, the FP12 format uses 12 lower bits
 * to represent fractional part, and upprer (32-12 = 20) bits to represent
 * the integer part. Now, say, 0x00012345 splits as integer part = 0x00012
 * and fractional part = 0x345/(2^12) ~= 0.20435, e.g. 0x12345 ~= 18.20435.
 *
 * The f and F formats are for printing this kind of numbers. For example,
 * the %.3.12f format will print a FP12 number with 3 digits after the decimal
 * dot, e.g. printf ("%.3.12f", 0x12345) will print "18.204" (integer part
 * 0x12 = 18, fractional part = 0x345 / 2^12 = 0.204).
 */

#ifndef PRINTF_LONG_SUPPORT
// Set to 1 for long format support (%l and %ll)
#define PRINTF_LONG_SUPPORT 1
#endif
// Uncomment for short format support (%h and %hh)
#define PRINTF_SHORT_SUPPORT
// Uncomment for fixed-point support (%[width].[fracdigits].[fracbits](f|F))
#define PRINTF_FP_SUPPORT

/**
 * This defines the backend functions that do actual low-level output.
 * If you need additional parameters passed to your backend, you can
 * store them as part of a larger backend structure, e.g.:
 *
 * @verbatim
 * struct my_backend
 * {
 *      printf_backend_t backend;
 *      void *data;
 *      int moredata;
 * };
 *
 * ...
 *
 * void (*putc) (printf_backend_t *self, char c)
 * {
 *      struct my_backend *myself = CONTAINER_OF (self, struct my_backend, backend);
 *      ...
 * }
 * @endverbatim
 */
typedef struct _printf_backend_t
{
    /**
     * The function that outputs a single character.
     * @param self A pointer to this printf_backend_t structure
     * @param c The character to echo
     */
    void (*putc) (struct _printf_backend_t *self, char c);

    /**
     * Flush the accumulation buffer, if supported.
     * This can be NULL (if the backend does not support buffering),
     * so you must check before using it.
     */
    void (*flush) (struct _printf_backend_t *self);
} printf_backend_t;

EXTERN_C printf_backend_t *printf_stdout;

/*
 * Unfortunately, we can't use __attribute__ ((format (printf, 1, 2)))
 * to check format strings at runtime because our format conversion
 * specifiers are not quite standard.
 */

/**
 * Define the low-level backend for printf() & company.
 *
 * @param stdout The backend for printing to stdout
 */
INLINE_ALWAYS void init_printf (printf_backend_t *stdout)
{ printf_stdout = stdout; }

/**
 * A general vprintf (), using a backend as the first argument
 * and a pointer to a list of format string arguments.
 *
 * @param fmt The C-style format string
 */
EXTERN_C void _vgprintf (printf_backend_t *backend,
                       const char *fmt, va_list va);

/**
 * A general printf (), using a backend as the first argument
 *
 * @param fmt The C-style format string
 */
EXTERN_C void _gprintf (printf_backend_t *backend, const char *fmt, ...);

/**
 * This is a snprintf () using same format strings as printf ().
 * The resulting string in buf is always zero-terminated, even if buf
 * doesn't contain enough space.
 *
 * @param buf The output buffer
 * @param size Output buffer size
 * @param fmt The C-style format string
 * @return The size of resulting string in buf, without zero terminator
 */
EXTERN_C size_t _snprintf (char *buf, size_t size, const char *fmt, ...);

/**
 * A variant of snprintf using va_list instead of varargs.
 *
 * @param buf The output buffer
 * @param size Output buffer size
 * @param fmt The C-style format string
 * @param va A pointer to variable arguments list
 * @return The size of resulting string in buf, without zero terminator
 */
EXTERN_C size_t _vsnprintf (char *buf, size_t size, const char *fmt, va_list va);

/**
 * The usual printf (), outputs via printf_stdout.
 *
 * @param fmt The C-style format string
 */
EXTERN_C void _printf (const char *fmt, ...);

/**
 * Output a single character via the stdout backend.
 */
EXTERN_C void _putchar (char c);

/**
 * Output a string followed by \n.
 * @arg str The string.
 */
EXTERN_C void _puts (const char *str);

/**
 * Flush the stdout buffer, if any
 */
EXTERN_C void _fflush (void);

#ifndef USING_LIBC
#  define printf    _printf
#  define snprintf  _snprintf
#  define putchar   _putchar
#  define puts      _puts
#  define fflush    _fflush
#endif

#endif // _PRINTF_H
