/*
    STM32 flash programming library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears/ugears.h"

#define EraseTimeout          0.01
#define ProgramTimeout        0.0001

#if defined FLASH_TYPE_1

static bool hsi_state;

bool flash_begin (uint32_t psize)
{
    (void)psize;

    // For write and erase operations on the Flash memory (write/erase), the
    // internal RC oscillator (HSI) must be ON (but not neccessarily in use).
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

    return true;
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
    if ((uintptr_t)addr >= FLASH_BANK2_ADDR)
        sr = &FLASH->SR2;
#endif

    *sr = FLASH_SR_EOP | FLASH_SR_WRPRTERR | FLASH_SR_PGERR;

    if (!flash_wait_busy (sr, erase_timeout))
        return false;

    // FLASH->CR
    sr [1] |= FLASH_CR_PER;
    // FLASH->AR
    sr [2] = (uintptr_t)addr;
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
    if ((uintptr_t)addr >= FLASH_BANK2_ADDR)
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
    if ((uintptr_t)addr >= FLASH_BANK2_ADDR)
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

#ifndef FLASH_PSIZE
#  define FLASH_PSIZE		FLASH_CR_PSIZE_x16
#endif

#ifdef FLASH_SR_RDERR
#define FLASH_SR_ALLERR (FLASH_SR_RDERR | \
    FLASH_SR_WRPERR | FLASH_SR_PGAERR | \
    FLASH_SR_PGPERR | FLASH_SR_PGSERR)
#else
#define FLASH_SR_ALLERR (\
    FLASH_SR_WRPERR | FLASH_SR_PGAERR | \
    FLASH_SR_PGPERR | FLASH_SR_PGSERR)
#endif

// Program/erase parallelism for current operation
static uint32_t g_psize = UINT32_MAX;

bool flash_begin (uint32_t psize)
{
    // psize must not exceed hardware limits
    if (psize > FLASH_PSIZE)
        return false;

    // Unlock the Flash control register
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;

    // set program/erase parallelism
    FLASH->CR = (FLASH->CR & ~FLASH_CR_PSIZE_Msk) | (psize & FLASH_CR_PSIZE_Msk);

    g_psize = psize;

    return true;
}

void flash_end ()
{
    // force error if flash_write gets called not within flash_begin/end()
    g_psize = UINT32_MAX;

    // Lock the Flash control register again
    FLASH->CR |= FLASH_CR_LOCK;
}

static bool flash_wait_busy (uint32_t clocks)
{
    uint32_t c = systick_counter ();
    for (;;)
    {
        uint32_t sr = FLASH->SR;
        if (sr & FLASH_SR_ALLERR)
            return false;

        if (!(sr & FLASH_SR_BSY))
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

// Return the sector number corresponding to address or UINT32_MAX on error
static uint32_t flash_sn (void *addr)
{
    uintptr_t address = (uintptr_t)addr;
    if ((address < STM32_FLASH_ORIGIN) ||
        (address >= STM32_FLASH_ORIGIN + STM32_FLASH_SIZE))
        return UINT32_MAX;

    unsigned sn = 0;
    address -= STM32_FLASH_ORIGIN;

#ifdef FLASH_OPTCR_DB1M
    // dual flash banks on STM32F42x & STM32F43x use identical layout
    if (address >= 1024 * 1024)
    {
        address -= 1024 * 1024;
        sn += 12;
    }
    // in dual bank mode identical layout on pages 0-7 and 12-19
    else if ((FLASH->OPTCR & FLASH_OPTCR_DB1M) &&
        (address >= 512 * 1024))
    {
        address -= 512 * 1024;
        sn += 12;
    }
#endif

    if (address >= 128 * 1024)
        sn += 5 + (address - 128 * 1024) / (128 * 1024);
    else if (address >= 64 * 1024)
        sn += 4;
    else
        sn += address / (16 * 1024);

    return sn;
}

bool flash_erase (void *addr)
{
    const uint32_t erase_timeout = sysclk_t2c (EraseTimeout);
    uint32_t sn = flash_sn (addr);
    if (sn == UINT32_MAX)
        return false;

    // clear error flags
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_ALLERR;

    if (!flash_wait_busy (erase_timeout))
        return false;

    // Set SECTOR-ERASE and sector number
    FLASH->CR = (FLASH->CR & ~FLASH_CR_SNB_Msk) |
            (FLASH_CR_SER | (sn << FLASH_CR_SNB_Pos));
    FLASH->CR |= FLASH_CR_STRT;

    bool ret = flash_wait_busy (erase_timeout);

    // Clear the SECTOR-ERASE bit
    FLASH->CR &= ~FLASH_CR_SER;

    return ret;
}

bool flash_write (void *addr, const void *src, uint32_t size)
{
    const uint32_t program_timeout = sysclk_t2c (ProgramTimeout);

    if (!flash_wait_busy (program_timeout))
        return false;

    // Set PG bit in FLASH->CR
    FLASH->CR |= FLASH_CR_PG;

#define TRANSFER(t) \
    do { \
        if (size < sizeof (t)) \
            return false; \
        *((volatile t *)addr) = *(t *)src; \
        addr = ((t *)addr) + 1; \
        src = ((t *)src) + 1; \
        size -= sizeof (t); \
    } while (0)

    while (size > 0)
        switch (g_psize)
        {
            case FLASH_CR_PSIZE_x8:  TRANSFER (uint8_t); break;
            case FLASH_CR_PSIZE_x16: TRANSFER (uint16_t); break;
            case FLASH_CR_PSIZE_x32: TRANSFER (uint32_t); break;
            case FLASH_CR_PSIZE_x64: TRANSFER (uint64_t); break;
            default: return false;
        }

#undef TRANSFER

    bool ret = flash_wait_busy (program_timeout);

    // Clear the ProgramminG bit in FLASH->CR
    FLASH->CR &= ~FLASH_CR_PG;

    return ret;
}

bool flash_write16 (void *addr, uint16_t data)
{
    const uint32_t program_timeout = sysclk_t2c (ProgramTimeout);

    // clear error flags
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_ALLERR;

    if (!flash_wait_busy (program_timeout))
        return false;

    // Set PG bit in FLASH->CR
    FLASH->CR |= FLASH_CR_PG;

#define TRANSFER(t, i, d) do { ((volatile t *)addr) [i] = d; } while (0)

    switch (g_psize)
    {
        case FLASH_CR_PSIZE_x8:
            TRANSFER (uint8_t, 0, data);
            TRANSFER (uint8_t, 1, data >> 8);
            break;

        case FLASH_CR_PSIZE_x16:
            TRANSFER (uint16_t, 0, data);
            break;

        case FLASH_CR_PSIZE_x32:
        case FLASH_CR_PSIZE_x64:
        default:
            return false;
    }

#undef TRANSFER

    bool rc = flash_wait_busy (program_timeout);

    // Clear the ProgramminG bit in FLASH->CR
    FLASH->CR &= ~FLASH_CR_PG;

    return rc;
}

#endif
