/*
    uLZ compression library
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"
#include "useful/ulz.h"
#include "useful/bitstream.h"
#include "ulz_priv.h"

// Uncomment for noisy compressor debugging
//#define NOISY

static unsigned ulz_read_uleb128 (const uint8_t **idata, unsigned isize)
{
    // this is like uleb128() but with a few additional sanity checks
    unsigned d, r = 0;
    unsigned shift = 0;
    const uint8_t *src = *idata;
    const uint8_t *end = src + isize;

    do
    {
        if (src >= end)
            return 0;

        d = *src++;
        r |= (d & 0x7F) << shift;
        shift += 7;

        if (shift > 7 * 5)
            return 0;
    } while (d & 0x80);

    *idata = src;

    return r;
}

unsigned ulz_decompress_size (const void *idata, unsigned isize)
{
    const uint8_t *src = (const uint8_t *)idata;
    return ulz_read_uleb128 (&src, isize);
}

static bool ulz16u_read (bitstream_t *bs, uint32_t *value)
{
    unsigned v = bs_read_bits (bs, ULZ16U_0_BITS);
    if (bs->exhausted)
        return false;

    if (!(v & 1))
        *value = (v >> 1) + ULZ16U_0_LOW;
    else if (!(v & 2))
    {
        v |= bs_read_bits (bs, ULZ16U_10_BITS - ULZ16U_0_BITS) << ULZ16U_0_BITS;
        *value = (v >> 2) + ULZ16U_10_LOW;
    }
    else if (!(v & 4))
    {
        v |= bs_read_bits (bs, ULZ16U_110_BITS - ULZ16U_0_BITS) << ULZ16U_0_BITS;
        *value = (v >> 3) + ULZ16U_110_LOW;
    }
    else
    {
        v |= bs_read_bits (bs, ULZ16U_111_BITS - ULZ16U_0_BITS) << ULZ16U_0_BITS;
        *value = (v >> 3) + ULZ16U_111_LOW;

        // if value is larger than ULZ16U_MAX, it is encoded in 32 raw bits
        if (*value == ULZ16U_RAW32)
            return bs_read_bytes (bs, value, sizeof (uint32_t));
    }

    return !bs->exhausted;
}

bool ulz_decompress (const void *idata, unsigned isize,
                     void *odata, unsigned *osize)
{
    bitstream_t ibs;
    bs_init (&ibs, (void *)idata, isize);

    uint8_t *cur = (uint8_t *)odata;

    unsigned dec_size = ulz_read_uleb128 ((const uint8_t **)&ibs.ptr, isize);
    if (dec_size > *osize)
        // either broken compressed stream, or not enough big buffer
        return false;

    uint8_t *end = cur + dec_size;
    *osize = dec_size;

    while (cur < end)
    {
        uint32_t lit_len;
        if (!ulz16u_read (&ibs, &lit_len) ||
            (cur + lit_len > end) ||
            !bs_read_bytes (&ibs, cur, lit_len))
            return false;

#ifdef NOISY
        printf ("LIT: [%.*s]\n", lit_len, cur);
#endif

        cur += lit_len;
        if (cur >= end)
            break;

        uint32_t ref_len, ref_ofs;
        if (!ulz16u_read (&ibs, &ref_len) ||
            (cur + (ref_len += 2) > end) ||
            !ulz16u_read (&ibs, &ref_ofs))
            return false;
        ref_ofs += 1;

#ifdef NOISY
        printf ("REF: [%.*s] <- %u\n", ref_len, cur - ref_ofs, ref_ofs);
#endif

        // Copy byte by byte, don't use memcpy as it may optimize
        // and this won't work on overlapping memory areas
        uint8_t *ref = cur - ref_ofs;
        while (ref_len--)
            *cur++ = *ref++;
    }

    return true;
}
