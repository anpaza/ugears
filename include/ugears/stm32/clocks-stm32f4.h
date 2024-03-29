/*
    STM32F4x system clocks management
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_CLOCKS_STM32F4_H
#define _STM32_CLOCKS_STM32F4_H

#include "cmsis.h"
#include <useful/useful.h>

/**
 * @file clocks-stm32f4.h
 *      This file contains the definitions and functions for setting up
 *      initial MCU clock setup and, optionally, providing functions
 *      for clock manipulations at runtime.
 *
 * The following macros may be defined prior to including this file.
 * Most of them have reasonable defaults, so you may get started by
 * using even an almost-empty HARDWARE_H. Then, step by step, refine
 * your configuration to approach your ideal.
 *
 * @li SYSCLK_SOURCE - one of HSI,HSE,PLL - the source for
 *      main system clock. Default is HSI.
 * @li HSI_VALUE - the frequency of the internal RC oscillator.
 *      Default value is 8000000.
 * @li HSE_VALUE - the frequency of the crystal resonator.
 *      The valid range is 4MHz...26MHz. There's no default.
 * @li CLOCK_DYNAMIC - if this macro is defined, the functions
 *      for changing the clocks dynamically at runtime will be
 *      made public. This also means that the macros for clock
 *      frequencies will be variables, not constants. This adds
 *      about 800 bytes of code to your program.
 * @li HCLK_DIV - one of 1,2,4,8,16,64,128,256,512 - the divider
 *      for clock of the AHB bus (HCLK = SYSCLK/DIV)
 *      Default value is 1.
 * @li PCLK1_DIV - one of 1,2,4,8,16 - the divider for clock
 *      of the APB1 bus (PCLK1 = HCLK/DIV). APB1 bus frequency
 *      should not exceed 36MHz.
 *      Default value is 1.
 * @li PCLK2_DIV - one of 1,2,4,8,16 - the divider for clock
 *      of the APB2 bus (PCLK2 = HCLK/DIV)
 *      Default value is 1.
 * @li PLL_SOURCE - one of HSI,HSE - the source for
 *      Phase-Locked-Loop clock generator. Both are divided
 *      by PLL_M before being feed to PLL input. The PLL
 *      input must be in range 1..2MHz. Default is HSI.
 * @li PLL_M - 2..63 - the divider for the HSI or HSE clock
 *      before entering the PLL.
 * @li PLL_N - 50...432 - the clock multiplicator in the
 *      Phase-Locked-Loop circuit. You must ensure the output
 *      of PLL (before dividing by PLL_P) is >= 100MHz.
 * @li PLL_P - 2,4,6,8 - the PLL output clock divider before
 *      entering the system clock multiplexor.
 * @li PLL_Q - 2..15 - the PLL output clock divider before
 *      feeding to USB OTG FS clock, the random number generator
 *      clock and the SDIO clock.
 * @li PLL_VOS - Voltage Scaling for use in PLL mode. This chooses
 *     one or two or three power profiles (depending on MCU).
 *     VOS 0 supports max 168 MHz HCLK, 1 is max 144 MHz, 2 is max 120 MHz.
 * @li FLASH_WS - 0..15 - flash memory wait states. You must
 *      choose wait states carefully, according to section
 *      "3.5.1 Relation between CPU clock frequency and Flash memory read time"
 *      in STM32F4 datasheet.
 * @li FLASH_PREFETCH - 0/1 - enable flash memory prefetch buffer.
 *      You must disable prefetch if supply voltage < 2.1V.
 * @li FLASH_ICACHE - 0/1 - enable instruction cache
 * @li FLASH_DCACHE - 0/1 - enable data cache
 * @li VECT_TAB_SRAM - define this if you need to relocate your
 *      vector Table in Internal SRAM. Default is FLASH memory.
 * @li VECT_TAB_OFFSET - you can define this to a multiple of 0x200
 *      if you need to relocate vector table to an address different
 *      from the default 0.
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
 * @li PLL48CK_FREQ - the frequency on the Q output of PLL,
 *      connected to USB peripherial. Should be 48000000.
 */

// clock sources
#define CLKSRC_HSI			1
#define CLKSRC_HSE			2
#define CLKSRC_PLL			3

/// n is one of 1,2,4,8,16,64,128,256,512
#define HCLK_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_HPRE_DIV, n)
/// n is one of 1,2,4,8,16
#define PCLK1_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_PPRE1_DIV, n)
/// n is one of 1,2,4,8,16
#define PCLK2_DIV_FLAGS(n)		JOIN2 (RCC_CFGR_PPRE2_DIV, n)

#ifndef HSE_STARTUP_TIMEOUT
/// Time out for HSE start up (loop count)
#define HSE_STARTUP_TIMEOUT		0xA000
#endif
#ifndef HSI_STARTUP_TIMEOUT
/// Time out for HSI start up (loop count)
#define HSI_STARTUP_TIMEOUT		0xA000
#endif
#ifndef PLL_STARTUP_TIMEOUT
/// Time out for PLL start up (loop count)
#define PLL_STARTUP_TIMEOUT		0xA000
#endif

/// The name of the clock on the AHB bus
#define BUS_CLOCK_AHB			HCLK
/// The name of the clock on the APB1 bus
#define BUS_CLOCK_APB1			PCLK1
/// The name of the clock on the APB2 bus
#define BUS_CLOCK_APB2			PCLK2

// default values

#ifndef HSI_VALUE
#  define HSI_VALUE                     16000000
#endif

#ifndef LSI_VALUE
#  define LSI_VALUE                     32000
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

#ifndef FLASH_WS
#  define FLASH_WS 0
#endif

#ifndef FLASH_PREFETCH
#  define FLASH_PREFETCH 1
#endif

#ifndef FLASH_ICACHE
#  define FLASH_ICACHE 1
#endif

#ifndef FLASH_DCACHE
#  define FLASH_DCACHE 1
#endif

#if defined STM32F427_437xx || defined STM32F429_439xx
#  define SYSCLK_FREQ_MAX		180000000
#else
#  define SYSCLK_FREQ_MAX		168000000
#endif

#if JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_PLL
#  ifndef PLL_SOURCE
#    define PLL_SOURCE			HSI
#  endif
#  if !defined (PLL_M) || !defined (PLL_N) || !defined (PLL_P) || !defined (PLL_Q)
#    error "You must define the most important PLL multipliers and dividers!"
#  endif
#endif

#ifndef PLL_VOS
#  define PLL_VOS                       0
#endif

#ifndef CLOCK_DYNAMIC

// Compute the frequency of SYSCLK
#if JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSI
#  define SYSCLK_FREQ			HSI_VALUE
#  define PLL48CK_FREQ			0

#elif JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSE
#  define SYSCLK_FREQ			HSE_VALUE
#  define PLL48CK_FREQ			0

#elif JOIN2 (CLKSRC_,SYSCLK_SOURCE) == CLKSRC_PLL
#  define VCO_FREQ(mul,div)		((mul * JOIN2 (PLL_SOURCE,_VALUE)) / (div * PLL_M))
#  if (VCO_FREQ(1, 1) < 1000000) || (VCO_FREQ(1, 1) > 2000000)
#    error "VCO input frequency must be from 1 to 2 MHz!"
#  endif
#  if (VCO_FREQ(PLL_N, 1) < 100000000)
#    error "VCO output frequency must not be less than 100MHz!"
#  endif
#  define SYSCLK_FREQ			(VCO_FREQ (PLL_N, PLL_P))
#  define PLL48CK_FREQ			(VCO_FREQ (PLL_N, PLL_Q))

#else
#  if !defined __MAKEDEP__
#    error "SYSCLK_SOURCE has an invalid value!"
#  endif
#endif

#define HCLK_FREQ			(SYSCLK_FREQ / HCLK_DIV)
#define PCLK1_FREQ			(HCLK_FREQ / PCLK1_DIV)
#define PCLK2_FREQ			(HCLK_FREQ / PCLK2_DIV)

#if SYSCLK_FREQ > SYSCLK_FREQ_MAX
#  error "System clock frequency is too high!"
#endif

#else // CLOCK_DYNAMIC

EXTERN_C uint32_t SYSCLK_FREQ, HCLK_FREQ, PCLK1_FREQ, PCLK2_FREQ, PLL48CK_FREQ;

/**
 * Reset all clocks to default state
 */
EXTERN_C void clock_reset ();

/**
 * Setup the main system clock to use HSI.
 * This does NOT stop other clocks, as they may be used independently.
 * @return
 *   0 on success, non-zero if failed to start the oscillator
 */
EXTERN_C uint8_t sysclk_HSI ();

/**
 * Setup the main system clock to use HSE.
 * This does NOT stop other clocks, as they may be used independently.
 * @return
 *   0 on success, non-zero if failed to start the oscillator
 */
EXTERN_C uint8_t sysclk_HSE ();

/**
 * Setup the main system clock to use main PLL.
 * You must set up the PLL before with clock_PLL_setup().
 * This does NOT stop other clocks, as they may be used independently.
 */
EXTERN_C uint8_t sysclk_PLL ();

/**
 * Setup the main PLL.
 * @arg clksrc
 *   The source clock for PLL (CLKSRC_HSI or CLKSRC_HSE)
 * @arg pllm
 *   range 2..63 - the divider for the HSI or HSE clock
 *   before entering the PLL.
 * @arg plln
 *   range 50...432 - the clock multiplicator in the
 *   Phase-Locked-Loop circuit. You must ensure the output
 *   of PLL (before dividing by PLL_P) is >= 100MHz.
 * @arg pllp
 *   one of 2,4,6,8 - the PLL output clock divider before
 *   entering the system clock multiplexor.
 * @arg pllq
 *   range 2..15 - the PLL output clock divider before
 *   feeding to USB OTG FS clock, the random number generator
 *   clock and the SDIO clock.
 * @arg vos
 *   Voltage Scaling (PLL_VOS). Use 0..1 or 0..2 here depending on which
 *   voltage settings your MCU supports. Feeding an out-of-value range
 *   will return an error code. For MCUs supporting two VOS modes
 *   0 is max HCLK 168MHz, 1 is max 144 MHz. For MCUs supporting
 *   three VOS modes 0..2 is max 168, 144, 120 MHz.
 * @return
 *   0 if PLL has been set up, non-zero on failure
 */
EXTERN_C uint8_t clock_PLL_setup (uint8_t clksrc, uint32_t pllm, uint32_t plln,
                                  uint32_t pllp, uint32_t pllq, uint32_t vos);

/**
 * Set the AHB bus clock. When using Ethernet, AHB clock must
 * exceed 25MHz.
 * @arg hclk_div_flags
 *   HCLK divider flags (RCC_CFGR_HPRE_DIVXXX, you may use the
 *   HCLK_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
EXTERN_C uint8_t clock_AHB (uint32_t hclk_div_flags);

/**
 * Set the APB1 clock. Make sure APB1 clock does not exceed 42MHz.
 * @arg pclk1_div_flags
 *   PCLK1 divider flags (RCC_CFGR_PPRE1_DIVXXX, you may use the
 *   PCLK1_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
EXTERN_C uint8_t clock_APB1 (uint32_t pclk1_div_flags);

/**
 * Set the APB2 clock. Make sure APB2 clock does not exceed 84MHz.
 * @arg pclk2_div_flags
 *   PCLK2 divider flags (RCC_CFGR_PPRE2_DIVXXX, you may use the
 *   PCLK2_DIV_FLAGS macro)
 * @return
 *   0 on success, non-zero on failure
 */
EXTERN_C uint8_t clock_APB2 (uint32_t pclk2_div_flags);

/**
 * Stop the HSE generator.
 */
INLINE_ALWAYS void clock_HSE_stop ()
{ RCC->CR &= ~RCC_CR_HSEON; }

/**
 * Stop the main PLL.
 * Make sure PLL is not the source of the system clock.
 */
EXTERN_C void clock_PLL_stop ();

/**
 * Stop the PLLs for SAI controller (PLLI2S and, if available, PLLSAI).
 */
EXTERN_C void clock_PLLI2S_stop ();

#endif // CLOCK_DYNAMIC

/**
 * Query HSI clock state
 * @return
 *     true if HSI clock is enabled
 */
INLINE_ALWAYS bool clock_HSI_enabled ()
{ return !!(RCC->CR & RCC_CR_HSION); }

/**
 * Start the HSI clock. This is used by flash interface, which needs HSI.
 */
EXTERN_C uint8_t clock_HSI_start ();

/**
 * Stop the HSI clock unconditionally. Take care not to stop the system clock!
 * The best practice is to save HSI clock state before starting modifying it.
 */
INLINE_ALWAYS void clock_HSI_stop ()
{ RCC->CR &= ~RCC_CR_HSION; }

/**
 * Set up the flash memory interface. This must be set according to
 * microcontroller voltage and frequency used, for details see section
 * "3.5.1 Relation between CPU clock frequency and Flash memory read time"
 * in microcontroller datasheet.
 * @arg ws
 *      Number of wait states (0..15).
 * @arg prefetch
 *      True to enable prefetch buffer (if VCC > 2.1V), false to disable.
 */
EXTERN_C void clock_flash_setup (uint8_t ws, bool prefetch);

/**
 * Set instruction and data cache states
 * @param icache true to enable instruction cache
 * @param dcache true to enable data cache
 */
EXTERN_C void clock_cache_setup (bool icache, bool dcache);

#endif // _STM32_CLOCKS_STM32F4_H
