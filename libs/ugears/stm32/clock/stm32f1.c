/*
    STM32F1xx system startup code
    Copyright (C) 2015 by Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include HARDWARE_H
#include "clocks-stm32f1.h"

#define _8MHZ			        8000000
#define _24MHZ			        24000000
#define _48MHZ			        48000000

#ifdef CLOCK_DYNAMIC
#define DYNCLKFUN
uint32_t SYSCLK_FREQ, HCLK_FREQ, PCLK1_FREQ, PCLK2_FREQ;
#else
#define DYNCLKFUN static inline
#endif

#if defined RCC_CFGR_PLLSRC && !defined RCC_CFGR_PLLSRC_HSI_DIV2
#  define RCC_CFGR_PLLSRC_HSI_DIV2      0
#endif
#if defined RCC_CFGR_PLLSRC && !defined RCC_CFGR_PLLSRC_HSE
#  define RCC_CFGR_PLLSRC_HSE           RCC_CFGR_PLLSRC
#endif

DYNCLKFUN void clock_reset ()
{
    /* Disable all interrupts from clock system */
    RCC->CIR = 0x00000000;

    /* We'll switch to HSI, so turn it on */
    RCC->CR |= RCC_CR_HSION;

    /* Switch system clock to HSI (SW=0), clear HCLK, PCLK1, PCLK2, MCO */
    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 |
        RCC_CFGR_PPRE2 | RCC_CFGR_MCO);

#ifdef RCC_APB1ENR_USBEN
    /* Turn off USB clock before turning off PLL */
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
#endif

    /* Turn off HSE and PLL, disable clock security system */
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);

    /* Disable bypassing HSE with external oscillator */
    RCC->CR &= ~RCC_CR_HSEBYP;

    /* Reset the PLL, also reset prescalers for some peripherials */
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL |
        RCC_CFGR_ADCPRE);

#ifdef RCC_CR_PLL2ON
    /* Disable PLL2 & PLL3, if supported */
    RCC->CR &= ~(RCC_CR_PLL2ON | RCC_CR_PLL3ON);
#endif
#ifdef RCC_CFGR2_PREDIV1
    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000;
#endif

    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = RCC_CIR_LSIRDYC | RCC_CIR_LSERDYC | RCC_CIR_HSIRDYC |
#ifdef RCC_CIR_PLL2RDYC
        RCC_CIR_PLL2RDYC | RCC_CIR_PLL3RDYC |
#endif
        RCC_CIR_HSERDYC | RCC_CIR_PLLRDYC | RCC_CIR_CSSC;
}

DYNCLKFUN void update_PCLK1_FREQ (uint32_t pclk1_div_flags)
{
    (void)pclk1_div_flags;
#ifdef CLOCK_DYNAMIC
    if (pclk1_div_flags != 0)
        pclk1_div_flags = 1 + ((pclk1_div_flags & (RCC_CFGR_PPRE1_1 | RCC_CFGR_PPRE1_0)) >> 8);

    PCLK1_FREQ = HCLK_FREQ >> pclk1_div_flags;
#endif
}

DYNCLKFUN void update_PCLK2_FREQ (uint32_t pclk2_div_flags)
{
    (void)pclk2_div_flags;
#ifdef CLOCK_DYNAMIC
    if (pclk2_div_flags != 0)
        pclk2_div_flags = 1 + ((pclk2_div_flags & (RCC_CFGR_PPRE2_1 | RCC_CFGR_PPRE2_0)) >> 11);

    PCLK2_FREQ = HCLK_FREQ >> pclk2_div_flags;
#endif
}

DYNCLKFUN void update_HCLK_FREQ (uint32_t hclk_div_flags)
{
    (void)hclk_div_flags;
#ifdef CLOCK_DYNAMIC
    if (hclk_div_flags == 0)
        ;
    else if ((hclk_div_flags & (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_2)) == RCC_CFGR_HPRE_3)
        hclk_div_flags = 1 + ((hclk_div_flags & (RCC_CFGR_HPRE_1 | RCC_CFGR_HPRE_0)) >> 4);
    else
        hclk_div_flags = 6 + ((hclk_div_flags & (RCC_CFGR_HPRE_1 | RCC_CFGR_HPRE_0)) >> 4);

    HCLK_FREQ = SYSCLK_FREQ >> hclk_div_flags;

    update_PCLK1_FREQ (RCC->CFGR & RCC_CFGR_PPRE1);
    update_PCLK2_FREQ (RCC->CFGR & RCC_CFGR_PPRE2);
#endif
}

DYNCLKFUN void clock_flash_setup (uint32_t clksrc, uint32_t sysclk)
{
    uint32_t acr = FLASH->ACR;

#ifdef FLASH_ACR_LATENCY
    // Disable latency & prefetch, enable half-cycle flash access
    acr &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);
#endif

    // Enable half-cycle flash access if <= 8MHz and HSE or HSI
    if (((clksrc == CLKSRC_HSE) && (sysclk == HSE_VALUE)) ||
        ((clksrc == CLKSRC_HSI) && (sysclk == HSI_VALUE)))
        acr |= FLASH_ACR_HLFCYA;
    else
        acr &= ~FLASH_ACR_HLFCYA;

#ifdef FLASH_ACR_PRFTBE
    if (sysclk < _24MHZ)
        ;
    else if (sysclk < _48MHZ)
        acr |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;
    else
        acr |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
#endif

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

void clock_HSI_stop ()
{
    RCC->CR &= ~RCC_CR_HSION;
}

DYNCLKFUN uint8_t sysclk_HSI ()
{
    if (clock_HSI_start () != 0)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
        ;

    clock_flash_setup (CLKSRC_HSI, HSI_VALUE);

#ifdef RCC_APB1ENR_USBEN
    /* Turn off USB clock before turning off PLL */
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
#endif

    // Disable all other clocks
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_PLLON);

#ifdef CLOCK_DYNAMIC
    SYSCLK_FREQ = HSI_VALUE;
    update_HCLK_FREQ (RCC->CFGR & RCC_CFGR_HPRE);
#endif

    return 0;
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

DYNCLKFUN uint8_t sysclk_HSE ()
{
    // If clock is >24MHz, switch to HSI, enable flash latency & prefetch
    if (HSE_VALUE > _24MHZ)
    {
        sysclk_HSI ();
        clock_flash_setup (CLKSRC_HSE, HSE_VALUE);
    }

    if (clock_HSE_start () != 0)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE)
        ;

    // If clock is <=24MHz, disable flash latency & prefetch
    if (HSE_VALUE <= _24MHZ)
        clock_flash_setup (CLKSRC_HSE, HSE_VALUE);

#ifdef RCC_APB1ENR_USBEN
    // Turn off USB clock before turning off PLL
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
#endif

    // Disable all other clocks
    RCC->CR &= ~(RCC_CR_HSION | RCC_CR_PLLON);

#ifdef CLOCK_DYNAMIC
    SYSCLK_FREQ = HSE_VALUE;
    update_HCLK_FREQ (RCC->CFGR & RCC_CFGR_HPRE);
#endif

    return 0;
}

DYNCLKFUN uint8_t sysclk_PLL (uint8_t clksrc, uint32_t plldiv, uint32_t pllmul
#ifdef RCC_CFGR2_PREDIV2
    , uint32_t pll2div, uint32_t pll2mul
#endif
    )
{
    // We can't change PLL settings if PLL is on
    if (RCC->CR & RCC_CR_PLLON)
        if (sysclk_HSI () != 0)
            return 2;

    // Check if PLL multipliers and dividers are in legal ranges
    // PLL multiplier from 2 to 16
    if (((1 << pllmul) & 0x1FFFC) == 0
#ifdef RCC_CFGR2_PREDIV1
        // PLL divider from 1 to 16
        || ((1 << plldiv) & 0x1FFFE) == 0
#else
        // PLL divider 1 or 2
        || ((1 << plldiv) & 0x6) == 0
#endif
        // PLL divider has no effect except 2 when used with HSI
        || ((clksrc == CLKSRC_HSI) && (plldiv != 2))
#ifdef RCC_CFGR2_PREDIV2
        || ((clksrc == CLKSRC_PLL2) &&
            ((1 << pll2mul) & 0x00117F00) == 0 || ((1 << pll2div) & 0x1FFFE) == 0)
#endif
       )
        return 3;

    // Enable the clock on which PLL is based on
    if (((clksrc == CLKSRC_HSI) && (clock_HSI_start () != 0)) ||
        (((clksrc == CLKSRC_HSE)
#ifdef RCC_CFGR2_PREDIV2
          || (clksrc == CLKSRC_PLL2)
#endif
        ) && (clock_HSE_start () != 0)))
        return 4;

    // Configure PLL source and multiplier
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL)) |
        ((clksrc == CLKSRC_HSI) ? RCC_CFGR_PLLSRC_HSI_DIV2 : RCC_CFGR_PLLSRC_HSE) |
        ((pllmul - 2) << RCC_CFGR_PLLMULL_Pos);

    // Configure PLL predivider
#ifdef RCC_CFGR2_PREDIV1
    RCC->CFGR2 = (RCC->CFGR2 & ~RCC_CFGR2_PREDIV1) | ((plldiv - 1) << RCC_CFGR2_PREDIV1_Pos);
#else
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLXTPRE) | ((plldiv == 1) ? 0 : RCC_CFGR_PLLXTPRE);
#endif

#if defined RCC_CFGR2_PREDIV2
    // Configure PLL2, if required
    if (clksrc == CLKSRC_PLL2)
    {
        // PLL2 pre-divider and multiplier
        RCC->CFGR2 = (RCC->CFGR2 & ~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL))
            | ((pll2div - 1) << 4)
            | (((pll2mul <= 16) ? (pll2mul - 2) : 15) << 8);

        // Enable PLL2
        RCC->CR |= RCC_CR_PLL2ON;

        // Wait for PLL2 to start up
        for (uint32_t i = 0; ; i++)
        {
            if (RCC->CR & RCC_CR_PLL2RDY)
                break;

            if (i > PLL_STARTUP_TIMEOUT)
                return 1;
        }
    }
#endif

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;

    // Wait for PLL to start up
    for (uint32_t i = 0; ; i++)
    {
        if (RCC->CR & RCC_CR_PLLRDY)
            break;

        if (i > PLL_STARTUP_TIMEOUT)
            return 1;
    }

#ifdef CLOCK_DYNAMIC
    // Compute resulting clock frequency
    uint32_t FREQ =
#  if defined RCC_CFGR2_PREDIV2
        ((clksrc == CLKSRC_PLL2) ? ((((HSE_VALUE / pll2div) * pll2mul) / plldiv) * pllmul) :
#  endif
        ((clksrc == CLKSRC_HSI) ? (HSI_VALUE / 2) : (HSE_VALUE / plldiv)) * pllmul;
#else
#  define FREQ SYSCLK_FREQ
#endif

    // If clock is >24MHz, enable flash latency & prefetch
    if (FREQ > _24MHZ)
    {
        sysclk_HSI ();
        clock_flash_setup (CLKSRC_PLL, FREQ);
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        ;

    // If clock is <=24MHz, disable flash latency & prefetch
    if (FREQ <= _24MHZ)
        clock_flash_setup (CLKSRC_PLL, FREQ);

    // Disable unused clocks
    if (clksrc == CLKSRC_HSI)
        RCC->CR &= ~RCC_CR_HSEON;
    else if (clksrc == CLKSRC_HSE)
        RCC->CR &= ~RCC_CR_HSION;

#ifdef CLOCK_DYNAMIC
    SYSCLK_FREQ = FREQ;
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
    // First, reset the clock system to default state
    clock_reset ();

    // Now, configure clocks to desired state
    if (
#if JOIN2(CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSI
        sysclk_HSI ()
#elif JOIN2(CLKSRC_,SYSCLK_SOURCE) == CLKSRC_HSE
        sysclk_HSE ()
#elif JOIN2(CLKSRC_,SYSCLK_SOURCE) == CLKSRC_PLL
        sysclk_PLL (JOIN2(CLKSRC_,PLL_SOURCE), PLL_DIV, PLL_MUL)
#endif
        == 0)
    {
        // optimize out the calls if flags are zeros (set by clock_reset())
        if (HCLK_DIV != 1)
            clock_AHB (HCLK_DIV_FLAGS (HCLK_DIV));
        if (PCLK1_DIV != 1)
            clock_APB1 (PCLK1_DIV_FLAGS (PCLK1_DIV));
        if (PCLK2_DIV != 1)
            clock_APB2 (PCLK2_DIV_FLAGS (PCLK2_DIV));
    }

    // Select the appropiate clocks for some peripherials, if asked
    uint32_t m = 0, n = 0;

#ifdef CLOCK_USB_DIV
    m |= RCC_CFGR_USBPRE_1;
    n |= JOIN2 (RCC_CFGR_USBPRE_,CLOCK_USB_DIV);
#endif
#ifdef CLOCK_ADC_DIV
    m |= RCC_CFGR_ADCPRE;
    n |= JOIN2 (RCC_CFGR_ADCPRE_DIV,CLOCK_ADC_DIV);
#endif

    // Optimize out the whole stuff if using defaults (set by clock_reset())
    if (n)
        RCC->CFGR = (RCC->CFGR & ~m) | n;
}
