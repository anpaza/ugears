/*
    STM32 low-level SPI library
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <ugears/ugears.h>

bool spi_configure (SPI_TypeDef *spi, uint32_t cr1, uint32_t bus_freq, uint32_t freq)
{
    // Available clock divisors from 2 to 256 in powers of two
    uint32_t cd, bf;
    for (cd = 2, bf = bus_freq / 2; cd <= 256; cd++, bf >>= 1)
        if (bf <= freq)
            break;
    cd = (cd - 2) << SPI_CR1_BR_Pos;
    if (cd > SPI_CR1_BR_Msk)
        return false;

    spi->CR1 = (cr1 & ~(SPI_CR1_BR_Msk | SPI_CR1_SPE_Msk | SPI_CR1_MSTR_Msk)) | cd;
    spi->CR2 = 0;

    return true;
}
