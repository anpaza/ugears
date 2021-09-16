/*
    STM32 timer library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_TIM_H
#define _STM32_TIM_H

/**
 * @file tim.h
 *      This library simplifies usage of hardware timers.
 *
 *      All timers have a similar set of registers, but various timers
 *      may have a shorter list of supported features. To simplify
 *      code, we don't always do runtime checks if the requested feature
 *      is supported by given timer; instead, we put the responsability
 *      for these checks on the caller code.
 */

#include "cmsis.h"
#include <useful/useful.h>

/// Return the number of TIM peripherial associated with given hw feature
#define TIM_NUM(x)		JOIN2 (x, _TIM_NUM)
/// Get a pointer to TIM peripherial associated with given hw feature
#define TIM(x)			JOIN2 (TIM, TIM_NUM(x))
/// Guess timer clock frequency by hardware feature name
#define TIM_CLOCK_FREQ(x)	CLOCK_FREQ (JOIN2 (_TIM, TIM_NUM (x)))

/**
 * Start the timer counting.
 * @arg tim
 *      The timer to start
 */
INLINE_ALWAYS void tim_start (TIM_TypeDef *tim)
{ tim->CR1 |= TIM_CR1_CEN; }

/**
 * Set timer prescaler
 * @arg tim
 *      The timer to modify
 * @arg prescaler
 *      The prescaler to divide timer source clock with. The actual prescaler
 *      will be (prescaler+1).
 */
INLINE_ALWAYS void tim_prescaler (TIM_TypeDef *tim, uint16_t prescaler)
{ tim->PSC = prescaler; }

/**
 * Set timer period
 * @arg tim
 *      The timer to modify
 * @arg period
 *      Timer period. The actual period will be (period+1).
 */
INLINE_ALWAYS void tim_period (TIM_TypeDef *tim, uint16_t period)
{ tim->ARR = period; }

/**
 * Настроить режим однократного срабатывания таймера.
 * @arg tim
 *      Таймер, который нужно настроить
 */
INLINE_ALWAYS void tim_oneshot (TIM_TypeDef *tim)
{
    tim->CR1 = TIM_CR1_OPM;             // режим однократного срабатывания
    tim->SR = 0;                        // сбрасываем флаги прерываний
    tim->DIER = TIM_DIER_UIE;           // включаем прерывание при обновлении счётчика
}

#endif // _STM32_TIM_H
