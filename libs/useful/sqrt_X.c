#include "useful/useful.h"

//#define USE_SQRT_NEWTON
#define USE_SQRT_BITROW

#ifdef USE_SQRT_NEWTON

/**
 * Оптимизированное вычисление квадратного корня по методу Ньютона.
 * Использует аппаратное деление и умножение.
 * Работает примерно на 30% медленнее побитного алгоритма "в столбик" (на x86).
 */
uint32_t fp_sqrt_X (uint32_t x, unsigned n)
{
    // Вырожденные случаи обрабатываем сразу
    if (x < 2)
        return x;

    // Вычислим начальное приближение результата.
    // Если x ~= 2^(2*n), тогда начальное приближение ~= 2^n.
    // Надо быть аккуратным, чтобы обработать и отрицательные значения n (числа менее единицы).
    int po2 = ((int)fls32 (x) - (int)n) / 2;

    // Начальное приближение результата
    uint32_t r = 1 << (n + po2);

    for (;;)
    {
        uint32_t old_r = r;
        r = (r + (udiv64_32 (((uint64_t)x) << n, r)) + 1) / 2;
        old_r = ABS ((int)old_r - (int)r);
        if (old_r < 8)
            break;
    }

    return r;
}

#endif // USE_SQRT_NEWTON

#ifdef USE_SQRT_BITROW

/**
 * Вычисление корня в столбик, в двоичной системе счисления.
 * Использует только сложение, сравнение и логический сдвиг.
 */
uint32_t fp_sqrt_X (uint32_t x, unsigned n)
{
    // Вырожденные случаи обрабатываем сразу
    if (x == 0)
        return x;

    uint32_t r = 0;
    uint32_t bit = 1 << ((fls32 (x) & ~1) | (n & 1));
    uint32_t max_shift = 16;

    for (;;)
    {
        // Вычисляем корень из целочисленной части и половину бит дробной части
        while (bit)
        {
            if (x >= r + bit)
            {
                x -= r + bit;
                r = (r >> 1) + bit;
            }
            else
                r >>= 1;

            bit >>= 2;
        }

        // Теперь делаем ещё n/2 итераций для уточнения дробной части
        if ((n == 0) || (max_shift < 2))
            break;

        unsigned nn = (n > max_shift) ? (max_shift - (n & 1)) : n;
        max_shift >>= 1;
        x <<= nn;
        bit = 1 << (nn - (2 - (nn & 1)));
        r <<= nn;
        n -= nn;
    }

    // Округляем последний бит
    if (x >= r)
        r++;

    return r;
}

#endif // USE_SQRT_BITROW
