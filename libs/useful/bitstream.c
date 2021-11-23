/*
    Bitstream functions
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/bitstream.h"
#include "useful/clike.h"

#define MASK(bits)   ((1 << bits) - 1)

void bs_init (bitstream_t *bs, void *buff, unsigned size)
{
    bs->ptr = buff;
    bs->end = bs->ptr + size;
    bs->acc = 0;
    bs->acc_bits = 0;
    bs->exhausted = false;
}

bool bs_read_bytes (bitstream_t *bs, void *dst, unsigned size)
{
    uint8_t *ptr = bs->ptr + size;
    if (ptr > bs->end)
    {
        bs->exhausted = true;
        return false;
    }

    memcpy (dst, bs->ptr, size);
    bs->ptr = ptr;
    return true;
}

uint32_t bs_read_bits (bitstream_t *bs, unsigned bits)
{
    uint32_t val = 0;
    unsigned val_bits = 0;
    uint32_t acc = bs->acc;
    unsigned acc_bits = bs->acc_bits;
    uint8_t *end = bs->end;

    while (bits != 0)
    {
        if (acc_bits == 0)
        {
            if (end <= bs->ptr)
            {
                bs->exhausted = true;
                break;
            }
            acc = *(--end);
            acc_bits = 8;
        }

        unsigned copy_bits = MIN (bits, acc_bits);
        val |= (uint32_t)(acc & MASK (copy_bits)) << val_bits;
        val_bits += copy_bits;
        bits -= copy_bits;
        acc_bits -= copy_bits;
        acc >>= copy_bits;
    }

    bs->end = end;
    bs->acc = acc;
    bs->acc_bits = acc_bits;

    return val;
}

// -------------------------------------------------------------------------- //

bool bs_write_bytes (bitstream_t *bs, const void *src, unsigned size)
{
    uint8_t *ptr = bs->ptr + size;
    if (ptr > bs->end)
        return false;

    memcpy (bs->ptr, src, size);
    bs->ptr = ptr;
    return true;
}

bool bs_write_bits (bitstream_t *bs, unsigned bits, uint32_t val)
{
    unsigned acc_bits = bs->acc_bits;
    uint32_t acc = bs->acc & MASK (acc_bits);
    uint8_t *end = bs->end;

    while (bits != 0)
    {
        unsigned copy_bits = MIN (32 - acc_bits, bits);
        acc |= (uint32_t)(val & MASK (copy_bits)) << acc_bits;
        acc_bits += copy_bits;
        val >>= copy_bits;
        bits -= copy_bits;

        while (acc_bits >= 8)
        {
            if (end <= bs->ptr)
            {
                bs->exhausted = true;
                return false;
            }

            *(--end) = acc;
            acc >>= 8;
            acc_bits -= 8;
        }
    }

    bs->end = end;
    bs->acc = acc;
    bs->acc_bits = acc_bits;

    return true;
}

unsigned bs_write_finish (bitstream_t *bs, void *buff, unsigned size)
{
    uint8_t *end = bs->end;
    if (bs->acc_bits)
    {
        if (end <= bs->ptr)
        {
            bs->exhausted = true;
            return 0;
        }
        *(--end) = bs->acc;
        bs->acc_bits = 0;
    }

    uint8_t *start = (uint8_t *)buff;
    unsigned bits_size = start + size - end;
    uint8_t *ptr = bs->ptr;
    while (bits_size--)
        *ptr++ = *end++;

    // make bitstream ready for reading, just in case
    bs->ptr = start;
    bs->end = ptr;

    return ptr - start;
}
