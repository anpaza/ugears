/*
    Transmutate your STM32F1 BluePill into a cheap USB-UART dongle!
    Copyright (C) 2020 by Andrew Zabolotny

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

/*
 * This sample application uses:
 *
 * @li USB port for PC connection
 * @li One of the USARTx ports for target device connection
 * @li A LED to display activity
 */

#include "usb-cdc-acm.h"

// ----- // USB // ---------------------------------------------------------- //

void uca_received (const void *data, unsigned data_size)
{
    //uca_transmit (data, data_size);
}

void uca_line_state_changed ()
{
}

void uca_line_format_changed ()
{
    //uca_line_format.dwDTERate
    //uca_line_format.bDataBits
    //uca_line_format.bParityType
    //uca_line_format.bCharFormat
}

// ----- // USART // -------------------------------------------------------- //


// ----- // SysTick // ------------------------------------------------------ //

void SysTick_Handler ()
{
    clock++;

    //GPIO_TOGGLE (LED);
}

void systick_init ()
{
    // Set up system timer for CLOCKS_PER_SEC interrupts per second
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);
    // Enable interrupts from SysTick counter
    systick_int_enable (0);
}

int main (void)
{
    // Initialize activity LED
    RCC_ENABLE (RCC_GPIO (LED));
    GPIO_SETUP (LED);

    systick_init ();
    uca_init ();
    uca_printf ();

    for (;;)
    {
        // Do nothing :-D
        __WFI ();
    }
}
