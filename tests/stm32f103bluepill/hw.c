/*
 * STM32VL-DISCOVERY specific functions.
 * Copyright (C) 2014 Andrey Zabolotnyi
 */

#include "hw.h"

void usart1_init ()
{
    // Enable USART1 and GPIOs
    RCC_BEGIN;
        RCC_ENA (_USART1);
        RCC_ENA (RCC_GPIO (USART1_TX));
        RCC_ENA (RCC_GPIO (USART1_RX));
    RCC_END;

    // Initialize USART GPIOs
    GPIO_SETUP (USART1_TX);
    GPIO_SETUP (USART1_RX);

    // Initialize USART1
    usart_init (USART1, USART1_CLOCK, USART1_SETUP);

    // Redirect printf() over USART1
    usart_printf (USART1);
}

void led_init ()
{
    // Enable clocking for board peripherials
    RCC_ENABLE (RCC_GPIO (LED));
    // Initial pin setup
    GPIO_SETUP (LED);
}

__WEAK void SysTick_Handler ()
{
    clock++;
}

void systick_init ()
{
    // Set up system timer for CLOCKS_PER_SEC interrupts per second
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);
    // Enable interrupts from SysTick counter
    systick_int_enable (0);
}

// At early initialization disable JTAG, enable SWD
__attribute__((constructor)) void disable_jtag ()
{
    RCC_ENABLE (_AFIO);
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
}
