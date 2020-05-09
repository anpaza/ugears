/*
 * STM32VL-DISCOVERY specific functions.
 * Copyright (C) 2014 Andrey Zabolotnyi
 */

#include "stm32vldiscovery.h"

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

    // USART1 is on APB2
    usart_init (USART1, APB2_CLOCK, USART1_SETUP);

    // Redirect printf() over USART1
    usart_printf (USART1);
}

void led_init ()
{
    // Enable clocking for board peripherials
    RCC_BEGIN;
        RCC_ENA (RCC_GPIO (BLED));
        RCC_ENA (RCC_GPIO (GLED));
        RCC_ENA (RCC_GPIO (USRBUT));
    RCC_END;

    // Set up LED pins
    GPIO_SETUP (GLED);
    GPIO_SETUP (BLED);
    // Set up button pin
    GPIO_SETUP (USRBUT);
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
