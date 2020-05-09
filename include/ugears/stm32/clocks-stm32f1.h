/*
    STM32F1x system clocks management
    Copyright (C) 2015 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _CLOCKS_STM32F1_H
#define _CLOCKS_STM32F1_H

#include "useful.h"

/**
 * The following macros may be defined prior to including
 * this file (usually from hardware*.h):
 *
 * @li HSI_VALUE - the frequency of the internal RC oscillator.
 * @li HSE_VALUE - the frequency of the crystal resonator.
 * @li CLOCK_DYNAMIC - if this macro is defined, the functions
 *      for changing the clocks dynamically at runtime will be
 *      made public. This also means that the macros for clock
 *      frequencies will be variables, not constants. This adds
 *      about 200 bytes of code to your program.
 * @li SYSCLK_SOURCE - one of HSI,HSE,PLL - the source for
 *      main system clock.
 * @li HCLK_DIV - one of 1,2,4,8,16,64,128,256,512 - the divider
 *      for clock of the AHB bus (HCLK = SYSCLK/DIV)
 * @li PCLK1_DIV - one of 1,2,4,8,16 - the divider for clock
 *      of the APB1 bus (PCLK1 = HCLK/DIV). APB1 bus frequency
 *      should not exceed 36MHz.
 * @li PCLK2_DIV - one of 1,2,4,8,16 - the divider for clock
 *      of the APB2 bus (PCLK2 = HCLK/DIV)
 * @li PLL_SOURCE - one of HSI,HSE,PLL2 - the source for
 *      Phase-Locked-Loop clock generator. HSI selects HSI/2
 *      as the source for PLL and HSE selects HSE/PLL_DIV
 *      as the PLL source. Also, on some devices you may
 *      choose the PLL2 output to be input to PLL.
 * @li PLL_DIV - 1,2,3..16 - the divider for the HSE clock
 *      before entering the PLL. This has no effect on HSI.
 * @li PLL_MUL - 2,3,4,...,16 - the clock multiplicator in
 *      the Phase-Locked-Loop circuit.
 * @li PLL23_DIV - 1,2,3...16 - the divider applied on HSE
 *      before feeding HSE on input to PLL2 and PLL3.
 *      Not all devices support this feature, consult the manual.
 * @li PLL2_MUL - 8,9..14,16,20 - the multiplier applied on
 *      input frequence in the PLL2 circuit.
 *      Not all devices support this feature, consult the manual.
 * @li PLL3_MUL - 8,9..14,16,20 - the multiplier applied on
 *      input frequence in the PLL3 circuit.
 *      Not all devices support this feature, consult the manual.
 *
 * Additionaly, you may ask the init code to select the clock
 * for USART, I2C and other peripherials:
 *
 * @li CLOCK_USB_DIV - one of 1,1_5 (USB clock = PLL/CLOCK_USB_DIV)
 * @li CLOCK_ADC_DIV - one of 2,4,6,8 (ADC clock = PCLK2/CLOCK_ADC_DIV)
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
 * @li PCLK1_FREQ - the frequency of the APB1 bus. This is
 *      a constant value, unless CLOCK_DYNAMIC is defined,
 *      in which case it becomes a global variable.
 * @li PCLK2_FREQ - the frequency of the APB2 bus. This is
 *      a constant value, unless CLOCK_DYNAMIC is defined,
 *      in which case it becomes a global variable.
 */

// clock sources
#define CLKSRC_HSI    1
#define CLKSRC_HSE    2
#define CLKSRC_PLL    3
#define CLKSRC_PLL2   4

/// n is one of 1,2,4,8,16,64,128,256,512
#define HCLK_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_HPRE_DIV, n)
/// n is one of 1,2,4,8,16
#define PCLK1_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_PPRE1_DIV, n)
/// n is one of 1,2,4,8,16
#define PCLK2_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_PPRE2_DIV, n)

#ifndef HSE_STARTUP_TIMEOUT
/// Time out for HSE start up
#define HSE_STARTUP_TIMEOUT		0x5000
#endif
#ifndef HSI_STARTUP_TIMEOUT
/// Time out for HSI start up
#define HSI_STARTUP_TIMEOUT		0x5000
#endif
#ifndef PLL_STARTUP_TIMEOUT
/// PLL takes more time to start up
#define PLL_STARTUP_TIMEOUT		0xA000
#endif

#if defined RCC_CFGR_OTGFSPRE || defined RCC_CFGR_USBPRE
// USB prescaler can be either 1.5 (@ 72MHz PLL) or 1 (@ 48 MHz PLL)
#define RCC_CFGR_USBPRE_1_5		0
#if defined RCC_CFGR_OTGFSPRE
#  define RCC_CFGR_USBPRE_1		RCC_CFGR_OTGFSPRE
#elif defined RCC_CFGR_USBPRE
#  define RCC_CFGR_USBPRE_1		RCC_CFGR_USBPRE
#endif
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

#ifndef PCLK1_DIV
#  define PCLK1_DIV 1
#endif

#ifndef PCLK2_DIV
#  define PCLK2_DIV 1
#endif

#ifndef CLOCK_ADC_DIV
#  define CLOCK_ADC_DIV 2
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

#if defined PLL23_DIV || defined PLL2_MUL || defined PLL3_MUL || \
    JOIN2 (CLKSRC_,PLL_SOURCE) == CLKSRC_PLL2
#  if !defined RCC_CFGR2_PREDIV2
#    error "PLL2 & 3 are not supported on this device"
#  endif
#  ifndef PLL23_DIV
#    define PLL23_DIV			1
#  endif
#  if JOIN2 (CLKSRC_,PLL_SOURCE) == CLKSRC_PLL2 && !defined PLL2_MUL
#    define PLL2_MUL			8
#  endif
#endif

#ifndef CLOCK_DYNAMIC

// Compute the frequency of SYSCLK
#if JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSI
#  define SYSCLK_FREQ			HSI_VALUE

#elif JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSE
#  define SYSCLK_FREQ			HSE_VALUE

#elif JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_PLL
#  if JOIN2 (CLKSRC_,PLL_SOURCE) == CLKSRC_HSI
#    define SYSCLK_FREQ			((HSI_VALUE / 2) * PLL_MUL)
#    if SYSCLK_FREQ > 36000000
#      error "Maximum frequency that can be achieved with HSI is 36MHz!"
#    endif
#  elif JOIN2 (CLKSRC_,PLL_SOURCE) == CLKSRC_HSE
#    define SYSCLK_FREQ			((HSE_VALUE / PLL_DIV) * PLL_MUL)
#  elif JOIN2 (CLKSRC_,PLL_SOURCE) == CLKSRC_PLL2
#    define SYSCLK_FREQ \
       ((((HSE_VALUE / PLL23_DIV) * PLL2_MUL) / PLL_DIV) * PLL_MUL)
#  endif

#else
#  if !defined __MAKEDEP__
#    error "SYSCLK_SOURCE has an invalid value!"
#  endif
#endif

#define HCLK_FREQ			(SYSCLK_FREQ/HCLK_DIV)
#define PCLK1_FREQ			(HCLK_FREQ/PCLK1_DIV)
#define PCLK2_FREQ			(HCLK_FREQ/PCLK2_DIV)

#if SYSCLK_FREQ > 72000000
#  error "System clock frequency should not exceed 72MHz!"
#endif
#if PCLK1_FREQ > 36000000
#  error "APB1 bus frequency should not exceed 36MHz!"
#endif

#else // CLOCK_DYNAMIC

extern uint32_t SYSCLK_FREQ, HCLK_FREQ, PCLK1_FREQ, PCLK2_FREQ;

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
 * Setup the main system clock to use PLL
 * @arg clksrc
 *   The source clock for PLL (CLKSRC_HSI or CLKSRC_HSE)
 * @arg plldiv
 *   PLL clock divider (see the comment on the PLL_DIV macro)
 * @arg pllmul
 *   PLL frequency multiply factor (see the comment on the PLL_MUL macro)
 * @return
 *   0 if PLL has been set up, non-zero on failure
 */
extern uint8_t sysclk_PLL (uint8_t clksrc, uint32_t plldiv, uint32_t pllmul
#if defined RCC_CFGR2_PREDIV2
    , uint32_t pll2div, uint32_t pll2mul
#endif
    );

/**
 * @arg hclk_div_flags
 *   HCLK divider flags (you may use the HCLK_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
extern uint8_t clock_AHB (uint32_t hclk_div_flags);

/**
 * @arg pclk1_div_flags
 *   PCLK1 divider flags (you may use the PCLK1_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
extern uint8_t clock_APB1 (uint32_t pclk1_div_flags);

/**
 * @arg pclk2_div_flags
 *   PCLK2 divider flags (you may use the PCLK2_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
extern uint8_t clock_APB2 (uint32_t pclk2_div_flags);

#endif // CLOCK_DYNAMIC

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
static inline bool clock_HSI_enabled ()
{ return !!(RCC->CR & RCC_CR_HSION); }

#endif // _CLOCKS_STM32F1_H
