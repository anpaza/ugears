/*
 * STM32VL-DISCOVERY specific functions.
 * Copyright (C) 2014 Andrey Zabolotnyi
 */

#include "hw.h"

void serial_init ()
{
    // Enable serial and GPIOs
    RCC_BEGIN;
        RCC_ENA_USART (SERIAL);
        RCC_ENA_GPIO (SERIAL_TX);
        RCC_ENA_GPIO (SERIAL_RX);
    RCC_END;

    // Initialize USART GPIOs
    GPIO_SETUP (SERIAL_TX);
    GPIO_SETUP (SERIAL_RX);

    // Initialize serial port
    usart_init (USART (SERIAL), CLOCK_USART (SERIAL), SERIAL_SETUP);

    // Redirect printf() over serial port
    usart_printf (USART (SERIAL));
}

void led_init ()
{
    // Enable clocking for board peripherials
    RCC_BEGIN;
        RCC_ENA_GPIO (BLED);
        RCC_ENA_GPIO (GLED);
        RCC_ENA_GPIO (USRBUT);
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
