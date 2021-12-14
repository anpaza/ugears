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
/**
 * Guess timer clock frequency by hardware feature name.
 *
 * Note this does not take into account the RCC_DCKCFGR_TIMPRE bit found on
 * some platforms.
 *
 * For now we suppose if respective bus clock divider is 1,
 * bus clock is used as-is, otherwise it is doubled.
 */
#define TIM_CLOCK_FREQ(x) \
    (CLOCK_FREQ (JOIN2 (_TIM, TIM_NUM (x))) * \
     ((JOIN2 (JOIN2 (BUS_CLOCK_, JOIN2 (RCC_REG, JOIN2 (_TIM, TIM_NUM (x)))), _DIV) == 1) ? 1 : 2))
/// Return the IRQ number corresponding to this timer
#define TIM_IRQ_NUM(x)		JOIN2 (x, _TIM_IRQ_NUM)
/// Return the IRQ handler name corresponding to this timer
#define TIM_IRQ_HANDLER(x)	JOIN2 (x, _TIM_IRQ_HANDLER)


/// Timer counts only once then stops (default mode)
#define TIM_CONF_MODE_ONESHOT	0x00000000
/// Timer repeatedly reloads the counter each time it ends
#define TIM_CONF_MODE_REPEATED	0x00000001
/// Isolate timer mode bits with this mask
#define TIM_CONF_MODE		0x00000001

/// Timer counter increments until it reaches period (default mode)
#define TIM_CONF_DIR_UP		0x00000000
/// Timer counter decrements until it reaches zero
#define TIM_CONF_DIR_DOWN	0x00000002
/// Isolate timer counting direction option with this mask
#define TIM_CONF_DIR		0x00000002

/// Enable IRQ on timer expiration
#define TIM_CONF_IRQ		0x00000004

/**
 * Configure basic properties of a timer.
 *
 * @param tim The timer to configure
 * @param prescaler The prescaler counter (real prescaler is prescaler+1)
 * @param period Timer period (real period is period+1)
 * @param mode Timer configuration options (see TIM_CONF_xxx bits)
 * @return false if timer does not support requested mode
 */
EXTERN_C bool tim_init (TIM_TypeDef *tim, uint16_t prescaler, uint16_t period,
                        uint32_t conf);

/**
 * Start the timer.
 * @arg tim The timer to start
 */
INLINE_ALWAYS void tim_start (TIM_TypeDef *tim)
{ tim->CR1 |= TIM_CR1_CEN; }

/**
 * Stop the timer and clear the counter.
 * @arg tim The timer to stop
 */
INLINE_ALWAYS void tim_stop (TIM_TypeDef *tim)
{
    tim->CR1 &= ~TIM_CR1_CEN;
    tim->CNT = 0;
}

/**
 * Check if timer is running
 * @param tim The timer to check
 * @return true if timer is running, false if it is stopped
 */
INLINE_ALWAYS bool tim_is_running (TIM_TypeDef *tim)
{ return (tim->CR1 & TIM_CR1_CEN) != 0; }

/**
 * Timer counts up (from 0 to period) or down (from period to 0)
 * @param tim The timer to set up
 * @param up true for up, false for down
 */
INLINE_ALWAYS void tim_set_direction (TIM_TypeDef *tim, bool up)
{
    if (up)
        tim->CR1 &= ~TIM_CR1_DIR;
    else
        tim->CR1 |= ~TIM_CR1_DIR;
}

/**
 * Check if timer is counting up or down
 * @param tim The timer to check
 * @return true if timer counts up
 */
INLINE_ALWAYS bool tim_get_direction (TIM_TypeDef *tim)
{ return (tim->CR1 & TIM_CR1_DIR) == 0; }

/**
 * Set timer prescaler
 * @arg tim The timer to modify
 * @arg prescaler The prescaler to divide timer source clock with. The actual prescaler
 *      will be (prescaler+1).
 */
INLINE_ALWAYS void tim_set_prescaler (TIM_TypeDef *tim, uint16_t prescaler)
{ tim->PSC = prescaler; }

/**
 * Set timer period
 * @arg tim The timer to modify
 * @arg period Timer period. The actual period will be (period+1).
 */
INLINE_ALWAYS void tim_set_period (TIM_TypeDef *tim, uint16_t period)
{ tim->ARR = period; }

/**
 * Set timer's current counter
 * @param tim The timer to modify
 * @param counter Counter value
 */
INLINE_ALWAYS void tim_set_counter (TIM_TypeDef *tim, uint16_t counter)
{ tim->CNT = counter; }

/**
 * Reset timer's counter to 0 (if counting up) or to reload value
 * (if counting down).
 * @param tim The timer to reset
 */
INLINE_ALWAYS void tim_reset_counter (TIM_TypeDef *tim)
{ tim_set_counter (tim, tim_get_direction (tim) ? 0 : tim->ARR); }

/**
 * Get current timer counter
 * @param tim The timer
 * @return Current timer counter
 */
INLINE_ALWAYS uint32_t tim_get_counter (TIM_TypeDef *tim)
{ return tim->CNT; }

/**
 * Set up the timer for one shot counting.
 * @arg tim The timer to set up
 */
INLINE_ALWAYS void tim_set_oneshot (TIM_TypeDef *tim)
{ tim->CR1 = (tim->CR1 & ~TIM_CR1_ARPE) | TIM_CR1_OPM; }

/**
 * Set up the timer for repeated counting.
 * @arg tim The timer to set up
 */
INLINE_ALWAYS void tim_set_repeated (TIM_TypeDef *tim)
{ tim->CR1 = (tim->CR1 & ~TIM_CR1_OPM) | TIM_CR1_ARPE; }

/**
 * Enable or disable timer IRQ generation.
 * @arg tim The timer to set up
 */
INLINE_ALWAYS void tim_set_irq (TIM_TypeDef *tim, bool enable)
{
    if (enable)
    {
        tim->SR &= ~TIM_SR_UIF;
        tim->DIER |= TIM_DIER_UIE;
    }
    else
        tim->DIER &= ~TIM_DIER_UIE;
}

#endif // _STM32_TIM_H
