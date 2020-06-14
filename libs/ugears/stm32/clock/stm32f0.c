/*
    STM32F0xx system startup code
    Copyright (C) 2015 by Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include HARDWARE_H
#include <ugears/ugears.h>

#ifdef CLOCK_DYNAMIC
#define DYNCLKFUN
uint32_t SYSCLK_FREQ, HCLK_FREQ, PCLK_FREQ;
#else
#define DYNCLKFUN static inline
#endif

DYNCLKFUN void clock_reset ()
{
    /* Disable all interrupts from clock system */
    RCC->CIR = 0x00000000;

    /* We'll switch to HSI, so turn it on */
    RCC->CR |= RCC_CR_HSION;

    /* Switch system clock to HSI (SW=0), clear HCLK, PCLK, MCO dividers */
    RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE |
#ifdef RCC_CFGR_MCO_PRE
        RCC_CFGR_MCO_PRE |
#endif
        RCC_CFGR_PLLNODIV | RCC_CFGR_ADCPRE | RCC_CFGR_MCO);

    /* Turn off HSE and PLL, disable clock security system */
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);

    /* Disable bypassing HSE with external oscillator */
    RCC->CR &= ~RCC_CR_HSEBYP;

    /* Reset the PLL */
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMUL);

    /* Clear PLL pre-divider */
    RCC->CFGR2 &= ~RCC_CFGR2_PREDIV;

    /* Reset UART, I2C, USB, CEC, ADC clocks */
    RCC->CFGR3 &= ~(
#ifdef RCC_CFGR3_USART2SW
        RCC_CFGR3_USART2SW |
#endif
#ifdef RCC_CFGR3_CECSW
        RCC_CFGR3_CECSW |
#endif
#ifdef RCC_CFGR3_USBSW
        RCC_CFGR3_USBSW |
#endif
        RCC_CFGR3_USART1SW |
        RCC_CFGR3_I2C1SW);

    /* Disable HSI14 */
    RCC->CR2 &= ~RCC_CR2_HSI14ON;

    /* Clear interrupt flags that could be set during the switching */
    RCC->CIR = RCC_CIR_LSIRDYC | RCC_CIR_LSERDYC | RCC_CIR_HSIRDYC |
        RCC_CIR_HSERDYC | RCC_CIR_PLLRDYC | RCC_CIR_HSI14RDYC |
#ifdef RCC_CIR_HSI48RDYC
        RCC_CIR_HSI48RDYC |
#endif
        RCC_CIR_CSSC;
}

DYNCLKFUN void update_PCLK_FREQ (uint32_t pclk_div_flags)
{
    (void)pclk_div_flags;
#ifdef CLOCK_DYNAMIC
    if (pclk_div_flags != 0)
        pclk_div_flags = 1 + ((pclk_div_flags & (RCC_CFGR_PPRE_1 | RCC_CFGR_PPRE_0)) >> 8);

    PCLK_FREQ = HCLK_FREQ >> pclk_div_flags;
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

    update_PCLK_FREQ (RCC->CFGR & RCC_CFGR_PPRE);
#endif
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

    // Disable all other clocks
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_PLLON);

    // Disable latency & prefetch
    FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);

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
    if (clock_HSE_start () != 0)
        return 1;

    // If clock is >24MHz, enable flash latency & prefetch
    if (HSE_VALUE > 24000000)
        FLASH->ACR |= (FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE)
        ;

    // If clock is <=24MHz, disable flash latency & prefetch
    if (HSE_VALUE <= 24000000)
        FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);

    // Disable all other clocks
    RCC->CR &= ~(RCC_CR_HSION | RCC_CR_PLLON);

#ifdef CLOCK_DYNAMIC
    SYSCLK_FREQ = HSE_VALUE;
    update_HCLK_FREQ (RCC->CFGR & RCC_CFGR_HPRE);
#endif

    return 0;
}

DYNCLKFUN uint8_t sysclk_PLL (uint8_t clksrc, uint32_t plldiv, uint32_t pllmul)
{
    // We can't change PLL settings if PLL is on
    if (RCC->CR & RCC_CR_PLLON)
    {
        uint8_t ret = sysclk_HSI ();
        if (ret != 0)
            return ret;
    }

    // Check that pllmul is 2..16 and plldiv is 1..16
    if (((1 << pllmul) & 0x1FFFC) == 0 || ((1 << plldiv) & 0x1FFFE) == 0)
        return 3;

    // Enable the clock on which PLL is based on
    if (((clksrc == CLKSRC_HSI) && (clock_HSI_start () != 0)) ||
        ((clksrc == CLKSRC_HSE) && (clock_HSE_start () != 0)))
        return 4;

    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL)) |
        ((clksrc == CLKSRC_HSI) ?
            RCC_CFGR_PLLSRC_HSI_DIV2 : RCC_CFGR_PLLSRC_HSE_PREDIV) |
        ((pllmul - 2) << 18);
    RCC->CFGR2 = (RCC->CFGR2 & ~RCC_CFGR2_PREDIV) | ((plldiv - 1) << RCC_CFGR2_PREDIV_Pos);

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
#  if defined PLL_DIV_AFTER
        (((clksrc == CLKSRC_HSI) ? HSI_VALUE : HSE_VALUE) / plldiv) * pllmul;
#  else
        ((clksrc == CLKSRC_HSI) ? (HSI_VALUE / 2) : (HSE_VALUE / plldiv)) * pllmul;
#  endif
#else
#  define FREQ SYSCLK_FREQ
#endif

    // If clock is >24MHz, enable flash latency & prefetch
    if (FREQ > 24000000)
        FLASH->ACR |= (FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        ;

    // If clock is <=24MHz, disable flash latency & prefetch
    if (FREQ <= 24000000)
        FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTBE);

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

DYNCLKFUN uint8_t clock_APB (uint32_t pclk_div_flags)
{
    if (pclk_div_flags & ~RCC_CFGR_PPRE)
        return 1;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE) | pclk_div_flags;

    update_PCLK_FREQ (pclk_div_flags);

    return 0;
}

// Called from SystemInit()
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
        sysclk_PLL (PLL_SOURCE, PLL_DIV, PLL_MUL)
#endif
        == 0)
    {
        // optimize out the calls if flags are zeros (set by clock_reset())
        if (HCLK_DIV != 1)
            clock_AHB (HCLK_DIV_FLAGS (HCLK_DIV));
        if (PCLK_DIV != 1)
            clock_APB (PCLK_DIV_FLAGS (PCLK_DIV));
    }

    // Select the appropiate clocks for some peripherials, if asked
    uint32_t m = 0, n = 0;

#if defined CLOCK_USB && defined RCC_CFGR3_USBSW
    m |= RCC_CFGR3_USBSW;
    n |= JOIN2 (RCC_CFGR3_USBSW_,CLOCK_USB);
#endif
#if defined CLOCK_CEC && defined RCC_CFGR3_CECSW
    m |= RCC_CFGR3_CECSW;
    n |= JOIN2 (RCC_CFGR3_CECSW_,CLOCK_CEC);
#endif
#if defined CLOCK_I2C1 && defined RCC_CFGR3_I2C1SW
    m |= RCC_CFGR3_I2C1SW;
    n |= JOIN2 (RCC_CFGR3_I2C1SW_,CLOCK_I2C1);
#endif
#if defined CLOCK_USART1 && defined RCC_CFGR3_USART1SW
    m |= RCC_CFGR3_USART1SW;
    n |= JOIN2 (RCC_CFGR3_USART1SW_,CLOCK_USART1);
#endif
#if defined CLOCK_USART2 && defined RCC_CFGR3_USART2SW
    m |= RCC_CFGR3_USART2SW;
    n |= JOIN2 (RCC_CFGR3_USART2SW_,CLOCK_USART2);
#endif

    // Optimize out the whole stuff if using defaults (set by clock_reset())
    if (n)
        RCC->CFGR3 = (RCC->CFGR3 & ~m) | n;
}
