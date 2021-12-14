/*
    System clocks management
    Copyright (C) 2015 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_CLOCKS_H
#define _STM32_CLOCKS_H

#if defined STM32F0
#include "clocks-stm32f0.h"
#elif defined STM32F1
#include "clocks-stm32f1.h"
#elif defined STM32F2
#include "clocks-stm32f2.h"
#elif defined STM32F3
#include "clocks-stm32f3.h"
#elif defined STM32F4
#include "clocks-stm32f4.h"
#endif

#include <useful/useful.h>

/**
 * This macro tries to guess the frequency of the bus where a specific
 * peripherial is connected. This in the end expands to one of the
 * {HCLK,PCLK1,PCLK2,...)_FREQ variables.
 *
 * Note this does not work on USB peripherials as those use a separate
 * fixed 48MHz clock.
 *
 * Please use the respective XXX_CLOCK_FREQ (where XXX stands for DMA,
 * I2C, TIM, SPI, etc...) because some peripherials use additional clock
 * modifiers which aren't taken into account by this macro (e.g. timers
 * have a bus clock X2 multiplier if APB prescaler > 0 etc).
 *
 * @param x The name of the peripherial device with a underscore prepended
 *      (e.g. _SPI1, _USART4 etc). Underscore is required so that preprocessor
 *      won't expand SPI1, USART1 macros which are defined in cmsis headers.
 */
#define CLOCK_FREQ(x) \
    JOIN3 (JOIN2 (BUS_CLOCK_, JOIN2 (RCC_REG, x)), _FREQ, )
// using JOIN3 here just because CPP has problems with too many embedded JOIN2s

#endif // _STM32_CLOCKS_H
