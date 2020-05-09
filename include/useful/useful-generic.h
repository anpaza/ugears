/*
    Useful small inline functions
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __USEFUL_GENERIC_H__
#define __USEFUL_GENERIC_H__

#ifndef __USEFUL_ROL8
#define __USEFUL_ROL8
/// Циклическое вращение 8-х битного числа влево данное количество раз.
static inline uint8_t rol8 (uint8_t s, uint8_t count)
{ count &= 7; return (uint8_t)(((s) << count) | ((s) >> (8 - count))); }
#endif

#ifndef __USEFUL_ROR8
#define __USEFUL_ROR8
/// Циклическое вращение 8-х битного числа вправо данное количество раз.
static inline uint8_t ror8 (uint8_t s, uint8_t count)
{ count &= 7; return (uint8_t)(((s) >> count) | ((s) << (8 - count))); }
#endif

#ifndef __USEFUL_ROL32
#define __USEFUL_ROL32
/// Циклическое вращение 32-х битного числа влево данное количество раз.
static inline uint32_t rol32 (uint32_t s, uint8_t count)
{ return (uint32_t)(((s) << count) | ((s) >> (32 - count))); }
#endif

#ifndef __USEFUL_ROR32
#define __USEFUL_ROR32
/// Циклическое вращение 32-х битного числа вправо данное количество раз.
static inline uint32_t ror32 (uint32_t s, uint8_t count)
{ return (uint32_t)(((s) >> count) | ((s) << (32 - count))); }
#endif

#ifndef __USEFUL_BSWAP16
#define __USEFUL_BSWAP16
/// Изменения порядка байт в 16-битном числе.
static inline uint16_t bswap16 (uint16_t x)
{ return (uint16_t)(((x >> 8) | (x << 8)) & 0xffff); }
#endif

#ifndef __USEFUL_BSWAP32
#define __USEFUL_BSWAP32
/// Изменения порядка байт в 32-битном числе.
static inline uint32_t bswap32 (uint32_t x)
{ return (uint32_t)((x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24)); }
#endif

#ifndef __USEFUL_FLS32
#define __USEFUL_FLS32
/// Получить номер старшего значашего бита, 0..31
static inline uint32_t fls32 (uint32_t bits)
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

#ifndef __USEFUL_UDIV_64_32
#define __USEFUL_UDIV_64_32
/// Деление 64-битного числа на 32-битное, возвращает 0xffffffff в случае переполнения
extern uint32_t udiv64_32 (uint64_t u, uint32_t v);
#endif

#endif // __USEFUL_GENERIC_H__
