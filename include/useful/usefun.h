/*
    Useful function for embedded systems
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __USEFUN_H__
#define __USEFUN_H__

#include <stdint.h>

/**
 * @file usefun.h
 *      A number of useful functions, balancedly optimized for speed/size.
 *      Some functions are like their ANSI C counterparts, but they aren't
 *      meant to be 100% compliant with the standard.
 */

/**
 * Fill the first @a len bytes of the memory area pointed to by
 * @a dest with the constant byte @a c.
 * @arg dest
 *      A pointer of memory to fill
 * @arg c
 *      The value to fill with
 * @arg len
 *      Number of bytes to fill
 */
extern void memset (void *dest, char c, unsigned len);

/**
 * Fill the first @a len bytes of the memory area pointed to by
 * @a dest with the zero constant.
 * @arg dest
 *      A pointer of memory to fill
 * @arg len
 *      Number of bytes to fill
 */
extern void memclr (void *dest, unsigned len);

/**
 * Optimized traditional memcpy().
 * @arg dest
 *      The destination pointer
 * @arg src
 *      Source pointer
 * @arg len
 *      Number of bytes to copy
 */
extern void memcpy (void *dest, const void *src, unsigned len);

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
extern void xs_init (xs_rng_t xsr, uint32_t seed);

/**
 * Feed additional randomness into the random pool.
 * You may call this as many times as you wish,
 * but it doesn't make sense to call it more than 4 times
 * (after you call xs_init()).
 *
 * @arg xsr An initialized Random Number Generator state
 * @arg seed Additional seed to mix into the random pool
 */
extern void xs_seed (xs_rng_t xsr, uint32_t seed);

/**
 * Get next random number in series
 */
extern uint32_t xs_rand (xs_rng_t xsr);

/**
 * Set the starting point for rand().
 * This uses a global xs_rng_t.
 *
 * @arg seed The number that determines the pseudo-random sequence.
 */
extern void srand (unsigned seed);

/**
 * Get a pseudo-random number.
 * This uses a global xs_rng_t.
 *
 * @return A new random number in the range 0..MAX_UNSIGNED_INT
 */
extern unsigned rand ();

/**
 * Return the sine of the angle
 * @arg angle
 *      Angle, 90° = 64, 180° = 128, 270° = 192 etc.
 * @return
 *      The sine value in signed 1.8 format
 */
extern int sin64 (uint8_t angle);

/**
 * Return the cosine of the angle
 * @arg angle
 *      Angle, 90° = 64, 180° = 128, 270° = 192 etc.
 * @return
 *      The cosine value in signed 1.8 format
 */
static inline int cos64 (uint8_t angle)
{ return sin64 (angle + 64); }

/**
 * Decode a number in the unsigned LEB128 format
 * @arg data
 *      A pointer to encoded data. On return this pointer
 *      is updated to point past the encoded data.
 * @return
 *      The decoded number
 */
extern uint32_t uleb128 (const uint8_t **data);

/**
 * Decode a number in the signed LEB128 format
 * @arg data
 *      A pointer to encoded data. On return this pointer
 *      is updated to point past the encoded data.
 * @return
 *      The decoded number
 */
extern int32_t sleb128 (const uint8_t **data);

/**
 * Skip an (unused) LEB128 value
 * @arg data
 *      A pointer to encoded data
 * @return
 *      A pointer past the encoded LEB128 value
 */
static inline const uint8_t *skip_leb128 (const uint8_t *data)
{ while (*data & 0x80) data++; return data + 1; }

/**
 * Return the sign of the argument
 * @arg x
 *      The number to extract the sign from
 * @return
 *      -1 if @a x is negative, +1 if @a x is positive and 0 if @a x is 0.
 */
static inline int32_t sign (int32_t x)
{ return (x >> 31) + (x > 0); }

/**
 * Update checksum with the next block of data.
 * @arg sum
 *      The starting value of checksum. For first block this should be 0,
 *      for next blocks use the return value of previous ip_crc_block invocation.
 * @arg data
 *      A pointer to data
 * @uint len
 *      Data length in bytes
 * @return
 *      @a sum updated according to the contents of data block
 */
extern uint32_t ip_crc_block (uint32_t sum, const void *data, unsigned len);

/**
 * Finalize checksum computations.
 * @arg sum
 *      The value returned by ip_crc_block().
 * @return
 *      The 16-bit checksum in network endian format
 */
extern uint16_t ip_crc_fin (uint32_t sum);

/**
 * Compute and finalize the checksum of a data block.
 * @arg data
 *      A pointer to data
 * @uint len
 *      Data length in bytes
 * @return
 *      The 16-bit checksum in network endian format
 */
static inline uint16_t ip_crc (void *data, unsigned len)
{ return ip_crc_fin (ip_crc_block (0, data, len)); }

/**
 * Load a 16-bit little-endian value from an unaligned address
 * @arg data
 *      The unaligned address
 * @return
 *      The memory value at given address
 */
extern uint16_t uget16le (const void *data);

/**
 * Load a 16-bit big-endian value from an unaligned address
 * @arg data
 *      The unaligned address
 * @return
 *      The memory value at given address
 */
extern uint16_t uget16be (const void *data);

/**
 * Load a 32-bit little-endian value from an unaligned address
 * @arg data
 *      The unaligned address
 * @return
 *      The memory value at given address
 */
extern uint32_t uget32le (const void *data);

/**
 * Load a 32-bit big-endian value from an unaligned address
 * @arg data
 *      The unaligned address
 * @return
 *      The memory value at given address
 */
extern uint32_t uget32be (const void *data);

#if ARCH_ENDIAN == ARCH_LITTLE_ENDIAN
#define uget16		uget16le
#define uget32		uget32le
#else
#define uget16		uget16be
#define uget32		uget32be
#endif

#endif // __USEFUN_H__