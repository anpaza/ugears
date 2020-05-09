/*
    STM32F103 "Blue Pill" board hardware definition
    Copyright (C) 2018 by Andrew Zabolotny

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
#define LED_GPIO_CONFIG		PUSHPULL,X,LOW,X,0

// Configure USART1 on PA9(TX)/PA10(RX)

// UART TX pin
#define USART1_TX_PORT		A
#define USART1_TX_BIT		9
#define USART1_TX_GPIO_CONFIG	AF_PUSHPULL,X,LOW,1,1

// UART RX pin
#define USART1_RX_PORT		A
#define USART1_RX_BIT		10
#define USART1_RX_GPIO_CONFIG	AF,X,LOW,1,1

// Also we have USB connected to USB_DP and USB_DM but we don't support that yet

// That's all we have, folks!

#endif // __HARDWARE_H__
