/*
    uLZ compression library
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/ulz.h"
#include "useful/clike.h"
#include "useful/bitstream.h"

/* uLZ is a variant of Lempel-Ziv packer, optimized for low memory footprint,
 * low unpacker size and lack of computing power for decoding.
 *
 * The stream consists of literals (pieces of unmodified original data)
 * followed by backward references (an 'offset, length' pair which is a
 * reference back into the already unpacked data). Even if there are several
 * consecutive backward references, they are interleaved with a zero-length
 * literal, e.g. literals and references always toggle.
 *
 * Literals are encoded as (length, data...). Every quantity is encoded
 * using the ulz16u encoding (see below).
 *
 * References are encoded as (length-2, offset-1) pairs.
 *
 * Offsets and lengths are encoded using the following encoding, which
 * we'll call ulz16u (uLZ 16-bit unsigned).
 *
 * Code                 Bits    Low     High
 * 0XX                  3       0       3
 * 10XXXX               6       4       19
 * 110XXXXXXXX          11      20      275
 * 111XXXXXXXXXXXXXXXX  19      276     65811
 */

#define ULZ16U_0_LOW        0
#define ULZ16U_0_BITS       3
#define ULZ16U_0_PREFIX     0b000
#define ULZ16U_10_LOW       4
#define ULZ16U_10_BITS      6
#define ULZ16U_10_PREFIX    0b100000
#define ULZ16U_110_LOW      20
#define ULZ16U_110_BITS     11
#define ULZ16U_110_PREFIX   0b11000000000
#define ULZ16U_111_LOW      276
#define ULZ16U_111_BITS     19
#define ULZ16U_111_PREFIX   0b1110000000000000000
#define ULZ16U_MAX          65811

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
    return 0;
}

static bool ulz16u_write (bitstream_t *bs, unsigned value)
{
    unsigned bits;
    if (value < ULZ16U_10_LOW)
    {
        value = (value - ULZ16U_0_LOW) | ULZ16U_0_PREFIX;
        bits = ULZ16U_0_BITS;
    }
    else if (value < ULZ16U_110_LOW)
    {
        value = (value - ULZ16U_10_LOW) | ULZ16U_10_PREFIX;
        bits = ULZ16U_10_BITS;
    }
    else if (value < ULZ16U_111_LOW)
    {
        value = (value - ULZ16U_110_LOW) | ULZ16U_110_PREFIX;
        bits = ULZ16U_110_BITS;
    }
    else if (value <= ULZ16U_MAX)
    {
        value = (value - ULZ16U_111_LOW) | ULZ16U_111_PREFIX;
        bits = ULZ16U_111_BITS;
    }
    else
        return false;

    return bs_write_bits (bs, bits, value);
}

static void ulz_write_uleb128 (bitstream_t *bs, unsigned value)
{
    for (;;)
    {
        uint8_t chip = value & 0x7F;
        value >>= 7;
        if (value)
            chip |= 0x80;
        bs_write_bytes (bs, &chip, 1);
        if (!(chip & 0x80))
            return;
    }
}

bool ulz_compress (const void *idata, unsigned isize,
                   void *odata, unsigned *osize)
{
    bitstream_t obs;
    bs_init (&obs, odata, *osize);

    // write uncompressed data size to output stream first
    ulz_write_uleb128 (&obs, isize);

    uint8_t *start = (uint8_t *)idata;
    uint8_t *end = start + isize;

    // Current data pointer
    uint8_t *cur = start;
    // Start of literal
    uint8_t *lit_start = cur;

    while (cur < end)
    {
        unsigned ref_ofs;
        unsigned ref_len = 0;
        int ref_rating = 0;

        // Look back for data at 'cur'
        uint8_t *low = cur - (ULZ16U_MAX + 1);
        if (low < start)
            low = start;
        uint8_t *high = cur;
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

                assert (ofs_bits != 0);
                assert (len_bits != 0);

                int gain = len * 8 - len_bits - ofs_bits;
                // Take into account if we spend bits to encode zero-length literal
                if (cur == lit_start)
                    gain -= ULZ16U_0_BITS;
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
            // Put the literal into output bitstream
            unsigned lit_len = cur - lit_start;
            if (!ulz16u_write (&obs, lit_len) ||
                !bs_write_bytes (&obs, lit_start, lit_len))
                // BANG! no space for compressed data
                return false;

            //printf ("LIT: [%.*s]\n", lit_len, lit_start);

            // Put the reference into output stream
            if (!ulz16u_write (&obs, ref_len) ||
                !ulz16u_write (&obs, ref_ofs))
                return false;

            //printf ("REF: [%.*s] <- %u\n", ref_len, cur - ref_ofs, ref_ofs);

            cur += ref_len;
            lit_start = cur;
        }
    }

    *osize = bs_write_finish (&obs, odata, *osize);
    return *osize != 0;
}
