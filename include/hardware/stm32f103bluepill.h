/*
    STM32F103 "Blue Pill" board hardware definition
    Copyright (C) 2020 by Andrew Zabolotny

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _HARDWARE_H
#define _HARDWARE_H

// System clock configuration - define before including clocks-*.h
#define HSE_VALUE		8000000
// Use 48MHz clock (for USB) using HSE crystal as base for PLL
#define SYSCLK_SOURCE		PLL
#define PLL_SOURCE              HSE
#define PLL_DIV                 2
#define PLL_MUL                 12
#define CLOCK_USB_DIV           1

// Comment out to save a few bytes
#define CLOCK_DYNAMIC

// Debug LED cathode connected to VCC, anode to PC13 via 510 Ohm
#define LED_PORT		C
#define LED_PIN			13
#define LED_GPIO_CONFIG		OUTPUT_2MHz,OPENDRAIN,1

// We use USART1 in all samples but you can change it
// to any other USART by modifying the values below
#define SERIAL_USART_NUM	1
// Serial port setup
#define SERIAL_SETUP		USART_DEFAULT_SETUP
// Serial port IRQ priority
#define SERIAL_USART_IRQ_PRIO	32

// UART TX pin
#define SERIAL_TX_PORT		A
#define SERIAL_TX_PIN		9
#define SERIAL_TX_GPIO_CONFIG	OUTPUT_2MHz,AF_PUSHPULL,1

// Experimental feature: use UART only for receiving, TX is 'muted'
#define SERIAL_TX_MUTE_PORT	A
#define SERIAL_TX_MUTE_PIN	9
#define SERIAL_TX_MUTE_GPIO_CONFIG INPUT,FLOATING,X

// UART RX pin
#define SERIAL_RX_PORT		A
#define SERIAL_RX_PIN		10
#define SERIAL_RX_GPIO_CONFIG	INPUT,FLOATING,X

// USART1 DMA channels
#define SERIAL_TX_DMA_NUM	1
#define SERIAL_TX_DMA_STRM	4
#define SERIAL_TX_DMA_IRQ_PRIO	16
#define SERIAL_RX_DMA_NUM	1
#define SERIAL_RX_DMA_STRM	5
#define SERIAL_RX_DMA_IRQ_PRIO	16

// Also we have USB connected to USB_DP and USB_DM

#define USB_IRQ_PRIO		128

#define USB_DM_PORT		A
#define USB_DM_PIN		11
#define USB_DM_GPIO_CONFIG	INPUT,FLOATING,X

#define USB_DP_PORT		A
#define USB_DP_PIN		12
#define USB_DP_GPIO_CONFIG	INPUT,FLOATING,X

// Bring DP down for a few msecs to attract USB Host attention
#define USB_DP_INIT_PORT	A
#define USB_DP_INIT_PIN		12
#define USB_DP_INIT_GPIO_CONFIG	OUTPUT_2MHz,OPENDRAIN,0

//#define USB_CDC_STR_MANUFACTURER	u"uGears"
#define USB_CDC_STR_PRODUCT		u"uGears USB CDC device example"
//#define USB_CDC_STR_SERIALNUMBER	u"9999-9999-9999"

// USB CDC driver enables support for Set_Line_Coding, Set_Control_Line_State,
// Get_Line_Coding, and the notification Serial_State.
#define USB_CDC_LINE_CONTROL    1
// Device version
#define USB_CDC_VER             USB_BCD_VER (0,1)

// That's all we have, folks!

#endif // _HARDWARE_H
