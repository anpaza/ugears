/*
    STM32F0x system clocks management
    Copyright (C) 2015 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_CLOCKS_STM32F0_H
#define _STM32_CLOCKS_STM32F0_H

#include HARDWARE_H
#include <useful/useful.h>

/**
 * @file clocks-stm32f0.h
 *      This file contains the definitions and functions for setting up
 *      initial MCU clock setup and, optionally, providing functions
 *      for clock manipulations at runtime.
 *
 *      This file is typically included from HARDWARE_H.
 *
 * The following macros may be defined prior to including this file.
 * Most of them have reasonable defaults, so you may get started by
 * using even an almost-empty HARDWARE_H. Then, step by step, refine
 * your configuration to approach your ideal.
 *
 * @li SYSCLK_SOURCE - one of HSI,HSE,PLL - the source for
 *      main system clock. Default is HSI.
 * @li HSI_VALUE - the frequency of the internal RC oscillator.
 *      Default is 8000000.
 * @li HSE_VALUE - the frequency of the crystal resonator.
 *      There's no default.
 * @li CLOCK_DYNAMIC - if this macro is defined, the functions
 *      for changing the clocks dynamically at runtime will be
 *      made public. This also means that the macros for clock
 *      frequencies will be variables, not constants. This adds
 *      about 200 bytes of code to your program.
 * @li HCLK_DIV - one of 1,2,4,8,16,64,128,256,512 - the divider
 *      for clock of the AHB bus (HCLK = SYSCLK/DIV).
 *      Default value is 1.
 * @li PCLK_DIV - one of 1,2,4,8,16 - the divider for clock
 *      of the APB1,APB2 buses (PCLK = HCLK/DIV).
 *      Default value is 1.
 * @li PLL_DIV_AFTER - define this if the PLL divider is applied
 *      after PLL_SOURCE selection. Check the datasheet of your
 *      device if you're not sure (look for "Clock Tree" figure
 *      corresponding to your device). This should be defined
 *      at least for STM32F070x6,B, STM32F030xC, STM32F04x,
 *      STM32F071,72,78 and STM32F091,98.
 * @li PLL_SOURCE - one of HSI,HSE - the source for
 *      Phase-Locked-Loop clock generator. HSI selects HSI/2
 *      as the source for PLL and HSE selects HSE/PLL_DIV
 *      as the PLL source (if PLL_DIV_AFTER is not defined)
 *      and just HSI or HSE if PLL_DIV_AFTER is defined
 *      (but the selected clock will be divided by PLL_DIV before
 *      entering the PLL).
 *      Default value is HSI.
 * @li PLL_DIV - 1,2,3,...,16 - on some devices this define the
 *      divider for the HSE clock, and on some for both HSE and HSI
 *      clocks on entry to PLL.
 *      Default value is 1.
 * @li PLL_MUL - 2,3,4,...,16 - the clock multiplicator in
 *      the Phase-Locked-Loop circuit.
 *      Default value is 6.
 *
 * Additionaly, you may ask the init code to select the clock
 * for USART, I2C and other peripherials:
 *
 * @li CLOCK_USB - one of HSI48,PLL
 * @li CLOCK_CEC - one of HSI,LSE
 * @li CLOCK_I2C1 - one of HSI,SYSCLK
 * @li CLOCK_USART1 - one of PCLK,SYSCLK,LSE,HSI
 * @li CLOCK_USART2 - one of PCLK,SYSCLK,LSE,HSI
 *
 * This header file defines the following useful macros that
 * can be used in user code:
 *
 * @li SYSCLK_FREQ - system clock frequency. This is a constant
 *      value, unless CLOCK_DYNAMIC is defined, in which case
 *      it becomes a global variable.
 * @li HCLK_FREQ - AHB bus frequency. This is a constant
 *      value, unless CLOCK_DYNAMIC is defined, in which case
 *      it becomes a global variable.
 * @li PCLK_FREQ - the frequency of APB1 & APB2 buses. This is
 *      a constant value, unless CLOCK_DYNAMIC is defined,
 *      in which case it becomes a global variable.
 */

// clock sources
#define CLKSRC_HSI    1
#define CLKSRC_HSE    2
#define CLKSRC_PLL    3

/// n is one of 1,2,4,8,16,64,128,256,512
#define HCLK_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_HPRE_DIV, n)
/// n is one of 1,2,4,8,16
#define PCLK_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_PPRE_DIV, n)

#ifndef HSE_STARTUP_TIMEOUT
/// Time out for HSE start up
#define HSE_STARTUP_TIMEOUT		0xA000
#endif
#ifndef HSI_STARTUP_TIMEOUT
/// Time out for HSI start up
#define HSI_STARTUP_TIMEOUT		0xA000
#endif
#ifndef PLL_STARTUP_TIMEOUT
/// PLL takes more time to start up
#define PLL_STARTUP_TIMEOUT		0x14000
#endif

// default values

#ifndef HSI_VALUE
#  define HSI_VALUE                     8000000
#endif

#ifndef LSI_VALUE
#  define LSI_VALUE                     40000
#endif

#ifndef LSE_VALUE
#  define LSE_VALUE                     32768
#endif

#ifndef SYSCLK_SOURCE
#  define SYSCLK_SOURCE HSI
#endif

#ifndef HCLK_DIV
#  define HCLK_DIV 1
#endif

#ifndef PCLK_DIV
#  define PCLK_DIV 1
#endif

#if JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_PLL
#  ifndef PLL_SOURCE
#    define PLL_SOURCE			HSI
#  endif
#  ifndef PLL_DIV
#    define PLL_DIV			1
#  endif
#  ifndef PLL_MUL
#    define PLL_MUL			6
#  endif
#endif

#ifndef CLOCK_DYNAMIC

// Compute the frequency of SYSCLK
#if JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSI
#  define SYSCLK_FREQ			HSI_VALUE

#elif JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSE
#  define SYSCLK_FREQ			HSE_VALUE

#elif JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_PLL
#  if defined PLL_DIV_AFTER
// Newer devices apply the divider AFTER choosing the input PLL clock
#    define SYSCLK_FREQ			((JOIN2 (PLL_SOURCE,_VALUE) / PLL_DIV) * PLL_MUL)
// Older devices use either HSI/2 or HSE/PLL_DIV
#  elif JOIN2 (CLKSRC_,PLL_SOURCE) == CLKSRC_HSI
#    define SYSCLK_FREQ		((HSI_VALUE / 2) * PLL_MUL)
#  elif JOIN2 (CLKSRC_,PLL_SOURCE) == CLKSRC_HSE
#    define SYSCLK_FREQ		((HSE_VALUE / PLL_DIV) * PLL_MUL)
#  endif

#else
#  if !defined __MAKEDEP__
#    error "SYSCLK_SOURCE has an invalid value!"
#  endif
#endif

#define HCLK_FREQ			(SYSCLK_FREQ/HCLK_DIV)
#define PCLK_FREQ			(HCLK_FREQ/PCLK_DIV)

#if SYSCLK_FREQ > 24000000
#  error "System clock frequency should not exceed 24MHz!"
#endif

#else // CLOCK_DYNAMIC

extern uint32_t SYSCLK_FREQ, HCLK_FREQ, PCLK_FREQ;

/**
 * Reset all clocks to default state
 */
extern void clock_reset ();

/**
 * Setup the main system clock to use HSI
 * @return
 *   0 on success, non-zero if failed to start the oscillator
 */
extern uint8_t sysclk_HSI ();

/**
 * Setup the main system clock to use HSE
 * @return
 *   0 on success, non-zero if failed to start the oscillator
 */
extern uint8_t sysclk_HSE ();

/**
 * Setup the main system clock to use HSE
 * @arg clksrc
 *   The source clock for PLL (CLKSRC_HSI or CLKSRC_HSE)
 * @arg plldiv
 *   PLL clock divider (see the comment on the PLL_DIV macro)
 * @arg pllmul
 *   PLL frequency multiply factor (see the comment on the PLL_MUL macro)
 * @return
 *   0 if PLL has been set up, non-zero on failure
 */
extern uint8_t sysclk_PLL (uint8_t clksrc, uint32_t plldiv, uint32_t pllmul);

/**
 * @arg hclk_div_flags
 *   HCLK divider flags (you may use the HCLK_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
extern uint8_t clock_AHB (uint32_t hclk_div_flags);

/**
 * @arg pclk_div_flags
 *   PCLK divider flags (you may use the PCLK_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
extern uint8_t clock_APB (uint32_t pclk_div_flags);

#endif // CLOCK_DYNAMIC

// Для кросс-микроконтроллерного программирования
#define PCLK1_FREQ	PCLK_FREQ

/**
 * Start the HSI clock. This is used by flash interface, which needs HSI.
 */
extern uint8_t clock_HSI_start ();

/**
 * Stop the HSI clock unconditionally. Take care not to stop the system clock!
 * The best practice is to save HSI clock state before starting modifying it.
 */
extern void clock_HSI_stop ();

/**
 * Query HSI clock state
 * @return
 *     true if HSI clock is enabled
 */
INLINE_ALWAYS bool clock_HSI_enabled ()
{ return !!(RCC->CR & RCC_CR_HSION); }

#endif // _STM32_CLOCKS_STM32F0_H
