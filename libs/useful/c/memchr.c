/*
    Optimized C implementation for memchr()
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <useful/usefun.h>

#define LONG_MASK (__SIZEOF_LONG__ - 1)

const void *_memchr (const void *data, char c, unsigned len)
{
    register const char *src = (const char *)data;

#if USEFUL_OPTIMIZE == 1

    for (;;)
    {
        if (len == 0)
            return NULL;

        if ((((uintptr_t)src) & LONG_MASK) == 0)
            break;

        if (*src == c)
            return src;

        src++;
        len--;
    }
    // now a is guaranteed to be aligned, len is guaranteed to be >0

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
        unsigned long test = *(unsigned long *)src ^ cccc;

#if __SIZEOF_LONG__ == 8
#  define ONES  0x0101010101010101UL
#  define TOPS  0x8080808080808080UL
#elif __SIZEOF_LONG__ == 4
#  define ONES  0x01010101UL
#  define TOPS  0x80808080UL
#else
#  error "WTF?!"
#endif

        // ~x & (x - 1) will have 7th bit set only for x == 0
        test = ~test & (test - ONES);
        // now test contains 1 in 7th bit of every byte that is zero
        if (test & TOPS)
        {
#if __SIZEOF_LONG__ == 8
            if (!(test & 0x80808080))
                src += 4, test >>= 32;
#endif
            if (!(test & 0x8080))
                src += 2, test >>= 16;
            if (!(test & 0x80))
                src += 1/*, test >>= 8*/;

            return src;
        }

        src += __SIZEOF_LONG__;
        len -= __SIZEOF_LONG__;
    }

#endif

    /* Check byte by byte */
    while (len != 0)
    {
        if (*src == c)
            return src;

        src++;
        len--;
    }

    return NULL;
}
