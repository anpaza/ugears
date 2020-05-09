#include "useful/useful.h"

/* Алгоритм деления Кнута, оптимизированный для требуемой разрядности
 * делимого и делителя.
 */
uint32_t udiv64_32 (uint64_t u, uint32_t v)
{
    // Сдвигаем влево v (вместе с u), насколько это возможно без переполнения
    // Это а) максимизирует точность на первом этапе апроксимации и б) исключит деление на v1=0.
    uint32_t s = 0;
    if (!(v & 0xffff0000)) v <<= 16, s += 16;
    if (!(v & 0xff000000)) v <<=  8, s +=  8;
    if (!(v & 0xf0000000)) v <<=  4, s +=  4;
    if (!(v & 0xc0000000)) v <<=  2, s +=  2;
    if (!(v & 0x80000000)) v <<=  1, s +=  1;
    u <<= s;

    // u32 это третья и четвёртая 16-битная составляющие u
    uint32_t u32 = u >> 32;
    if (u32 >= v)
        // divide overflow
        return 0xffffffff;

    // v1 и v0 это старшая и младшая 16-битные составляющие v
    uint32_t v1 = v >> 16;
    uint32_t v0 = v & 0xffff;

    // u1 и u0 это вторая и первая 16-битные составляющие u
    uint32_t u1 = (((uint32_t)u) >> 16) & 0xffff;
    uint32_t u0 = ((uint32_t)u) & 0xffff;

    // Апроксимация старших 16 бит результата
    // через деление старших 32 бит u на верхние 16 бит v
    uint32_t q1 = u32 / v1;
    uint32_t rem = u32 - (q1 * v1);

    // Поправляем апроксимацию по нижним 16 битам v
    uint32_t q1v0 = q1 * v0;
    while (q1v0 > (rem << 16) + u1)
    {
        q1--;
        q1v0 -= v0;
        rem += v1;
        if (rem > 0xffff)
            break;
    }

    // Вычисляем нижние 16 бит результата
    // делением старших 32 бит остатка u на верхние 16 бит v
    uint32_t u21 = (uint32_t)(u >> 16) - (q1 * v);
    uint32_t q0 = u21 / v1;
    rem = u21 - (q0 * v1);

    // Опять поправляем апроксимацию по нижним 16 битам v
    uint32_t q0v0 = q0 * v0;
    while (q0v0 > (rem << 16) + u0)
    {
        q0--;
        q0v0 -= v0;
        rem += v1;
        if (rem > 0xffff)
            break;
    }

    return (q1 << 16) + q0;
}