/*
    Optimized C implementation for memrchr()
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"

#define LONG_ALIGN_MASK (__SIZEOF_LONG__ - 1)

void *CLIKE_P (memrchr) (const void *mem, int c, size_t size)
{
    register const uint8_t *src = (const uint8_t *)mem + size;
    register const uint8_t *end;
    register uint8_t val = c;

#if USEFUL_OPTIMIZE == 1

    end = (const uint8_t *)((uintptr_t)src & ~LONG_ALIGN_MASK);
    if (end >= (const uint8_t *)mem)
    {
        while (src > end)
        {
            src--;

            if (*src == val)
                return (void *)src;
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

        end = (const uint8_t *)(((uintptr_t)mem + LONG_ALIGN_MASK) & ~LONG_ALIGN_MASK);
        while (src > end)
        {
            src -= __SIZEOF_LONG__;
            unsigned long test = *(unsigned long *)src ^ cccc;

#if __SIZEOF_LONG__ == 8
#  define ONES      0x0101010101010101UL
#  define TOPS      0x8080808080808080UL
#  define TEST_16   0x8080000000000000UL
#  define TEST_8    0x8000000000000000UL
#  define MASK_ODD  0x00ff00ff00ff00ffUL
#  define MASK_EVEN 0xff00ff00ff00ff00UL
#elif __SIZEOF_LONG__ == 4
#  define ONES      0x01010101UL
#  define TOPS      0x80808080UL
#  define TEST_16   0x80800000UL
#  define TEST_8    0x80000000UL
#  define MASK_ODD  0x00ff00ffUL
#  define MASK_EVEN 0xff00ff00UL
#else
#  error "WTF?!"
#endif

            // ~x & (x - 1) will have 7th bit set only for x == 0
            // but since we don't have true SMD, the carry bit from a byte
            // may influence the next byte up, and give false zero detection.
            // Because of this, we do detection in two phases, first
            // odd bytes, then even bytes, neutralizing any carry between
            // odd and even bytes.
            unsigned long test_odd = test | MASK_EVEN;
            unsigned long test_even = test | MASK_ODD;
            test = (~test_odd & (test_odd - ONES)) |
                    (~test_even & (test_even - ONES));
            // now test contains 1 in 7th bit of every byte that is zero
            if (test & TOPS)
            {
                // Point src to last byte in block
                src += __SIZEOF_LONG__ - 1;

#if __SIZEOF_LONG__ == 8
                if (!(test & 0x8080808000000000UL))
                    src -= 4, test <<= 32;
#endif
                if (!(test & TEST_16))
                    src -= 2, test <<= 16;
                if (!(test & TEST_8))
                    src -= 1/*, test <<= 8*/;

                return (void *)src;
            }
        }
    }

#endif

    /* Check byte by byte */
    end = (const uint8_t *)mem;
    while (src > end)
    {
        src--;

        if (*src == val)
            return (void *)src;
    }

    return NULL;
}
