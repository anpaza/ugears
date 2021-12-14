/*
    Useful function for embedded systems
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _USEFUN_H
#define _USEFUN_H

#include <useful/useful.h>

/**
 * @file usefun.h
 *      A number of useful functions, balancedly optimized for speed/size.
 */

/**
 * Xorshift Random Number Generator by George Marsaglia:
 * https://en.wikipedia.org/wiki/Xorshift
 *
 * Current RNG state.
 */
typedef uint32_t xs_rng_t [5];

/**
 * Initialize RNG with a seed value.
 * RNG state is undefined until you call this function.
 *
 * @arg xsr The Random Number Generator state
 * @arg seed Initial seed
 */
EXTERN_C void xs_init (xs_rng_t xsr, uint32_t seed);

/**
 * Feed additional randomness into the random pool.
 * You may call this as many times as you wish,
 * but it doesn't make sense to call it more than 4 times
 * (after you call xs_init()).
 *
 * @arg xsr An initialized Random Number Generator state
 * @arg seed Additional seed to mix into the random pool
 */
EXTERN_C void xs_seed (xs_rng_t xsr, uint32_t seed);

/**
 * Get next random number in series
 */
EXTERN_C uint32_t xs_rand (xs_rng_t xsr);

/**
 * Decode a number in the unsigned LEB128 format
 * @arg data A pointer to encoded data. On return this pointer
 *      is updated to point past the encoded data.
 * @return The decoded number
 */
EXTERN_C uint32_t uleb128 (const uint8_t **data);

/**
 * Decode a number in the signed LEB128 format
 * @arg data A pointer to encoded data. On return this pointer
 *      is updated to point past the encoded data.
 * @return The decoded number
 */
EXTERN_C int32_t sleb128 (const uint8_t **data);

/**
 * Skip an (unused) LEB128 value
 * @arg data A pointer to encoded data
 * @return A pointer past the encoded LEB128 value
 */
INLINE_ALWAYS const uint8_t *skip_leb128 (const uint8_t *data)
{ while (*data & 0x80) data++; return data + 1; }

/**
 * Return the sign of the argument
 * @arg x The number to extract the sign from
 * @return -1 if @a x is negative, +1 if @a x is positive and 0 if @a x is 0.
 */
INLINE_ALWAYS int32_t sign (int32_t x)
{ return (x >> 31) + (x > 0); }

/**
 * Update checksum with the next block of data.
 * @arg sum The starting value of checksum. For first block this should be 0,
 *      for next blocks use the return value of previous ip_crc_block invocation.
 * @arg data A pointer to data
 * @arg len Data length in bytes
 * @return @a sum updated according to the contents of data block
 */
EXTERN_C uint32_t ip_crc_block (uint32_t sum, const void *data, unsigned len);

/**
 * Finalize checksum computations.
 * @arg sum The value returned by ip_crc_block().
 * @return The 16-bit checksum in network endian format
 */
EXTERN_C uint16_t ip_crc_fin (uint32_t sum);

/**
 * Compute and finalize the checksum of a data block.
 * @arg data A pointer to data
 * @arg len Data length in bytes
 * @return The 16-bit checksum in network endian format
 */
INLINE_ALWAYS uint16_t ip_crc (void *data, unsigned len)
{ return ip_crc_fin (ip_crc_block (0, data, len)); }

/**
 * Load a 16-bit little-endian value from an unaligned address
 * @arg data The unaligned address
 * @return The memory value at given address
 */
EXTERN_C uint16_t uget16le (const void *data);

/**
 * Load a 16-bit big-endian value from an unaligned address
 * @arg data The unaligned address
 * @return The memory value at given address
 */
EXTERN_C uint16_t uget16be (const void *data);

/**
 * Load a 32-bit little-endian value from an unaligned address
 * @arg data The unaligned address
 * @return The memory value at given address
 */
EXTERN_C uint32_t uget32le (const void *data);

/**
 * Load a 32-bit big-endian value from an unaligned address
 * @arg data The unaligned address
 * @return The memory value at given address
 */
EXTERN_C uint32_t uget32be (const void *data);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define uget16		uget16le
#define uget32		uget32le
#else
#define uget16		uget16be
#define uget32		uget32be
#endif

#endif // _USEFUN_H
