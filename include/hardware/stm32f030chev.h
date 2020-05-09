/*
    STM32F030 Chinese evaluation board hardware definition
    Copyright (C) 2014 by Andrew Zabolotny

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

#include "stm32f0xx.h"
#include "clocks-stm32f0.h"

// The debug LED
#define LED_PORT		A
#define LED_PIN			4
#define LED_GPIO_CONFIG		PUSHPULL,X,LOW,X,0

// Use APB1 clock (PCLK) for USART
#define USART1_CLOCK		APB_CLOCK
// Example serial port setup
#define USART1_SETUP		(USART_BAUD (9600) | USART_CHARBITS_8 | USART_PARITY_NONE | USART_STOPBITS_1)

// UART TX pin
#define USART1_TX_PORT		A
#define USART1_TX_PIN		9
#define USART1_TX_GPIO_CONFIG	AF_PUSHPULL,X,LOW,1,1

// Experimental feature: use UART only for receiving, TX is 'muted'
#define USART1_TX_MUTE_PORT	A
#define USART1_TX_MUTE_PIN	9
#define USART1_TX_MUTE_GPIO_CONFIG INPUT,X,LOW,X,X

// UART RX pin
#define USART1_RX_PORT		A
#define USART1_RX_PIN		10
#define USART1_RX_GPIO_CONFIG	AF,X,LOW,1,1

// That's all we have, folks!

#endif // __HARDWARE_H__
