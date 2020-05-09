/*
    File: printf.c

    Copyright (C) 2004  Kustaa Nyholm

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdint.h>
#include "printf.h"
#undef putc

printf_backend_t *stdout_backend;

#ifdef PRINTF_LONG_SUPPORT

static char *uli2a (unsigned long int num, unsigned int base, int uc, char * bf)
{
    int n = 0;
    unsigned int d = 1;
    while (num / d >= base)
        d *= base;
    while (d != 0)
    {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0|| d == 0)
        {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
    return bf;
}

static char *li2a (long num, char * bf)
{
    if (num < 0)
    {
        num = -num;
        *bf++ = '-';
    }
    return uli2a (num, 10, 0, bf);
}

#endif

static char *ui2a (unsigned int num, unsigned int base, int uc, char * bf)
{
    int n = 0;
    unsigned int d = 1;
    while (num / d >= base)
        d *= base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0)
        {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
    return bf;
}

static char *i2a (int num, char * bf)
{
    if (num < 0)
    {
        num = -num;
        *bf++ = '-';
    }
    return ui2a (num, 10, 0, bf);
}

#ifdef PRINTF_FP_SUPPORT
static char *ufp2a (unsigned int num, int fdig, int fbits, char *bf)
{
    bf = ui2a (num >> fbits, 10, 0, bf);
    *bf++ = '.';
    while (fdig > 0)
    {
        num = (num & ((1 << fbits) - 1)) * 10;
        *bf++ = (num >> fbits) + '0';
        fdig--;
    }

    *bf = 0;
    return bf;
}

static char *fp2a (int num, int fdig, int fbits, char *bf)
{
    if (num < 0)
    {
        num = -num;
        *bf++ = '-';
    }
    return ufp2a (num, fdig, fbits, bf);
}
#endif

static int a2d (char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        return -1;
}

static char a2i (char ch, const char** src, int base, uint8_t *nump)
{
    const char *p = *src;
    uint8_t num = 0;
    int digit;
    while ((digit = a2d (ch)) >= 0)
    {
        if (digit > base) break;
        num = num * base + digit;
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}

static void putchw (printf_backend_t *backend, int n, char z, char *bf)
{
    char fc = z ? '0' : ' ';
    char ch;
    char *p = bf;
    while (*p++ && n > 0)
        n--;
    while (n-- > 0)
        backend->putc (backend, fc);
    while ((ch = *bf++))
        backend->putc (backend, ch);
}

void tfp_format (printf_backend_t *backend, const char *fmt, va_list va)
{
    char bf[16];

    char ch;

    while ((ch = *(fmt++)))
    {
        if (ch != '%')
            backend->putc (backend, ch);
        else
        {
            char lz = 0;
#ifdef PRINTF_LONG_SUPPORT
            char lng = 0;
#endif
#ifdef PRINTF_FP_SUPPORT
            uint8_t fdig = 0;
            uint8_t fbits = 12;
#endif
            uint8_t w = 0;
            ch = *(fmt++);
            if (ch == '0')
            {
                ch = *(fmt++);
                lz = 1;
            }
            if (ch >= '0' && ch <= '9')
            {
                ch = a2i (ch, &fmt, 10, &w);
            }
#ifdef PRINTF_FP_SUPPORT
            if (ch == '.')
            {
                ch = a2i ('0', &fmt, 10, &fdig);
            }
            if (ch == '.')
            {
                ch = a2i ('0', &fmt, 10, &fbits);
            }
#endif
#ifdef PRINTF_LONG_SUPPORT
            if (ch == 'l')
            {
                ch = *(fmt++);
                lng = 1;
            }
#endif
            switch (ch)
            {
                case 0:
                    goto abort;

                case 'u' :
#ifdef PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a (va_arg (va, unsigned long int), 10, 0, bf);
                    else
#endif
                        ui2a (va_arg (va, unsigned int), 10, 0, bf);
                    putchw (backend, w, lz, bf);
                    break;

                case 'd' :
#ifdef PRINTF_LONG_SUPPORT
                    if (lng)
                        li2a (va_arg (va, unsigned long int), bf);
                    else
#endif
                        i2a (va_arg (va, int), bf);
                    putchw (backend, w, lz, bf);
                    break;

                case 'x': case 'X' :
#ifdef PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a (va_arg (va, unsigned long int), 16, (ch=='X'), bf);
                    else
#endif
                        ui2a (va_arg (va, unsigned int), 16, (ch=='X'), bf);
                    putchw (backend, w, lz, bf);
                    break;

#ifdef PRINTF_FP_SUPPORT
                case 'f' :
                    fp2a (va_arg (va, int), fdig, fbits, bf);
                    putchw (backend, w, lz, bf);
                    break;

                case 'F' :
                    ufp2a (va_arg (va, unsigned int), fdig, fbits, bf);
                    putchw (backend, w, lz, bf);
                    break;
#endif

                case 'c' :
                    backend->putc (backend, (char)(va_arg (va, int)));
                    break;

                case 's' :
                    putchw (backend, w, 0, va_arg (va, char*));
                    break;

                case '%' :
                    backend->putc (backend, ch);

                default:
                    break;
            }
        }
    }
abort:;
}

void tfp_printf (const char *fmt, ...)
{
    va_list va;
    va_start (va,fmt);
    tfp_format (stdout_backend, fmt, va);
    va_end (va);
}

static void sprintf_putc (printf_backend_t *backend, char c)
{
    register char *out = (char *)backend->data;
    *out++ = c;
    backend->data = out;
}

void tfp_sprintf (char* s, const char *fmt, ...)
{
    static printf_backend_t sprintf_backend;
    sprintf_backend.putc = sprintf_putc;
    sprintf_backend.data = s;

    va_list va;
    va_start (va, fmt);
    tfp_format (&sprintf_backend, fmt, va);
    va_end (va);

    sprintf_putc (&sprintf_backend, 0);
}

void tfp_putc (char c)
{
    stdout_backend->putc (stdout_backend, c);
}

void tfp_puts (const char *s)
{
    while (*s)
        tfp_putc (*s++);
    tfp_putc ('\r');
    tfp_putc ('\n');
}

void tfp_fflush ()
{
    if (stdout_backend->flush)
        stdout_backend->flush (stdout_backend);
}
