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
 */

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

/**
 * Get a pointer to the SPI peripherial associated with given feature.
 * Example: SPI(ST7567) will expand into something like SPI1
 */
#define SPI(x)			JOIN2 (SPI, SPI_NUM (x))


#endif // _STM32_SPI_H
