/*
    Bitstream functions
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include "useful.h"

/**
 * A bitstream used for reading.
 *
 * Despite there's a difference between bistreams used for reading and
 * bitstreams used for writing, we'll use one type for both reading and
 * writing.
 *
 * Whole bytes (the 'byte substream') are stored at the beginning of data
 * buffer (bottom-up), and the 'bits substream' is stored in reverse direction
 * at the end of the buffer (top-down).
 */
typedef struct
{
    /// Current data pointer
    uint8_t *ptr;
    /// Pointer to end of data
    uint8_t *end;
    /// Bit accumulator
    uint8_t acc;
    /// Number of bits in accumulator (0 to 8)
    uint8_t acc_bits;
    /// Set to true if a read past end of data is attempted
    bool exhausted;
} bitstream_t;

/**
 * Initialize a bitsteam for reading or writing.
 *
 * This implementation of bitstreams organizes data in two independent
 * interleaving sub-streams: the bit substream and the byte substream.
 * The bit substream is slow to read but can be used to store sub-byte
 * quantities. The byte substream is very fast to read/write.
 *
 * @param bs Bitstream
 * @param buff Source bitstream data or pointer to destination buffer
 * @param size Source data or destination buffer size
 */
extern void bs_init (bitstream_t *bs, void *buff, unsigned size);

/**
 * Read whole bytes of data from bitstream (from the byte substream).
 * This is fast as it does not involve any bitshifts, just plain data copy.
 *
 * @param bs Bitstream
 * @param dst A pointer to destination buffer
 * @param size Number of bytes to read
 * @return false if bitstream is exhausted
 */
extern bool bs_read_bytes (bitstream_t *bs, void *dst, unsigned size);

/**
 * Read up to 32 bits from bitstream (from the bit substream).
 * This is slower and is usually acceptable for small amounts of data
 * (e.g. control data).
 *
 * If a read past end of buffer is attempted, bs->exhausted is set to true.
 *
 * @param bs Bitstream
 * @param bits Number of bits to read (must be <= 32!)
 * @return the bits read
 */
extern uint32_t bs_read_bits (bitstream_t *bs, unsigned bits);

// -------------------------------------------------------------------------- //

/**
 * Write whole bytes to bitstream (to the byte substream).
 *
 * @param bs Bitstream
 * @param src Pointer to data to write into the bitstream
 * @param size Data size
 * @return false if data does not fit into the output buffer
 */
extern bool bs_write_bytes (bitstream_t *bs, const void *src, unsigned size);

/**
 * Write bits into the bitstream (to the bits substream).
 *
 * @param bs Bitstream
 * @param bits Number of bits to write
 * @param val The bits to be written (starting from lower bits)
 * @return false if data does not fit into the output buffer
 */
extern bool bs_write_bits (bitstream_t *bs, unsigned bits, uint32_t val);

/**
 * Finish writing to a bytestream. This will remove the internal gap
 * between the bit and byte substreams. The resulting data will be compacted
 * to occupy only as much space as needed.
 *
 * Writing to this bitstream is not allowed after this function is called
 * as it may introduce unexpected zero bits into the bit stream.
 *
 * The bistream is left in a state ready for reading e.g. as if
 * bs_init (bs, buff, return value) would be called.
 *
 * @param bs The bitstream
 * @param buff A pointer to output buffer, initially passed to bs_init.
 * @param size The size of the buffer, initially passed to bs_init.
 * @return The size of compacted bitstream data. On error this will return 0.
 */
extern unsigned bs_write_finish (bitstream_t *bs, void *buff, unsigned size);

#endif // _BITSTREAM_H
