/*
    uLZ compression library
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#define _GNU_SOURCE
#include "useful/clike.h"
#include "useful/ulz.h"
#include "useful/bitstream.h"
#include "ulz_priv.h"

// Uncomment for noisy compressor debugging
//#define NOISY

/// Return number of bits used to encode specified value
INLINE_ALWAYS unsigned ulz16u_bits (unsigned value)
{
    if (value < ULZ16U_10_LOW)
        return ULZ16U_0_BITS;
    if (value < ULZ16U_110_LOW)
        return ULZ16U_10_BITS;
    if (value < ULZ16U_111_LOW)
        return ULZ16U_110_BITS;
    if (value <= ULZ16U_MAX)
        return ULZ16U_111_BITS;
    // full 32-bit length
    return ULZ16U_111_BITS + 32;
}

static bool ulz16u_write (bitstream_t *bs, unsigned value)
{
    unsigned bits;
    if (value < ULZ16U_10_LOW)
    {
        value = ((value - ULZ16U_0_LOW) << 1) | ULZ16U_0_PREFIX;
        bits = ULZ16U_0_BITS;
    }
    else if (value < ULZ16U_110_LOW)
    {
        value = ((value - ULZ16U_10_LOW) << 2) | ULZ16U_10_PREFIX;
        bits = ULZ16U_10_BITS;
    }
    else if (value < ULZ16U_111_LOW)
    {
        value = ((value - ULZ16U_110_LOW) << 3) | ULZ16U_110_PREFIX;
        bits = ULZ16U_110_BITS;
    }
    else if (value <= ULZ16U_MAX)
    {
        value = ((value - ULZ16U_111_LOW) << 3) | ULZ16U_111_PREFIX;
        bits = ULZ16U_111_BITS;
    }
    else
    {
        uint32_t val32 = UINT32_LE (value);
        value = ((ULZ16U_RAW32 - ULZ16U_111_LOW) << 3) | ULZ16U_111_PREFIX;
        bits = ULZ16U_111_BITS;
        return bs_write_bits (bs, bits, value) &&
                bs_write_bytes (bs, &val32, sizeof (val32));
    }

    return bs_write_bits (bs, bits, value);
}

static bool ulz_write_uleb128 (bitstream_t *bs, unsigned value)
{
    uint8_t chips [5];
    uint8_t *cur = chips;
    for (;;)
    {
        uint8_t chip = value & 0x7F;
        value >>= 7;
        if (value)
            chip |= 0x80;
        *cur++ = chip;
        if (value == 0)
            break;
    }

    return bs_write_bytes (bs, chips, cur - chips);
}

static bool ulz_write_literal (bitstream_t *bs, uint8_t *lit, unsigned len)
{
    if (!ulz16u_write (bs, len) ||
        !bs_write_bytes (bs, lit, len))
        return false;

    return true;
}

bool ulz_compress (const void *idata, unsigned isize,
                   void *odata, unsigned *osize)
{

    bitstream_t obs;
    bs_init (&obs, odata, *osize);

    // write uncompressed data size to output stream first
    if (!ulz_write_uleb128 (&obs, isize))
        return false;

    uint8_t *start = (uint8_t *)idata;
    uint8_t *end = start + isize;

    // Current data pointer
    uint8_t *cur = start;
    // Start of literal
    uint8_t *lit_start = cur;

#ifdef NOISY
    unsigned count_dec = 0;
    unsigned count_enc = (obs.ptr - (uint8_t *)odata) * 8;
#endif

    while (cur < end)
    {
        unsigned ref_ofs = 0;
        unsigned ref_len = 0;

        // The following code is a pretty dumb attempt to find references
        // to preceeding data. This certainly can be done much faster,
        // but this code is very simple and easy to understand.

        // Look back for data at 'cur'
        uint8_t *low = cur - (ULZ16U_MAX + 1);
        if (low < start)
            low = start;
        uint8_t *high = cur;
        int ref_rating = 0;
        while (low < high)
        {
            uint8_t *ptr = (uint8_t *)memrchr (low, *cur, high - low);
            if (!ptr)
                break;

            unsigned len;
            for (len = 1; (len < (ULZ16U_MAX + 2)) && (ptr [len] == cur [len]); len++)
                ;

            if (len >= 2)
            {
                // Find out how many bits will take the (length-2, offset-1) pair
                unsigned ofs = cur - ptr;
                unsigned ofs_bits = ulz16u_bits (ofs - 1);
                unsigned len_bits = ulz16u_bits (len - 2);

                int gain = len * 8 - len_bits - ofs_bits;
                if (gain > ref_rating)
                {
                    ref_rating = gain;
                    ref_ofs = cur - ptr;
                    ref_len = len;
                }
            }

            high = ptr;
        }

        if (ref_len == 0)
            cur++;
        else
        {
            unsigned lit_len = cur - lit_start;

            // Cut our loses if literal gets way too long.
            // If we don't, a long literal trail may stop us from
            // seeing small gains, like text insertions in a big
            // uncompressible blob.
            if (lit_len < ULZ16U_MAX / 2)
            {
                // Check the overall compression rate
                unsigned enc_len = ulz16u_bits (lit_len) + lit_len * 8 +
                        ulz16u_bits (ref_len - 2) + ulz16u_bits (ref_ofs - 1);
                unsigned dec_len = ulz16u_bits (lit_len + ref_len) -
                        (lit_len ? ulz16u_bits (lit_len) : 0) +
                        (lit_len + ref_len) * 8;

                // Sometimes it's better to just put everything as literal
                if (enc_len >= dec_len)
                {
                    cur += 1;
                    continue;
                }
            }

            // Put the literal into output bitstream
            if (!ulz_write_literal (&obs, lit_start, lit_len))
                // BANG! no space for compressed data
                return false;

#ifdef NOISY
            printf ("LIT: [%.*s]\n", lit_len, lit_start);
            count_enc += ulz16u_bits (lit_len) + lit_len * 8;
#endif

            // Put the reference into output stream
            if (!ulz16u_write (&obs, ref_len - 2) ||
                !ulz16u_write (&obs, ref_ofs - 1))
                return false;

#ifdef NOISY
            printf ("REF: [%.*s] <- %u\n", ref_len, cur - ref_ofs, ref_ofs);
            count_enc += ulz16u_bits (ref_len - 2) + ulz16u_bits (ref_ofs - 1);
            count_dec += (cur + ref_len - lit_start) * 8;
            printf ("GAIN: %d bits of total %u\n", count_dec - count_enc, count_dec);
#endif

            cur += ref_len;
            lit_start = cur;
        }
    }

    // Put the last literal into the output stream
    if (!ulz_write_literal (&obs, lit_start, cur - lit_start))
        return false;

    *osize = bs_write_finish (&obs, odata, *osize);
    return *osize != 0;
}
