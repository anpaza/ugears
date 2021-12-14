/*
    System time functions
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _TIME_H
#define _TIME_H

#include "useful.h"
#include "fpmath.h"

/**
 * @file time.h System clock functionality.
 *
 * This file provides a global clock variable that's incremented
 * CLOCKS_PER_SEC times every second. This variable is used in time-dependent
 * functions (e.g. one-shot timers).
 *
 * It is expected that user code sets up a system tick interrupt that will
 * be triggered CLOCKS_PER_SEC times per second and interrupt handler will
 * increment the clock variable. A simple example using the SysTick timer:
 *
 * @verbatim
 * extern "C" void SysTick_Handler () { clock++; }
 *
 * ... in initialization
 *
 * systick_config (sysclk_t2c (1.0 / CLOCKS_PER_SEC));
 * systick_int_enable (0);
 * @endverbatim
 */

/** System clock type */
typedef uint32_t clock_t;

/** Maximal value of a clock_t variable */
#define CLOCK_MAX UINT32_MAX

/** The system clock variable */
EXTERN_C volatile clock_t clock;

/* CLOCKS_PER_SEC is expected to be defined in hardware.h
 * Value should not exceed 1000!
 */
#ifndef CLOCKS_PER_SEC
#  define CLOCKS_PER_SEC	32
#endif

/** Milliseconds to clocks */
#define MSEC2CLK(s) \
    (__builtin_constant_p (s) ? \
        ((uint32_t)(s * (CLOCKS_PER_SEC / 1000.0))) : \
        umul_h32 (s, (uint32_t) (4294967296.0 * CLOCKS_PER_SEC / 1000.0)) \
    )

/** Clocks to milliseconds - rounded */
#define CLK2MSEC(c) \
    ((c * 1000 + CLOCKS_PER_SEC/2) / CLOCKS_PER_SEC)

#endif // _TIME_H
