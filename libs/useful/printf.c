/*
    Simple printf implementation for microcontrollers
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"
#include "useful/printf.h"

printf_backend_t *printf_stdout;

#if PRINTF_LONG_SUPPORT
typedef signed long int_t;
typedef unsigned long uint_t;
#define __SIZEOF_INT_T__  __SIZEOF_LONG__
#else
typedef signed int int_t;
typedef unsigned int uint_t;
#define __SIZEOF_INT_T__  __SIZEOF_INT__
#endif

static char *u2a (uint_t num, unsigned base, bool upper, char *out)
{
    if (num == 0)
        *out++ = '0';
    else
    {
        // Find the order of magnitude of the number
        uint_t value = base;
        unsigned order = 1;
        while (value <= num)
        {
            uint_t old_value = value;
            value *= base;
            order++;
            if (value < old_value)
                // overflow
                break;
        }

        // Output digits in reverse direction
        for (unsigned n = order; n > 0; n--)
        {
            unsigned digit = num % base;
            num /= base;
            out [n - 1] = digit + (digit < 10 ? '0' : (upper ? 'A' : 'a') - 10);
        }
        out += order;
    }

    return out;
}

static char *s2a (int_t num, char *out)
{
    if (num < 0)
    {
        num = -num;
        *out++ = '-';
    }

    return u2a (num, 10, 0, out);
}

#if PRINTF_FP_SUPPORT

static uint8_t log2 [] = {4, 7, 10, 14, 17, 20, 24, 27, 30};

static char *ufp2a (uint_t num, unsigned fdig, unsigned fbits, char *out)
{
    out = u2a (num >> fbits, 10, 0, out);
    if (fdig == 0)
    {
        // log10 (1 << fbits)
        for (fdig = 0; fdig < ARRAY_LEN (log2); fdig++)
            if (fbits < log2 [fdig])
                break;
        fdig++;
    }

    *out++ = '.';
    while (fdig > 0)
    {
        num = (num & ((1 << fbits) - 1)) * 10;
        *out++ = (num >> fbits) + '0';
        fdig--;
    }

    return out;
}

static char *sfp2a (int_t num, unsigned fdig, unsigned fbits, char *out)
{
    if (num < 0)
    {
        num = -num;
        *out++ = '-';
    }

    return ufp2a (num, fdig, fbits, out);
}

#endif

static char a2i (char ch, const char **src, uint8_t *val)
{
    const char *lsrc = *src;
    uint8_t num = 0;
    while ((ch >= '0') && (ch <= '9'))
    {
        num = num * 10 + (ch - '0');
        ch = *lsrc++;
    }
    *src = lsrc;
    *val = num;
    return ch;
}

static void format_out (printf_backend_t *backend,
                        uint8_t width, bool leading_zeros,
                        char *value, size_t value_len)
{
    if (width == 0)
        width = value_len;
    else if (value_len > width)
        value_len = width;

    char fill = ' ';
    if (leading_zeros)
    {
        fill = '0';
        if (*value == '-')
        {
            backend->putch (backend, *value++);
            width--; value_len--;
        }
    }

    for (; width > value_len; width--)
        backend->putch (backend, fill);

    for (; width != 0; width--)
        backend->putch (backend, *value++);
}

void CLIKE_P (vgprintf) (printf_backend_t *backend, const char *fmt, va_list va)
{
    if (!backend)
        return;

    // buffer for numeric conversions, sign + max digits for base 10
    char buff [1 + ((__SIZEOF_INT_T__ == 2) ? 5 :
                    (__SIZEOF_INT_T__ == 4) ? 10 : 20)];

    for (;;)
    {
        char ch = *fmt++;
        if (ch == '\0')
            break;

        if (ch != '%')
        {
            backend->putch (backend, ch);
            continue;
        }

        uint8_t width = 0;
        bool leading_zeros = false;
        enum
        {
#if PRINTF_SHORT_SUPPORT
            fbyte,
            fshort,
#endif
            fint,
#if PRINTF_LONG_SUPPORT
            flong,
#endif
        } argsize = fint;
#if PRINTF_FP_SUPPORT
        uint8_t fdig = 0;
        uint8_t fbits = 12;
#endif

        ch = *fmt++;
        if (ch == '0')
        {
            leading_zeros = true;
            ch = *fmt++;
        }
        if (ch >= '0' && ch <= '9')
        {
            ch = a2i (ch, &fmt, &width);
        }
#if PRINTF_FP_SUPPORT
        if (ch == '.')
        {
            ch = a2i ('0', &fmt, &fdig);
        }
        if (ch == '.')
        {
            ch = a2i ('0', &fmt, &fbits);
        }
#endif
#if PRINTF_LONG_SUPPORT
        if (ch == 'l')
        {
            argsize = flong;
            ch = *fmt++;
        }
#endif
#if PRINTF_SHORT_SUPPORT
        if (ch == 'h')
        {
            argsize = fshort;
            ch = *fmt++;
            if (ch == 'h')
            {
                argsize = fbyte;
                ch = *fmt++;
            }
        }
#endif
        switch (ch)
        {
            case 0:
                goto abort;

            case 'd' :
            case 'u' :
            case 'x': case 'X' :
#if PRINTF_FP_SUPPORT
            case 'f' :
            case 'F' :
#endif
            {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                union
                {
                    uint_t u;
                    int_t s;
                    signed char schar;
                    unsigned char uchar;
                    signed short sshort;
                    unsigned short ushort;
                    signed int sint;
                    unsigned int uint;
                } val;
#else
#  error "Big endian CPUs are not supported!"
#endif
                char *end;

                val.u =
#if PRINTF_LONG_SUPPORT && (__SIZEOF_LONG__ > __SIZEOF_INT__)
                        (argsize == flong) ? va_arg (va, unsigned long) :
#endif
                        va_arg (va, unsigned);

                // char and short are promoted to int in varargs
                if (ch == 'd' || ch == 'f')
                {
                    if (0) ;
#if PRINTF_LONG_SUPPORT &&  (__SIZEOF_LONG__ > __SIZEOF_INT__)
                    else if (argsize == fint)
                        // sign extend int -> long
                        val.s = val.sint;
#endif
#if PRINTF_SHORT_SUPPORT
                    else if (argsize == fshort)
                        val.s = val.sshort;
                    else if (argsize == fbyte)
                        val.s = val.schar;
#endif
                }
                else
                {
                    if (0) ;
#if PRINTF_SHORT_SUPPORT
                    else if (argsize == fshort)
                        val.u = val.ushort;
                    else if (argsize == fbyte)
                        val.u = val.uchar;
#endif
                }

                switch (ch)
                {
                    case 'd': end = s2a (val.s, buff); break;
                    case 'u': end = u2a (val.u, 10, false, buff); break;
#if PRINTF_FP_SUPPORT
                    case 'F': end = ufp2a (val.s, fdig, fbits, buff); break;
                    case 'f': end = sfp2a (val.s, fdig, fbits, buff); break;
#endif
                    default: end = u2a (val.u, 16, (ch == 'X'), buff); break;
                }

                format_out (backend, width, leading_zeros, buff, end - buff);
                break;
            }

            case 's' :
            {
                char *str = va_arg (va, char *);
                format_out (backend, width, false, str, strlen (str));
                break;
            }

            case 'c' :
                backend->putch (backend, (char)(va_arg (va, int)));
                break;

            case '%' :
                backend->putch (backend, ch);
                break;

            default:
                // unknown conversion specification
                break;
        }
    }
abort:;
}

void CLIKE_P (gprintf) (printf_backend_t *backend, const char *fmt, ...)
{
    va_list va;
    va_start (va, fmt);
    CLIKE_P (vgprintf) (backend, fmt, va);
    va_end (va);
}

int CLIKE_P (printf) (const char *fmt, ...)
{
    va_list va;
    va_start (va, fmt);
    CLIKE_P (vgprintf) (printf_stdout, fmt, va);
    va_end (va);
    return 0; // not standard, but who cares...
}

typedef struct
{
    printf_backend_t be;
    char *cur;
    char *end;
} sprintf_backend_t;

static void sprintf_putc (printf_backend_t *backend, char c)
{
    sprintf_backend_t *myself = CONTAINER_OF (backend, sprintf_backend_t, be);
    if (myself->cur < myself->end)
        *myself->cur++ = c;
}

int CLIKE_P (vsnprintf) (char *buf, size_t size, const char *fmt, va_list va)
{
    sprintf_backend_t sprintf_backend;
    sprintf_backend.be.putch = sprintf_putc;
    sprintf_backend.cur = buf;
    sprintf_backend.end = buf + size - 1;

    CLIKE_P (vgprintf) (&sprintf_backend.be, fmt, va);

    *sprintf_backend.cur = 0;
    return sprintf_backend.cur - buf;
}

int CLIKE_P (snprintf) (char *buf, size_t size, const char *fmt, ...)
{
    va_list va;
    va_start (va, fmt);
    size_t ret = CLIKE_P (vsnprintf) (buf, size, fmt, va);
    va_end (va);

    return ret;
}

int CLIKE_P (putchar) (int c)
{
    printf_stdout->putch (printf_stdout, c);
    return c;
}

int CLIKE_P (puts) (const char *s)
{
    while (*s)
        CLIKE_P (putchar) (*s++);
    CLIKE_P (putchar) ('\n');
    return 1;
}

void CLIKE_P (fflush) ()
{
    if (printf_stdout->flush)
        printf_stdout->flush (printf_stdout);
}
