/*
    Optimized C implementation for memset()
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <useful/usefun.h>

#define LONG_MASK (__SIZEOF_LONG__ - 1)

void _memset (void *dest, char c, unsigned len)
{
    register uint8_t *d = (uint8_t *)dest;

#if USEFUL_OPTIMIZE == 1

    for (;;)
    {
        if (len == 0)
            return;

        if ((((uintptr_t)d) & LONG_MASK) == 0)
            break;

        *d = c;

        d++;
        len--;
    }
    // now d is guaranteed to be aligned, len is guaranteed to be >0

    unsigned long cccc = (unsigned long)(unsigned char)c;
    cccc |= cccc << 8;
#if __SIZEOF_LONG__ > 2
    cccc |= cccc << 16;
#endif
#if __SIZEOF_LONG__ > 4
    cccc |= cccc << 32;
#endif

    while (len >= __SIZEOF_LONG__)
    {
        *(unsigned long *)d = cccc;

        d += __SIZEOF_LONG__;
        len -= __SIZEOF_LONG__;
    }

#endif

    /* Fill byte by byte */
    while (len != 0)
    {
        *d = c;

        d++;
        len--;
    }
}
