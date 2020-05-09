/*
    Useful function for embedded systems
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __FPMATH_H__
#define __FPMATH_H__

#include <stdint.h>

/**
 * @file fpmath.h
 *      Fixed-point math routines for various fixed-point formats.
 */

/// Convert a floating-point constant number to 32-bit unsigned fixed-point
#define FxPu(x,n)	((uint32_t)((x) * ((float)(1U << (n))) + 0.5))
/// Convert a floating-point constant number to 32-bit signed fixed-point
#define FxPs(x,n)	((int32_t)((x) * ((float)(1U << (n))) + 0.5))

/// Convert a floating-point constant number < 1 to 0.32 fixed-point
#define FxPu32(x)	((uint32_t)((x) * 4294967296.0L + 0.5))
/// Convert a floating-point constant number to 20.12 fixed-point
#define FxPu12(x)	FxPu (x, 12)
/// Convert a floating-point constant number to 24.8 fixed-point
#define FxPu8(x)	FxPu (x, 8)
/// Convert a floating-point constant number to 25.7 fixed-point
#define FxPu7(x)	FxPu (x, 7)

/**
 * Return the result of multiplication of two fixed-point numbers.
 * The caller must take care that
 * (integer bits of X + xfb) + (integer bits of Y + yfb)
 * is less than 32, otherwise the result is unpredictable.
 * @arg x
 *      The number to multiply
 * @arg xfb
 *      Fractional bits in X
 * @arg y
 *      The multiplier
 * @arg yfb
 *      Fractional bits in Y
 * @arg rfb
 *      Fractional bits in result
 * @return
 *      The result of fixed-point multiplication
 */
static inline uint32_t fp_umul (uint32_t x, uint32_t xfb, uint32_t y, uint32_t yfb, uint32_t rfb)
{
    if (xfb + yfb < rfb)
        return (x * y) << (rfb - (xfb + yfb));
    else
        return (x * y) >> ((xfb + yfb) - rfb);
}

/**
 * Same as fp_umul(), but with signed args.
 * @arg x
 *      The number to multiply
 * @arg xfb
 *      Fractional bits in X
 * @arg y
 *      The multiplier
 * @arg yfb
 *      Fractional bits in Y
 * @arg rfb
 *      Fractional bits in result
 * @return
 *      The result of fixed-point multiplication
 */
static inline int32_t fp_smul (int32_t x, uint32_t xfb, int32_t y, uint32_t yfb, uint32_t rfb)
{
    if (xfb + yfb < rfb)
        return (x * y) << (rfb - (xfb + yfb));
    else
        return (x * y) >> ((xfb + yfb) - rfb);
}

/**
 * Return the upper 32 bits of the 64-bit result of multiplication
 * of two 32-bit unsigned integers.
 *
 * This is equivalent to multiplication of two numbers in the 0.32
 * fixed-point format.
 * @arg x
 *      The number to multiply
 * @arg y
 *      The multiplier
 * @return
 *      Returns (x * y) >> 32
 */
static inline uint32_t umul_h32 (uint32_t x, uint32_t y)
{
    uint32_t r;

    // Optimize obvious cases

    if (__builtin_constant_p (x) && __builtin_constant_p (y))
        return ((uint64_t)x * (uint64_t)y) >> 32;

    if (__builtin_constant_p (x) && (x < 2))
        return 0;

    if (__builtin_constant_p (y) && (y < 2))
        return 0;

#if defined ARCH_ARM
#if __CORTEX_M < 3
    /* Cortex-M0 & friends don't have UMULL, so we'll multiply as:
     * R = (XL + XH*2^16) * (YL + YH*2^16) =
     *     (XL*YL) + (XL*YH*2^16) + (XH*YL*2^16) + (XH*YH*2^32)
     *
     * so, given that XL, XH, YL, YH are all 16-bit integers, then:
     * R>>32 = (XL*YH)>>16 + (XH*YL)>>16 + (XH*YH)
     */
    uint32_t tmp1, tmp2;
    __asm__ __volatile__ (
            "ldr    %2, =#0xffff\n"
            "mov    %1, %3\n"
            "and    %1, %1, %2          // %1 = XL\n"
            "and    %2, %4, %2          // %2 = YL\n"
            "lsr    %3, #16             // %3 = XH\n"
            "lsr    %4, #16             // %4 = YH\n"
            "mov    %0, %1\n"
            "mul    %0, %0, %4\n"
            "lsr    %0, #16             // r = (XL*YH)>>16\n"
            "mov    %1, %3\n"
            "mul    %1, %1, %2\n"
            "lsr    %1, #16\n"
            "add    %0, %1              // r += (XH*YL)>>16\n"
            "mov    %1, %3\n"
            "mul    %1, %1, %4\n"
            "add    %0, %1              // r += (XH*YH)\n"
            : "=&l" (r), "=&l" (tmp1), "=&l" (tmp2), "+l" (x), "+l" (y)
    );
#else
    uint32_t tmp;
    __asm__ __volatile__ (
            "umull   %1, %0, %2, %3\n"
            : "=r" (r), "=r" (tmp)
            : "r" (x), "r" (y)
    );
#endif
#else
    r = ((uint64_t)x * (uint64_t)y) >> 32;
#endif

    return r;
}

/**
 * Вычисляет скользящую среднюю за 2^period последних отсчётов, без необходимости
 * хранения всех предыдущих отсчётов. Следует учесть, что это цифровой фильтр
 * первого порядка, который конвергирует к константному значению примерно за
 * 3*(2^period) отсчётов.
 *
 * Младшие @a period бит используются для накопления ошибки, поэтому чем этих
 * бит больше, тем результат точнее. Вместе с тем, это означает, что младшие
 * @a period бит используются для накопления ошибки, поэтому при поступлении
 * на вход функции постоянного @a x приведёт к тому, что @ma конвергирует к
 * значению, отличающемся от @a x не более чем на 2^period (на самом деле
 * алгоритм улучшен так, чтобы результат отличался не более чем на 2^(period-1))
 * @arg x
 *     Новый отсчёт
 * @arg ma
 *     Указатель на скользящую среднюю
 * @arg period
 *     Длина усредняемого периода, степень двойки
 */
static inline void update_moving_average_16 (int16_t x, int16_t *ma, uint8_t period)
{
#if defined ARCH_ARM
    // MA = ((MA * (N - 1)) + X) / N
    // ==> MA = MA + (X - MA) / N
    // *ma = ((x - *ma) >> period) + *ma
    // also we add the carry flag after '>>', gives better convergence
    uint32_t tmp;
    __asm__ __volatile__ (
        "ldrh	%0, [%3]\n"
        "sub	%1, %0\n"
        "asr	%1, %2\n"
        "adc	%0, %1\n"
        "strh	%0, [%3]\n"
        : "=&r" (tmp), "+r" (x)
        : "Mr" (period), "r" (ma)
    );
#else
    *ma = ((x - *ma) >> period) + *ma;
#endif
}

/**
 * То же, что и update_moving_average_16(), но в качестве аккумулятора
 * использует 32-битное знаковое целое.
 * @arg x
 *     Новый отсчёт
 * @arg ma
 *     Указатель на скользящую среднюю
 * @arg period
 *     Длина усредняемого периода, степень двойки
 */
static inline void update_moving_average_32 (int32_t x, int32_t *ma, uint8_t period)
{
#if defined ARCH_ARM
    // MA = ((MA * (N - 1)) + X) / N
    // ==> MA = MA + (X - MA) / N
    // *ma = ((x - *ma) >> period) + *ma
    // also we add the carry flag after '>>', gives better convergence
    uint32_t tmp;
    __asm__ __volatile__ (
        "ldr	%0, [%3]\n"
        "sub	%1, %0\n"
        "asr	%1, %2\n"
        "adc	%0, %1\n"
        "str	%0, [%3]\n"
        : "=&r" (tmp), "+r" (x)
        : "Mr" (period), "r" (ma)
    );
#else
    *ma = ((x - *ma) >> period) + *ma;
#endif
}

/**
 * Return the sine of the angle
 * @arg angle
 *      Angle, 90° = 64, 180° = 128, 270° = 192 etc.
 * @return
 *      The sine value in signed 1.8 format
 */
extern int fp_sin_8 (uint8_t angle);

/**
 * Return the cosine of the angle
 * @arg angle
 *      Angle, 90° = 64, 180° = 128, 270° = 192 etc.
 * @return
 *      The cosine value in signed 1.8 format
 */
static inline int fp_cos_8 (uint8_t angle)
{ return fp_sin_8 (angle + 64); }

/**
 * Вычисление арктангенса (y / x), результат в формате ФТ16.
 * В отличие от простого арктангенса, правильно вычисляет квадрант результата.
 * Точность результата - не ниже 0.01 градуса.
 * @arg y
 *      Первый аргумент
 * @arg x
 *      Второй аргумент
 * @return
 *      Угол в условных единицах от 0 до 65535, 16384=90°, 32768=180°, 49152=270°, 65536=360°.
 */
extern int32_t fp_atan2_16 (int32_t y, int32_t x);

/**
 * Вычисление квадратного корня из числа с фиксированной точкой.
 * Работает с любым форматом чисел ФТ.
 * @arg x
 *      Число, из которого требуется извлечь квадратный корень
 * @arg n
 *      Количество бит после запятой в формате ФТ
 * @return
 *      Квадратный корень из числа, ФТ
 */
extern uint32_t fp_sqrt_X (uint32_t x, unsigned n);

/**
 * Возвращает арксинус от аргумента.
 * @arg x
 *      Значение синуса
 * @arg n
 *      Количество бит в дробной части числа x с ФТ, 0..16
 * @return
 *      Значение угла, 0..65535, 16384=90°, 32768=180°, 49152=270°, 65536=360°.
 */
extern int32_t fp_asin_16 (int32_t x, unsigned n);

#endif // __FPMATH_H__
