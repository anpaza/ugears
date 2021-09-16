/*
    STM32 SPI library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_SPI_H
#define _STM32_SPI_H

/**
 * @file spi.h
 *      A simple library to work with the SPI peripherial.
 *
 * The following macros are expected to be defined in your HARDWARE_H
 * in order to deal with SPI peripherial:
 *
 * @li {HWFN}_SPI_NUM defines the number of the SPI peripherial to use
 *      by hardware feature HWFN.
 * @li {HWFN}_SPI_CR1 defines the setup of the CR1 of the SPI peripherial
 *      for hardware feature HWFN. This is a combination of SPI_CR1_XXX flags,
 *      not including the BR (bitrate) flags.
 * @li {HWFN}_SPI_CR2 defines the setup of the CR2 of the SPI peripherial
 *      for hardware feature HWFN. This is a combination of SPI_CR2_XXX flags.
 * @li {HWFN}_SPI_FREQ defines the maximum desired SPI operational frequency.
 *      As STM32 has a limited number of frequency divisors, the actual
 *      frequency used may be less than requested, but never larger.
 *
 * Example:
 * @code
 * // SBUS1 is SPI3
 * #define SBUS1_SPI_NUM		3
 * // Master mode, CPOL=0 and CPHA=0
 * #define SBUS1_SPI_CR1		SPI_CR1_MSTR
 * #define SBUS1_SPI_CR2		SPI_CR2_SSOE
 * // up to 10MHz SPI clock
 * #define SBUS1_SPI_FREQ		10000000
 * @endcode
 */

#include "cmsis.h"
#include <useful/useful.h>

/// 000: fPCLK/2
#define SPI_CR1_CLK_2		(0)
/// 001: fPCLK/4
#define SPI_CR1_CLK_4		(SPI_CR1_BR_0)
/// 010: fPCLK/8
#define SPI_CR1_CLK_8		(SPI_CR1_BR_1)
/// 011: fPCLK/16
#define SPI_CR1_CLK_16		(SPI_CR1_BR_1 | SPI_CR1_BR_0)
/// 100: fPCLK/32
#define SPI_CR1_CLK_32		(SPI_CR1_BR_2)
/// 101: fPCLK/64
#define SPI_CR1_CLK_64		(SPI_CR1_BR_2 | SPI_CR1_BR_0)
/// 110: fPCLK/128
#define SPI_CR1_CLK_128		(SPI_CR1_BR_2 | SPI_CR1_BR_1)
/// 111: fPCLK/256
#define SPI_CR1_CLK_256		(SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0)

/// Return the number of SPI peripherial associated with a hw feature
#define SPI_NUM(x)		JOIN2 (x, _SPI_NUM)
/// Guess SPI clock frequency by hardware feature name
#define SPI_CLOCK_FREQ(x)	CLOCK_FREQ (JOIN2 (_SPI, SPI_NUM (x)))

/**
 * Get a pointer to the SPI peripherial associated with given feature.
 * Example: SPI(ST7567) will expand into something like SPI1
 */
#define SPI(x)			JOIN2 (SPI, SPI_NUM (x))


/**
 * Configure an SPI interface.
 * This function must be invoked with disabled SPI.
 *
 * @param spi Pointer to SPI peripherial device registers.
 * @param cr1 SPI peripherial setup (flags in CR1 register)
 * @param cr1 SPI peripherial setup (flags in CR2 register)
 * @param bus_freq The frequency on the bus SPI device is connected to
 *  (use the SPI_CLOCK_FREQ(x) to find it)
 * @param freq Maximum SPI frequency (function sets maximal possible
 *      frequency not exceeding this value)
 * @return false if SPI bus frequency cannot be set not to exceed @a freq.
 */
EXTERN_C bool spi_configure (SPI_TypeDef *spi, uint32_t cr1, uint32_t cr2,
                             uint32_t bus_freq, uint32_t freq);

/**
 * Simplified invocation of spi_configure()
 * @arg x Hardware feature name
 */
#define SPI_CONFIGURE(x) \
    spi_configure (SPI (x), JOIN2 (x, _SPI_CR1), JOIN2 (x, _SPI_CR2), \
                   CLOCK_FREQ (JOIN2 (_SPI, SPI_NUM (x))), \
                   JOIN2 (x, _SPI_FREQ))

/**
 * Enable the SPI peripherial.
 * Also sets master mode if selected in {x}_SPI_CR1.
 *
 * @param spi Pointer to SPI peripherial device registers.
 * @param cr1 SPI peripherial setup (flags in CR1 register)
 */
INLINE_ALWAYS void spi_enable (SPI_TypeDef *spi, uint32_t cr1)
{ spi->CR1 |= SPI_CR1_SPE | (cr1 & SPI_CR1_MSTR); }

/**
 * Disable the SPI peripherial.
 *
 * @param spi Pointer to SPI peripherial device registers.
 */
INLINE_ALWAYS void spi_disable (SPI_TypeDef *spi)
{ spi->CR1 &= ~SPI_CR1_SPE; }

/**
 * Enable SPI interface for a hardware feature.
 * Also sets master mode if selected in {x}_SPI_CR1.
 *
 * @arg x Hardware feature name
 */
#define SPI_ENABLE(x) \
    spi_enable (SPI (x), JOIN2 (x, _SPI_CR1))

/**
 * Disable SPI interface for a hardware feature
 *
 * @arg x Hardware feature name
 */
#define SPI_DISABLE(x) \
    spi_disable (SPI (x))

#endif // _STM32_SPI_H
