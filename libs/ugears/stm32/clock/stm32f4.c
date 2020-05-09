/*
    STM32F4xx system startup code
    Copyright (C) 2015 by Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include HARDWARE_H
#include "clocks-stm32f4.h"

#ifdef CLOCK_DYNAMIC
#define DYNCLKFUN
uint32_t SYSCLK_FREQ, HCLK_FREQ, PCLK1_FREQ, PCLK2_FREQ, PLL48CK_FREQ;
#else
#define DYNCLKFUN static inline
#endif

DYNCLKFUN void clock_flash_setup (uint8_t ws, bool prefetch)
{
    uint32_t acr = FLASH->ACR & ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTEN);
    acr |= (ws & FLASH_ACR_PRFTEN);
    if (prefetch)
        acr |= FLASH_ACR_PRFTEN;
    FLASH->ACR = acr;
}

uint8_t clock_HSI_start ()
{
    // Run the HSI oscillator
    RCC->CR |= RCC_CR_HSION;

    // Wait for HSI oscillator to start up
    for (uint32_t i = 0; ; i++)
    {
        if (RCC->CR & RCC_CR_HSIRDY)
            return 0;

        if (i > HSI_STARTUP_TIMEOUT)
            return 1;
    }
}

DYNCLKFUN uint8_t clock_HSE_start ()
{
    // Run the HSE oscillator
    RCC->CR |= RCC_CR_HSEON;

    // Wait for HSI oscillator to start up
    for (uint32_t i = 0; ; i++)
    {
        if (RCC->CR & RCC_CR_HSERDY)
            return 0;

        if (i > HSE_STARTUP_TIMEOUT)
            return 1;
    }
}

DYNCLKFUN void clock_PLL_stop ()
{
    /* Turn off OTGFS, SDIO, RNG clocks before turning off PLL */
    RCC->AHB2ENR &= ~(RCC_AHB2ENR_OTGFSEN | RCC_AHB2ENR_RNGEN);
    RCC->APB2ENR &= ~RCC_APB2ENR_SDIOEN;

    /* Turn off main PLL */
    RCC->CR &= ~RCC_CR_PLLON;

    /* Reset the PLL config to boot state */
    RCC->PLLCFGR = RCC_PLLCFGR_RST_VALUE;
}

DYNCLKFUN void clock_PLLI2S_stop ()
{
#ifdef RCC_PLLI2S_SUPPORT
#ifdef RCC_APB2ENR_SAI1EN
    /* Turn off SAI clock before turning off PLLs */
    RCC->APB2ENR &= ~RCC_APB2ENR_SAI1EN;
#endif

    /* Turn off PLL2I2S */
    RCC->CR &= ~RCC_CR_PLLI2SON;
#ifdef RCC_CR_PLLSAION
    /* Turn off PLLSAI */
    RCC->CR &= ~RCC_CR_PLLSAION;
#endif

    /* Reset PLLI2S config */
    RCC->PLLI2SCFGR = RCC_PLLI2SCFGR_RST_VALUE;
#if defined RCC_PLLSAICFGR_RST_VALUE
    /* Reset PLLSAI config */
    RCC->PLLSAICFGR = RCC_PLLSAICFGR_RST_VALUE;
#endif
#endif
}

DYNCLKFUN void clock_reset ()
{
    /* Disable all interrupts from clock system */
    RCC->CIR = 0x00000000;

    /* Start HSI and wait until it starts up */
    clock_HSI_start ();

    /* Switch system clock to HSI (SW=0), clear HCLK, PCLK1, PCLK2, MCO */
    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2 |
        RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE | RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE);

    clock_PLL_stop ();
    clock_PLLI2S_stop ();

    // Set 0 wait states, don't touch prefetch
    FLASH->ACR = FLASH->ACR & ~FLASH_ACR_LATENCY;

    /* Turn off HSE, disable clock security system */
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON);

    /* Disable bypassing HSE with external oscillator */
    RCC->CR &= ~RCC_CR_HSEBYP;

    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = RCC_CIR_LSIRDYC | RCC_CIR_LSERDYC
      | RCC_CIR_HSIRDYC | RCC_CIR_HSERDYC | RCC_CIR_CSSC
      | RCC_CIR_PLLRDYC | RCC_CIR_PLLI2SRDYC
#ifdef RCC_CIR_PLLSAIRDYC
      | RCC_CIR_PLLSAIRDYC
#endif
      ;

    /* Now using 16MHz internal HSI... */
}

static void update_PCLK1_FREQ (uint32_t pclk1_div_flags)
{
    (void)pclk1_div_flags;
#ifdef CLOCK_DYNAMIC
    if ((pclk1_div_flags & RCC_CFGR_PPRE1_2) == 0)
        pclk1_div_flags = 0;
    else
        pclk1_div_flags = 1 + ((pclk1_div_flags & (RCC_CFGR_PPRE1_1 | RCC_CFGR_PPRE1_0)) >> 10);

    PCLK1_FREQ = HCLK_FREQ >> pclk1_div_flags;
#endif
}

static void update_PCLK2_FREQ (uint32_t pclk2_div_flags)
{
    (void)pclk2_div_flags;
#ifdef CLOCK_DYNAMIC
    if ((pclk2_div_flags & RCC_CFGR_PPRE2_2) == 0)
        pclk2_div_flags = 0;
    else
        pclk2_div_flags = 1 + ((pclk2_div_flags & (RCC_CFGR_PPRE2_1 | RCC_CFGR_PPRE2_0)) >> 13);

    PCLK2_FREQ = HCLK_FREQ >> pclk2_div_flags;
#endif
}

static void update_HCLK_FREQ (uint32_t hclk_div_flags)
{
    (void)hclk_div_flags;
#ifdef CLOCK_DYNAMIC
    if ((hclk_div_flags & RCC_CFGR_HPRE_3) == 0)
        ;
    else
    {
        hclk_div_flags = 1 + ((hclk_div_flags & (RCC_CFGR_HPRE_1 | RCC_CFGR_HPRE_0)) >> 4);
        if (hclk_div_flags > 4)
            hclk_div_flags++;
    }

    HCLK_FREQ = SYSCLK_FREQ >> hclk_div_flags;

    update_PCLK1_FREQ (RCC->CFGR & RCC_CFGR_PPRE1);
    update_PCLK2_FREQ (RCC->CFGR & RCC_CFGR_PPRE2);
#endif
}

DYNCLKFUN uint8_t sysclk_HSI ()
{
    if (clock_HSI_start () != 0)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
        ;

    // Set 0 wait states, don't touch prefetch
    FLASH->ACR = FLASH->ACR & ~FLASH_ACR_LATENCY;

#ifdef CLOCK_DYNAMIC
    SYSCLK_FREQ = HSI_VALUE;
    update_HCLK_FREQ (RCC->CFGR & RCC_CFGR_HPRE);
#endif

    return 0;
}

DYNCLKFUN uint8_t sysclk_HSE ()
{
    if (clock_HSE_start () != 0)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE)
        ;

#ifdef CLOCK_DYNAMIC
    SYSCLK_FREQ = HSE_VALUE;
    update_HCLK_FREQ (RCC->CFGR & RCC_CFGR_HPRE);
#endif

    return 0;
}

DYNCLKFUN uint8_t clock_PLL_setup (uint8_t clksrc,
    uint32_t pllm, uint32_t plln, uint32_t pllp, uint32_t pllq)
{
    // We can't change PLL settings if PLL is on
    if (RCC->CR & RCC_CR_PLLON)
        return 2;

    // Check that the arguments are in range
    if ((pllm & ~63) || (pllm < 2) ||
        (plln < 50) || (plln > 432) ||
        (pllp < 2) || (pllp > 8) || (pllp & 1) ||
        (pllq < 2) || (pllq > 15))
        return 3;

    // Enable the clock on which PLL is based on
    if (((clksrc == CLKSRC_HSI) && (clock_HSI_start () != 0)) ||
        ((clksrc == CLKSRC_HSE) && (clock_HSE_start () != 0)))
        return 4;

    // Configure PLL source and multiplier
    RCC->PLLCFGR = (RCC->CFGR & ~(RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLP |
        RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLSRC)) |
        ((clksrc == CLKSRC_HSE) ? RCC_PLLCFGR_PLLSRC_HSE : 0) |
        (pllm) | (plln << 6) | ((pllp / 2 - 1) << 16) | (pllq << 24);

    return 0;
}

DYNCLKFUN uint8_t sysclk_PLL ()
{
    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;

    // Wait for PLL to start up
    for (uint32_t i = 0; ; i++)
    {
        if (RCC->CR & RCC_CR_PLLRDY)
            break;

        if (i > HSE_STARTUP_TIMEOUT)
            return 1;
    }

#ifdef CLOCK_DYNAMIC
    // Compute resulting clock frequency
    uint32_t pllcfgr = RCC->PLLCFGR;
    uint32_t pllm = pllcfgr & 63;
    uint32_t plln = (pllcfgr >> 6) & 511;
    uint32_t pllp = (((pllcfgr >> 16) & 3) + 1) * 2;
    uint32_t pllq = (pllcfgr >> 24) & 15;

    uint32_t VCO_IN_FREQ = ((pllcfgr & RCC_PLLCFGR_PLLSRC_HSE) ?
        HSE_VALUE : HSI_VALUE) / pllm;
    if ((VCO_IN_FREQ < 1000000) || (VCO_IN_FREQ > 2000000))
        // VCO input frequency must be from 1 to 2 MHz!
        return 2;

    uint32_t VCO_OUT_FREQ = VCO_IN_FREQ * plln;
    if (VCO_OUT_FREQ < 100000000)
        // VCO output frequency must not be less than 100MHz!
        return 3;

    SYSCLK_FREQ = VCO_OUT_FREQ / pllp;
    if (SYSCLK_FREQ > SYSCLK_FREQ_MAX)
        // System clock frequency is too high!
        return 4;

    PLL48CK_FREQ = VCO_OUT_FREQ / pllq;
#endif

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        ;

#ifdef CLOCK_DYNAMIC
    update_HCLK_FREQ (RCC->CFGR & RCC_CFGR_HPRE);
#endif

    return 0;
}

DYNCLKFUN uint8_t clock_AHB (uint32_t hclk_div_flags)
{
    if (hclk_div_flags & ~RCC_CFGR_HPRE)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE) | hclk_div_flags;

    update_HCLK_FREQ (hclk_div_flags);

    return 0;
}

DYNCLKFUN uint8_t clock_APB1 (uint32_t pclk1_div_flags)
{
    if (pclk1_div_flags & ~RCC_CFGR_PPRE1)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE1) | pclk1_div_flags;

    update_PCLK1_FREQ (pclk1_div_flags);

    return 0;
}

DYNCLKFUN uint8_t clock_APB2 (uint32_t pclk2_div_flags)
{
    if (pclk2_div_flags & ~RCC_CFGR_PPRE2)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE2) | pclk2_div_flags;

    update_PCLK2_FREQ (pclk2_div_flags);

    return 0;
}

// Called from SystemInit ()
void clock_init ()
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    /* set CP10 and CP11 Full Access */
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
#endif

    // First, reset the clock system to default state
    clock_reset ();

    // optimize out the calls if flags are zeros (set by clock_reset())
    if (HCLK_DIV != 1)
        clock_AHB (HCLK_DIV_FLAGS (HCLK_DIV));
    if (PCLK1_DIV != 1)
        clock_APB1 (PCLK1_DIV_FLAGS (PCLK1_DIV));
    if (PCLK2_DIV != 1)
        clock_APB2 (PCLK2_DIV_FLAGS (PCLK2_DIV));

    // set up flash memory interface
    clock_flash_setup (FLASH_WS, FLASH_PREFETCH);

    // Now, configure clocks to desired state
    if (
#if JOIN2(CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSI
        sysclk_HSI ()
#elif JOIN2(CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSE
        sysclk_HSE ()
#elif JOIN2(CLKSRC_,SYSCLK_SOURCE) == CLKSRC_PLL
        clock_PLL_setup (JOIN2 (CLKSRC_,PLL_SOURCE), PLL_M, PLL_N, PLL_P, PLL_Q) ||
        sysclk_PLL ()
#endif
       )
    {
        // failure-basilure, what do we do, oh, oh...
    }

    // Enable instruction & data cache
    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN;

    /* Configure the Vector Table location add offset address */
#ifdef VECT_TAB_OFFSET
#ifdef VECT_TAB_SRAM
    /* Vector Table Relocation in Internal SRAM */
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;
#else
    /* Vector Table Relocation in Internal FLASH */
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
#endif
#endif
}
