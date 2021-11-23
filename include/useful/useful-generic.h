/*
    Useful small inline functions
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _USEFUL_GENERIC_H
#define _USEFUL_GENERIC_H

// Keep IDEs happy
#include "useful.h"

#ifndef _USEFUL_ROL8
#define _USEFUL_ROL8
/// Циклическое вращение 8-х битного числа влево данное количество раз.
INLINE_ALWAYS uint8_t rol8 (uint8_t s, uint8_t count)
{ count &= 7; return (uint8_t)(((s) << count) | ((s) >> (8 - count))); }
#endif

#ifndef _USEFUL_ROR8
#define _USEFUL_ROR8
/// Циклическое вращение 8-х битного числа вправо данное количество раз.
INLINE_ALWAYS uint8_t ror8 (uint8_t s, uint8_t count)
{ count &= 7; return (uint8_t)(((s) >> count) | ((s) << (8 - count))); }
#endif

#ifndef _USEFUL_ROL32
#define _USEFUL_ROL32
/// Циклическое вращение 32-х битного числа влево данное количество раз.
INLINE_ALWAYS uint32_t rol32 (uint32_t s, uint8_t count)
{ return (uint32_t)(((s) << count) | ((s) >> (32 - count))); }
#endif

#ifndef _USEFUL_ROR32
#define _USEFUL_ROR32
/// Циклическое вращение 32-х битного числа вправо данное количество раз.
INLINE_ALWAYS uint32_t ror32 (uint32_t s, uint8_t count)
{ return (uint32_t)(((s) >> count) | ((s) << (32 - count))); }
#endif

// -------------------------------------------------------------------------- //

#ifndef _USEFUL_BSWAP16
#define _USEFUL_BSWAP16
/// Изменения порядка байт в 16-битном числе.
INLINE_ALWAYS uint16_t bswap16 (uint16_t x)
{ return (uint16_t)(((x >> 8) | (x << 8)) & 0xffff); }
#endif

#ifndef _USEFUL_BSWAP32
#define _USEFUL_BSWAP32
/// Изменения порядка байт в 32-битном числе.
INLINE_ALWAYS uint32_t bswap32 (uint32_t x)
{ return (uint32_t)((x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24)); }
#endif

// -------------------------------------------------------------------------- //

#ifndef _USEFUL_FLS32
#define _USEFUL_FLS32
/// Получить номер старшего значашего бита, 0..31
INLINE_ALWAYS uint32_t fls32 (uint32_t bits)
{
    unsigned r = 0;
    if (bits & 0xffff0000) { r += 16; bits >>= 16; }
    if (bits & 0x0000ff00) { r +=  8; bits >>=  8; }
    if (bits & 0x000000f0) { r +=  4; bits >>=  4; }
    if (bits & 0x0000000c) { r +=  2; bits >>=  2; }
    if (bits & 0x00000002) { r +=  1;              }
    return r;
}
#endif

// -------------------------------------------------------------------------- //

#ifndef _USEFUL_UDIV_64_32
#define _USEFUL_UDIV_64_32
/// Деление 64-битного числа на 32-битное, возвращает 0xffffffff в случае переполнения
EXTERN_C uint32_t udiv64_32 (uint64_t u, uint32_t v);
#endif

// -------------------------------------------------------------------------- //

#ifndef _USEFUL_BITGET
#define _USEFUL_BITGET
/// Получить состояние указанного бита в массиве бит по указанному адресу
INLINE_ALWAYS bool bitget (void *data, unsigned n)
{
    return (((long *)data) [n / (sizeof (long) * 8)] &
            (1L << (n & (sizeof (long) * 8 - 1)))) != 0;
}
#endif

#ifndef _USEFUL_BITSET
#define _USEFUL_BITSET
/// Установить бит в 1 в массиве бит по указанному адресу
INLINE_ALWAYS void bitset (void *data, unsigned n)
{
    ((long *)data) [n / (sizeof (long) * 8)] |= 1L << (n & (sizeof (long) * 8 - 1));
}
#endif

#ifndef _USEFUL_BITCLR
#define _USEFUL_BITCLR
/// Установить бит в 0 в массиве бит по указанному адресу
INLINE_ALWAYS void bitclr (void *data, unsigned n)
{
    ((long *)data) [n / (sizeof (long) * 8)] &= ~(1L << (n & (sizeof (long) * 8 - 1)));
}
#endif

// -------------------------------------------------------------------------- //

/*
 * Atomic compare-and-exchange operation.
 * Compares @a value with @a expected. If equal, @a value becomes @a desired
 * and function returns true. If not, @a desired becomes @a value and function
 * returns false.
 *
 * @param value A pointer to value to exchange with
 * @param expected A pointer to the expected value of @a value
 * @param desired The value that's desired to end up in @a value
 * @return true if @a value become @a desired, false if @a expected
 *      become @a value.
 */
#define DECLARE_CMPXCHG(suffix,type) \
INLINE_ALWAYS bool cmpxchg_##suffix (type *value, type *expected, type desired) \
{ \
    return __atomic_compare_exchange_n (value, expected, desired, \
                                        true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); \
}

#ifndef _USEFUL_CMPXCHG8
#define _USEFUL_CMPXCHG8
DECLARE_CMPXCHG (u8, uint8_t)
#endif

#ifndef _USEFUL_CMPXCHG16
#define _USEFUL_CMPXCHG16
DECLARE_CMPXCHG (u16, uint16_t)
#endif

#ifndef _USEFUL_CMPXCHG32
#define _USEFUL_CMPXCHG32
DECLARE_CMPXCHG (u32, uint32_t)
#endif

#ifndef _USEFUL_CMPXCHG64
#define _USEFUL_CMPXCHG64
DECLARE_CMPXCHG (u64, uint64_t)
#endif

#if __UINTPTR_MAX__ == __UINT16_MAX__
#  define cmpxchg_p(v,e,d) cmpxchg_u16((uintptr_t *)v, (uintptr_t *)e, (uintptr_t)d)
#elif __UINTPTR_MAX__ == __UINT32_MAX__
#  define cmpxchg_p(v,e,d) cmpxchg_u32((uintptr_t *)v, (uintptr_t *)e, (uintptr_t)d)
#elif __UINTPTR_MAX__ == __UINT64_MAX__
#  define cmpxchg_p(v,e,d) cmpxchg_u64((uintptr_t *)v, (uintptr_t *)e, (uintptr_t)d)
#else
#  error "Unknown size of uintptr_t type"
#endif

// -------------------------------------------------------------------------- //

/*
 * Atomically increment a memory cell by given value and return memory content
 * after increment.
 *
 * @param data A pointer to memory cell
 * @param value The amount to increment the cell
 * @return Memory value after increment
 */
#define DECLARE_ATOMIC_ADD_FETCH(suffix,type) \
INLINE_ALWAYS type atomic_add_fetch_##suffix (type *data, type value) \
{ return __atomic_add_fetch (data, value, __ATOMIC_SEQ_CST); }

#ifndef _ATOMIC_ADD_FETCH_U8
#define _ATOMIC_ADD_FETCH_U8
DECLARE_ATOMIC_ADD_FETCH (u8, uint8_t)
#endif

#ifndef _ATOMIC_ADD_FETCH_U16
#define _ATOMIC_ADD_FETCH_U16
DECLARE_ATOMIC_ADD_FETCH (u16, uint16_t)
#endif

#ifndef _ATOMIC_ADD_FETCH_U32
#define _ATOMIC_ADD_FETCH_U32
DECLARE_ATOMIC_ADD_FETCH (u32, uint32_t)
#endif

#ifndef _ATOMIC_ADD_FETCH_U64
#define _ATOMIC_ADD_FETCH_U64
DECLARE_ATOMIC_ADD_FETCH (u64, uint64_t)
#endif

// -------------------------------------------------------------------------- //

/*
 * Atomically decrement a memory cell by given value and return memory content
 * after decrement.
 *
 * @param data A pointer to memory cell
 * @param value The amount to decrement the cell
 * @return Memory value after decrement
 */
#define DECLARE_ATOMIC_SUB_FETCH(suffix,type) \
INLINE_ALWAYS type atomic_sub_fetch_##suffix (type *data, type value) \
{ return __atomic_sub_fetch (data, value, __ATOMIC_SEQ_CST); }

#ifndef _ATOMIC_SUB_FETCH_U8
#define _ATOMIC_SUB_FETCH_U8
DECLARE_ATOMIC_SUB_FETCH (u8, uint8_t)
#endif

#ifndef _ATOMIC_SUB_FETCH_U16
#define _ATOMIC_SUB_FETCH_U16
DECLARE_ATOMIC_SUB_FETCH (u16, uint16_t)
#endif

#ifndef _ATOMIC_SUB_FETCH_U32
#define _ATOMIC_SUB_FETCH_U32
DECLARE_ATOMIC_SUB_FETCH (u32, uint32_t)
#endif

#ifndef _ATOMIC_SUB_FETCH_U64
#define _ATOMIC_SUB_FETCH_U64
DECLARE_ATOMIC_SUB_FETCH (u64, uint64_t)
#endif

// -------------------------------------------------------------------------- //

#ifndef _ATOMIC_IRQ_STATE
#define _ATOMIC_IRQ_STATE

// No generic cross-platform way to handle IRQ state, so no-ops

/// A datatype to hold the current IRQ enabled state
typedef uint8_t atomic_irq_state_t;

/// A value meaning "IRQs are enabled"
#define ATOMIC_IRQ_STATE_ENA 1
/// A value meaning "IRQs are disabled"
#define ATOMIC_IRQ_STATE_DIS 0

/**
 * Get current IRQ enabled state
 * @return a value of atomic_irq_state_t type
 */
INLINE_ALWAYS atomic_irq_state_t atomic_irq_get_state ()
{ return ATOMIC_IRQ_STATE_ENA; }

/**
 * Set IRQ enabled state according to passed value.
 * @param state The new IRQ enabled state
 */
INLINE_ALWAYS void atomic_irq_set_state (atomic_irq_state_t state)
{ (void)state; }

#endif // _ATOMIC_IRQ_STATE

#endif // _USEFUL_GENERIC_H
