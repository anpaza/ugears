/*
    SysTick timer management
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_SYSTICK_H
#define _STM32_SYSTICK_H

#include "cmsis.h"
#include "nvic.h"
#include <useful/fpmath.h>

#ifndef SYSTICK_DIV8
/// If non-zero, timer counts SYSCLK/8 rather than SYSCLK.
#define SYSTICK_DIV8		0
#endif

/**
 * @file systick.h
 *      This file provides some shortcuts for managing and using the
 *      ARM Cortext-M family SysTick timer.
 */

#if SYSTICK_DIV8 != 0
#define SYSTICK_DIV 8
#else
#define SYSTICK_DIV 1
#endif

/**
 * Configure the SysTick timer with the given period in SYSCLK ticks
 * (if SYSTICK_DIV8 == 0) or SYSCLK/8 ticks (if SYSTICK_DIV8 != 0).
 * If you change SYSCLK_FREQ, the timer period in seconds will change
 * (but not in CPU clocks, of course).
 * @arg period
 *      Period length in SysTick ticks. The maximal value is 2^24.
 */
INLINE_ALWAYS void systick_config (uint32_t period)
{
    // stop the timer in case it's running
    SysTick->CTRL = 0;
    // load top and current values
    SysTick->LOAD = period - 1;
    SysTick->VAL = 0;
    // re-start the timer
    SysTick->CTRL = (SYSTICK_DIV8 ? 0 : SysTick_CTRL_CLKSOURCE_Msk) |
                    SysTick_CTRL_ENABLE_Msk;
}

/**
 * Enable SysTick_Handler.
 * @arg priority
 *      IRQ priority
 */
INLINE_ALWAYS void systick_int_enable (uint8_t priority)
{
    /* set Priority for Systick Interrupt */
    nvic_set_priority (SysTick_IRQn, priority);
    /* Enable SysTick IRQ */
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

/**
 * Disable SysTick_Handler.
 */
INLINE_ALWAYS void systick_int_disable ()
{
    /* Disable SysTick IRQ */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

/**
 * Enable or disable the SysTick timer.
 * Note that calling @a systick_config() will enable the timer as well.
 * @arg enable
 *      If non-zero, enable SysTick timer, otherwise disable
 */
INLINE_ALWAYS void systick_enable_set (bool enable)
{
    if (enable)
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    else
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

/**
 * Return the current SysTick timer counter
 */
INLINE_ALWAYS uint32_t systick_counter ()
{ return SysTick->VAL; }

/**
 * Return the SysTick timer counter reload value
 */
INLINE_ALWAYS uint32_t systick_reload ()
{ return SysTick->LOAD + 1; }

/**
 * Sleep for not less than given number of CPU clocks.
 *
 * The system clock should be running in order to use this function
 * with a period not less than 50 clocks.
 *
 * This function is inaccurate for very small delays; use __delay_clocks()
 * for very short delays.
 *
 * Never call this function directly as it depends on SYSTICK_DIV8.
 * @arg clocks
 *      Number of CPU clocks to delay execution.
 */
extern void _delay_clocks (uint32_t clocks);

/**
 * This smart inline function will generate a series of constant-execution-time
 * commands which summary will give the expected delay in CPU clocks.
 *
 * The correct expansion of this function depends on optimization options.
 * It is suggested that at least the -fpeel-loops option is used.
 * @arg clocks
 *      Number of CPU clocks to delay execution. Must be constant,
 *      otherwise the function will generate a mess.
 */
INLINE_ALWAYS __attribute__((always_inline))
void _delay_few_const_clocks (uint32_t clocks)
{
#ifdef MCU_CORE_CORTEX_M0
    // On Cortex-M0 unconditional branch takes 3 cycles
    for (; clocks >= 3; clocks -= 3)
        __asm__ __volatile__ ("b 1f\n1:\n");
#elif defined MCU_CORE_CORTEX_M3 || defined MCU_CORE_CORTEX_M4
    // On Cortex-M3 & M4 multiply-accumulate takes 2 cycles
    for (; clocks > 1; clocks -= 2)
    {
        uint32_t temp;
        __asm__ __volatile__ ("mla %0,%0,%0,%0" : "=r" (temp));
    }
#endif
    // On all cores NOP takes 1 clock
    for (; clocks > 0; clocks--)
        __asm__ __volatile__ ("nop");
}

/**
 * Delay execution for a number of CPU clocks.
 *
 * This macro optimally expands either to a series of simple
 * instructions for very small delays (up to 50 clocks), or
 * calls _delay_clocks() for larger timings.
 * @arg clocks
 *      Number of CPU clocks to delay execution.
 */
#define delay_clocks(clocks) \
    (__builtin_constant_p (clocks) ? \
        ((clocks < 50) ? _delay_few_const_clocks (clocks) : \
                         _delay_clocks (clocks/SYSTICK_DIV)) : \
        _delay_clocks (clocks/SYSTICK_DIV))

/**
 * Convert time in seconds to CPU clocks.
 * WARNING: This function will return zero for non-const arguments!
 * This is because it otherwise will result in bloated floating-point math code.
 */
#define sysclk_t2c(secs) \
	(__builtin_constant_p (secs) ? \
	    (__builtin_constant_p (SYSCLK_FREQ) ? \
	        ((uint32_t)(SYSCLK_FREQ * secs)) : \
	        umul_h32 (SYSCLK_FREQ, (uint32_t)(secs * 4294967296.0))\
	    ) : \
	    0 \
	)

/**
 * Delay execution for given amount of time in seconds.
 * You can use exponential notation to conveniently declare
 * time delays in microseconds, e.g. 4.7E-6 for 4.7us.
 */
#define delay(secs) \
    delay_clocks (sysclk_t2c (secs))

#endif // _STM32_SYSTICK_H
