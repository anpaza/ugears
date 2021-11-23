/*
    Optimized C implementation for memcmp()
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"

#define LONG_ALIGN_MASK (__SIZEOF_LONG__ - 1)

int _memcmp (const void *s1, const void *s2, size_t n)
{
    register const uint8_t *a = (const uint8_t *)s1;
    register const uint8_t *b = (const uint8_t *)s2;
    register int res;

#if USEFUL_OPTIMIZE == 1

    for (;;)
    {
        if (n == 0)
            return 0;

        if ((((uintptr_t)a) & LONG_ALIGN_MASK) == 0)
            break;

        res = (int)*a - (int)*b;
        if (res != 0)
            return res;

        a++;
        b++;
        n--;
    }
    // now a is guaranteed to be aligned, n is guaranteed to be >0

    // if b is aligned too, compare by words
    if ((((uintptr_t)b) & LONG_ALIGN_MASK) == 0)
        while (n >= __SIZEOF_LONG__)
        {
            if (*(const long *)a != *(const long *)b)
                break;

            a += __SIZEOF_LONG__;
            b += __SIZEOF_LONG__;
            n -= __SIZEOF_LONG__;
        }

#endif

    // Compare the remaining bytes by one
    for (;;)
    {
        if (n == 0)
            return 0;

        res = (int)*a - (int)*b;
        if (res != 0)
            return res;

        a++;
        b++;
        n--;
    }
}
