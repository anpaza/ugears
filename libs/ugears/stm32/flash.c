/*
    STM32 flash programming library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears.h"

#define EraseTimeout          0.01
#define ProgramTimeout        0.0001

#ifdef FLASH_TYPE_1

static bool hsi_state;

void flash_begin ()
{
    hsi_state = clock_HSI_enabled ();
    if (!hsi_state)
        clock_HSI_start ();

    // Authorize the FPEC of Bank1 Access
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;

#ifdef STM32F10X_XL
    // Authorize the FPEC of Bank2 Access
    FLASH->KEYR2 = FLASH_FKEY1;
    FLASH->KEYR2 = FLASH_FKEY2;
#endif
}

void flash_end ()
{
    // Set the Lock Bit to lock the FPEC and the CR of  Bank1
    FLASH->CR |= FLASH_CR_LOCK;

#ifdef STM32F10X_XL
    // Set the Lock Bit to lock the FPEC and the CR of  Bank2
    FLASH->CR2 |= FLASH_CR_LOCK;
#endif

    if (!hsi_state)
        clock_HSI_stop ();
}

static bool flash_wait_busy (volatile uint32_t *sr, uint32_t clocks)
{
    uint32_t c = systick_counter ();
    for (;;)
    {
        uint32_t srval = *sr;
        if (srval & (FLASH_SR_WRPRTERR | FLASH_SR_PGERR))
            return false;

        if (!(srval & FLASH_SR_BSY))
            return true;

        uint32_t nc = systick_counter ();
        uint32_t pc = c;
        c -= nc;
        if ((int32_t)(c ^ pc) < 0)
            c += systick_reload ();

        clocks -= c;
        if (clocks & 0x80000000)
            return false;

        c = nc;
    }
}

bool flash_erase (void *addr)
{
    const uint32_t erase_timeout = sysclk_t2c (EraseTimeout);
    volatile uint32_t *sr = &FLASH->SR;

#ifdef STM32F10X_XL
    if ((uint32_t)addr >= FLASH_BANK2_ADDR)
        sr = &FLASH->SR2;
#endif

    *sr = FLASH_SR_EOP | FLASH_SR_WRPRTERR | FLASH_SR_PGERR;

    if (!flash_wait_busy (sr, erase_timeout))
        return false;

    // FLASH->CR
    sr [1] |= FLASH_CR_PER;
    // FLASH->AR
    sr [2] = (uint32_t)addr;
    // FLASH->CR
    sr [1] |= FLASH_CR_STRT;

    bool ret = flash_wait_busy (sr, erase_timeout);

    // Clear the PAGE-ERASE bit
    sr [1] &= ~FLASH_CR_PER;

    return ret;
}

bool flash_write16 (void *addr, uint16_t data)
{
    const uint32_t program_timeout = sysclk_t2c (ProgramTimeout);
    volatile uint32_t *sr = &FLASH->SR;

#ifdef STM32F10X_XL
    if ((uint32_t)addr >= FLASH_BANK2_ADDR)
        sr = &FLASH->SR2;
#endif

    *sr = FLASH_SR_EOP | FLASH_SR_WRPRTERR | FLASH_SR_PGERR;

    if (!flash_wait_busy (sr, program_timeout))
        return false;

    // Set PG bit in FLASH->CR
    sr [1] |= FLASH_CR_PG;

    *((volatile uint16_t *)addr) = data;
    bool rc = flash_wait_busy (sr, program_timeout);

    // Clear the ProgramminG bit in FLASH->CR
    sr [1] &= ~FLASH_CR_PG;

    return rc;
}

bool flash_write (void *addr, const void *src, uint32_t size)
{
    const uint32_t program_timeout = sysclk_t2c (ProgramTimeout);
    volatile uint32_t *sr = &FLASH->SR;

#ifdef STM32F10X_XL
    if ((uint32_t)addr >= FLASH_BANK2_ADDR)
        sr = &FLASH->SR2;
#endif

    if (!flash_wait_busy (sr, program_timeout))
        return false;

    // Set PG bit in FLASH->CR
    sr [1] |= FLASH_CR_PG;

    while (size > 1)
    {
        *((volatile uint16_t *)addr) = *(uint16_t *)src;
        addr = ((uint16_t *)addr) + 1;
        src = ((uint16_t *)src) + 1;
        size -= 2;
    }

    bool ret = flash_wait_busy (sr, program_timeout);

    // Clear the ProgramminG bit in FLASH->CR
    sr [1] &= ~FLASH_CR_PG;

    return ret;
}

#elif defined FLASH_TYPE_2

#endif
