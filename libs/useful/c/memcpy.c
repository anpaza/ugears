/*
    Optimized C implementation for memcpy()
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <useful/usefun.h>

#define LONG_MASK (__SIZEOF_LONG__ - 1)

void _memcpy (void *dest, const void *src, unsigned len)
{
    register uint8_t *d = (uint8_t *)dest;
    register const uint8_t *s = (const uint8_t *)src;

#if USEFUL_OPTIMIZE == 1

    // Proceed byte by byte up to nearest long boundary
    for (;;)
    {
        if (len == 0)
            return;

        if ((((uintptr_t)d) & LONG_MASK) == 0)
            break;

        *d++ = *s++;
        len--;
    }
    // now d is guaranteed to be aligned, len is guaranteed to be >0

    // if s is aligned too, compare by words
    if ((((uintptr_t)s) & LONG_MASK) == 0)
        while (len >= __SIZEOF_LONG__)
        {
            *(long *)d = *(const long *)s;

            d += __SIZEOF_LONG__;
            s += __SIZEOF_LONG__;
            len -= __SIZEOF_LONG__;
        }
#endif

    // Copy the remaining bytes by one
    while (len != 0)
    {
        *d++ = *s++;
        len--;
    }
}
