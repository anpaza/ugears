/*
    Optimized C implementation for memchr()
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"

#define LONG_ALIGN_MASK (__SIZEOF_LONG__ - 1)

void *CLIKE_P (memchr) (const void *mem, int c, size_t size)
{
    register const uint8_t *src = (const uint8_t *)mem;
    register const uint8_t *end;
    register uint8_t val = c;

#if USEFUL_OPTIMIZE == 1

    end = (const uint8_t *)(((uintptr_t)mem + LONG_ALIGN_MASK) & ~LONG_ALIGN_MASK);
    if (end <= src + size)
    {
        while (src < end)
        {
            if (*src == val)
                return (void *)src;

            src++;
        }
        // now src is guaranteed to be aligned

        unsigned long cccc = (unsigned long)val;
        cccc |= cccc << 8;
#if __SIZEOF_LONG__ > 2
        cccc |= cccc << 16;
#endif
#if __SIZEOF_LONG__ > 4
        cccc |= cccc << 32;
#endif

        end = (const uint8_t *)(((uintptr_t)mem + size) & ~LONG_ALIGN_MASK);
        while (src < end)
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
            // This is not quite SMD since we have carry between components,
            // but this will influence only higher bytes, e.g.:
            // ~0x0100 & (0x0100 - 0x0101) = 0xfeff due to carry flag
            // but we don't care as we'll detect the lower ff first.
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

                return (void *)src;
            }

            src += __SIZEOF_LONG__;
        }
    }

#endif

    /* Check byte by byte */
    end = (const uint8_t *)mem + size;
    while (src < end)
    {
        if (*src == val)
            return (void *)src;

        src++;
    }

    return NULL;
}
