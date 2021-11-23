/*
    uLZ compression library
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _ULZ_PRIV_H
#define _ULZ_PRIV_H

/* uLZ is a variant of Lempel-Ziv packer, optimized for low memory footprint,
 * low unpacker size and lack of computing power for decoding.
 *
 * The compression results compares well with the well-known LZ4 algorithm.
 * Sometimes uLZ wins, sometimes loses, all in all on a par with LZ4.
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
 * References are encoded as (length-2, offset-1) pairs, because length < 2
 * and offset < 1 doesn't make sense.
 *
 * Offsets and lengths are encoded using the following encoding, which
 * we'll call ulz16u (uLZ 16-bit unsigned). Lower bits are to the right.
 *
 * Code                 Bits    Low     High
 * XX0                  3       0       3
 * XXXX01               6       4       19
 * XXXXXXXX011          11      20      275
 * XXXXXXXXXXXXXXXX111  19      276     65811
 *
 * Decoder first reads the lowest three bits, and depending on them decides
 * how many more bits it has to read.
 *
 * To allow encoding very large numbers the last rule is: if value equals 65811
 * (19 bits of '1'), it is followed by a 32-bit full-size quantity.
 */

#define ULZ16U_0_LOW        0
#define ULZ16U_0_BITS       3
#define ULZ16U_0_PREFIX     0b0
#define ULZ16U_10_LOW       4
#define ULZ16U_10_BITS      6
#define ULZ16U_10_PREFIX    0b01
#define ULZ16U_110_LOW      20
#define ULZ16U_110_BITS     11
#define ULZ16U_110_PREFIX   0b011
#define ULZ16U_111_LOW      276
#define ULZ16U_111_BITS     19
#define ULZ16U_111_PREFIX   0b111
#define ULZ16U_MAX          65810
#define ULZ16U_RAW32        65811

#endif // _ULZ_PRIV_H
