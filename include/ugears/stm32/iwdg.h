/*
    STM32 Independent Watchdog Timer library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_IWDG_H
#define _STM32_IWDG_H

/**
 * @file iwdg.h
 *      This library is a thin shim between the hardware and application,
 *      providing a easier way to perform the most frequent tasks.
 */

#include "cmsis.h"
#include <useful/useful.h>

#if defined STM32F0 || defined STM32F3
#  define IWDG_HAS_WINDOW
#endif

#ifndef LSI_VALUE
#  if defined STM32F || defined STM32F1 || defined STM32F3
#    define LSI_VALUE 40000
#  elif defined STM32F2 || defined STM32F4
#    define LSI_VALUE 32000
#  else
#    error "LSI_VALUE not known for current MCU!"
#  endif
#endif

/// This value, if written to IWDG->KR, starts the IWDG
#define IWDG_KEY_START		0xCCCC
/// This value, if written to IWDG->KR, allows to change IWDG counters
#define IWDG_KEY_INIT		0x5555
/// This value, when written to IWDG->KR, resets the watchdog counter
#define IWDG_KEY_RELOAD		0xAAAA

/**
 * Initialize the Independent Watchdog. You can't stop the Watchdog once
 * it is started. Still, you can call iwdg_init() multiple times to change
 * watchdog timings.
 * @arg reload_ms
 *      Independent watchdog timeout, in milliseconds. You have to call
 *      iwdg_reload() at least once per that much milliseconds, otherwise
 *      the MCU will reboot.
 * @arg window_ms
 *      Watchdog window, in milliseconds. This must be less or equal than
 *      reload_us and denotes the time interval at the end of reload_us when
 *      calling iwdg_reload() will not reboot the MCU. In other words, you
 *      can't call iwdg_reload more often than (reload_us-window_us) milliseconds.
 * @return
 *      false if requested time interval can't be achived (this means that either
 *      parameters failed sanity check, or that reload_ms is larger than maximal
 *      time period allowed, which is around 32000ms).
 */
EXTERN_C bool iwdg_init (uint32_t reload_ms
#ifdef IWDG_HAS_WINDOW
    , uint32_t window_ms
#endif
    );

/**
 * This function restarts the watchdog counter. Don't let the counter hit zero,
 * since that will cause MCU to reboot.
 */
INLINE_ALWAYS void iwdg_reload ()
{ IWDG->KR = IWDG_KEY_RELOAD; }

#endif // _STM32_IWDG_H
