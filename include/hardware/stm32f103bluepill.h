/*
    STM32F103 "Blue Pill" board hardware definition
    Copyright (C) 2020 by Andrew Zabolotny

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

// System clock configuration - define before including clocks-*.h
#define HSE_VALUE		8000000
#define SYSCLK_SOURCE		HSE
// Comment out to save a few bytes
#define CLOCK_DYNAMIC

#include "stm32f1xx.h"
#include "clocks-stm32f1.h"

// The debug LED
#define LED_PORT		C
#define LED_BIT			13
#define LED_GPIO_CONFIG		OUTPUT_2MHz,PUSHPULL,0

// Use APB2 clock for USART1
#define USART1_CLOCK		APB2_CLOCK
// Configure USART1 on PA9(TX)/PA10(RX)
#define USART1_SETUP		(USART_BAUD (9600) | USART_CHARBITS_8 | USART_PARITY_NONE | USART_STOPBITS_1)

// UART TX pin
#define USART1_TX_PORT		A
#define USART1_TX_BIT		9
#define USART1_TX_GPIO_CONFIG	OUTPUT_2MHz,AF_PUSHPULL,1

// Experimental feature: use UART only for receiving, TX is 'muted'
#define USART1_TX_MUTE_PORT	A
#define USART1_TX_MUTE_BIT	9
#define USART1_TX_MUTE_GPIO_CONFIG INPUT,FLOATING,X

// UART RX pin
#define USART1_RX_PORT		A
#define USART1_RX_BIT		10
#define USART1_RX_GPIO_CONFIG	INPUT,FLOATING,X

// USART1 DMA channels
#define USART1_TX_DMA_NUM	1
#define USART1_TX_DMA_CHAN	4
#define USART1_TX_DMA_IRQ_PRIO	0
#define USART1_RX_DMA_NUM	1
#define USART1_RX_DMA_CHAN	5
#define USART1_RX_DMA_IRQ_PRIO	0

// Also we have USB connected to USB_DP and USB_DM but we don't support that yet

// That's all we have, folks!

#endif // __HARDWARE_H__
