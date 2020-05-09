/*
    STM32 GPIO helpers library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears.h"

/**
 * STM32F1xx configures port via the AFIO unit.
 * Other microcontroller types use the SYSCFG unit, although
 * register names are the same.
 */
#if !defined SYSCFG && defined AFIO
#   define SYSCFG AFIO
#endif

void exti_config (uint32_t exti, uint32_t conf)
{
    uint32_t mask = (1 << exti);
    if (conf & EXTI_INT)
        EXTI->IMR |= mask;
    else
        EXTI->IMR &= ~mask;

    if (conf & EXTI_EVENT)
        EXTI->EMR |= mask;
    else
        EXTI->EMR &= ~mask;

    if (conf & EXTI_RISING)
        EXTI->RTSR |= mask;
    else
        EXTI->RTSR &= ~mask;

    if (conf & EXTI_FALLING)
        EXTI->FTSR |= mask;
    else
        EXTI->FTSR &= ~mask;

    if (exti < 16)
    {
        volatile uint32_t *exticr = &SYSCFG->EXTICR [exti / 4];
        uint32_t shift = (exti & 3) * 4;
        mask = 0x0000000f << shift;
        *exticr = (*exticr & ~mask) |
            ((conf & GPIO_PORT_MASK) >> GPIO_PORT_SHIFT) << shift;
    }
}
