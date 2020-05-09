/*
    Accelerated computation of arctangent, arcsine and arccosine
    in floating-point format.

    Copyright (C) 2016 Andrey Zabolotnyi <zapparello@ya.ru>
    Based on algorithm by Nikitin V.F. (C) 2000

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/useful.h"
#include "useful/fpmath.h"

// Тангенс от 15 градусов
#define TAN15(n)	FxPs (0.26795, n)
// Квадратный корень из 3
#define SQRT3(n)	FxPs (1.732051, n)
// Угол PI/6 в единицах 360°=65536
#define ANGLE_PI_6	((int32_t)(32768.0 / 6.0))

// Множитель для преобразования из радиан в угол
#define MUL_1_PI(n)	FxPu ((32768.0 / 3.141592), n)
// Множитель для преобразования из радиан в треть угла
#define MUL_PI_3(n)	FxPu ((32768.0 / 3.141592) / 3.0, n)
// Множитель для преобразования из радиан в одну пятую угла
#define MUL_PI_5(n)	FxPu ((32768.0 / 3.141592) / 5.0, n)

int32_t fp_atan2_16 (int32_t y, int32_t x)
{
    unsigned flags = 0;
#define FLAG_NEG_X	1
#define FLAG_NEG_Y	2
#define FLAG_INV	4
#define FLAG_NEG	8

    // Приводим оба аргумента к положительным значениям
    if (y < 0)
    {
        y = -y;
        flags ^= FLAG_NEG_Y;
    }
    if (x < 0)
    {
        x = -x;
        flags ^= FLAG_NEG_X;
    }

    // Приводим к диапазону 0-45 градусов
    if (y > x)
    {
        flags |= FLAG_INV;
        XCHG (x, y);
    }

    if (x == 0)
        // Неопределённость вида 0/0
        return 0;

    int32_t res = 0;

    // Делим y на x, беззнаково т.к. x и y гарантировано положительные
    // Результат гарантировано меньше либо равен 2^15
    y = udiv64_32 (((uint64_t)y) << 15, x);

    // Приводим y к диапазону -15..+15 градусов используя формулу
    // atan (x) = PI/6 + atan ((x * sqrt (3) - 1) / (x + sqrt (3)))
    while (y > TAN15 (15))
    {
        res += ANGLE_PI_6;
        y = (y * SQRT3 (15) - FxPs (1, 15+15)) / (y + SQRT3 (15));
    }

    // Для увеличения точности будем работать с y в беззнаковом режиме
    if (y < 0)
    {
        y = -y;
        flags |= FLAG_NEG;
    }

    // Апроксимация разложением в ряд по Маклорену.
    // Для диапазона -15..15 градусов и точности результата достаточно первых 3-х членов.
    // atan = y - y^3 / 3 + y^5 / 5 = y * (1 - y^2 * 0.333 + y^4 * 0.2)
    // Действовать нужно осторожно чтобы, с одной стороны, не потерять точность,
    // с другой стороны, чтобы не было переполнения при макс. значениях на входе
    // Диапазон значений y на входе от 0 до tg(15 градусов)*2^15 = 8780
    uint32_t y2 = ((uint32_t)y * (uint32_t)y) >> 11; // ФТ19
    uint32_t y4 = (y2 * y2) >> 19; // ФТ19
    uint32_t a = MUL_1_PI (4); // ФТ20
    uint32_t b = (y2 * MUL_PI_3 (4)) >> 19; // ФТ20
    uint32_t c = (y4 * MUL_PI_5 (4)) >> 19; // ФТ20
    y = (((uint32_t)y) * (a - b + c)) >> 19; // (ФТ15*ФТ20)>>19 = ФТ16

    if (flags & FLAG_NEG)
        y = -y;

    res += y;

    if (flags & FLAG_INV)
        res = 16384 - res;

    switch (flags & (FLAG_NEG_X | FLAG_NEG_Y))
    {
        case FLAG_NEG_X:
            res = 32768 - res;
            break;

        case FLAG_NEG_Y:
            res = 65536 - res;
            break;

        case FLAG_NEG_X | FLAG_NEG_Y:
            res += 32768;
            break;
    }

    return res;
}

/*
 * asin(x) = atan(1/sqrt(1 - x^2))
 */
int32_t fp_asin_16 (int32_t x, unsigned n)
{
    if (x == 0 || n > 30)
        return 0;

    int32_t one = (1 << n);
    if (x <= -one)
        return -16384; // -PI/2
    if (x >= one)
        return 16384; // PI/2

    bool neg = x < 0;
    if (neg)
        x = -x;

    // Приводим x к формату ФТ.16
    if (n > 16)
        x >>= (n - 16);
    else
        x <<= (16 - n);
    // Вычисляем x^2 в ФТ.16
    uint32_t t = ((uint32_t)x * (uint32_t)x) >> 16;
    // sqrt (1.0 - x^2)
    t = fp_sqrt_X ((1U << 16) - t, 16);
    if (t == 0)
        return neg ? -16384 : 16384;
    // x / sqrt (1.0 - x^2)
    t = (((uint32_t)x) << 16) / t;
    return fp_atan2_16 (neg ? -t : t, (1U << 16));
}

// acos(x) = pi/2 - asin(x)
int32_t fp_acos_16 (int32_t x, unsigned n)
{
    return (16384 - fp_asin_16 (x, n)) & 0xffff;
}
