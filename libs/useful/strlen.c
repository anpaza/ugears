/*
    A library of generally useful functions
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <useful/usefun.h>

size_t _strlen (const char *str)
{
    const char *orig = str;

#if USEFUL_OPTIMIZE == 0

    /* Optimize by size */
    while (*str)
        str++;
    return str - orig;

#else

    /* Optimize by speed */
    while ((((uintptr_t)str) & (sizeof (unsigned long) - 1)))
    {
        if (!*str)
            return str - orig;
        str++;
    }

    for (;;)
    {
        unsigned long test = *(unsigned long *)str;

#if __SIZEOF_LONG__ == 8
#  define ONES	0x0101010101010101UL
#  define TOPS	0x8080808080808080UL
#elif __SIZEOF_LONG__ == 4
#  define ONES	0x01010101UL
#  define TOPS	0x80808080UL
#else
#  error "WTF?!"
#endif

        // ~x & (x - 1) will have 7th bit set only for x == 0
        test = ~test & (test - ONES);
        // now test contains 1 in 7th bit of every byte that is zero
        if (test & TOPS)
        {
            size_t ret = str - orig;
#if __SIZEOF_LONG__ == 8
            if (!(test & 0x80808080))
                ret += 4, test >>= 32;
#endif
            if (!(test & 0x8080))
                ret += 2, test >>= 16;
            if (!(test & 0x80))
                ret += 1, test >>= 8;

            return ret;
        }

        str += sizeof (unsigned long);
    }

#endif
}
