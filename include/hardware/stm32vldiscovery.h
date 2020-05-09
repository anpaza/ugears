/*
    STM32VL-Discovery board hardware definition
    Copyright (C) 2014 by Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

// System clock configuration - define before including clocks-*.h
#define SYSCLK_SOURCE		HSE
#define HSE_VALUE               8000000
#define CLOCK_DYNAMIC

#include "stm32f1xx.h"
#include "clocks-stm32f1.h"

// RTC crystal frequency, Hz
#define LSE_VALUE		32768

/// Use a more or less high precision RTC clock
#define RTC_TICKS_PER_SEC       32

/// Use a 8Hz SysTick timer (stm32vldiscovery.c)
#define CLOCKS_PER_SEC          8

// A green LED on port PC9
#define GLED_PORT		C
#define GLED_BIT		9
#define GLED_GPIO_CONFIG	OUTPUT_2MHz,PUSHPULL,0

// And a blue one on PC8
#define BLED_PORT		C
#define BLED_BIT		8
#define BLED_GPIO_CONFIG	OUTPUT_2MHz,PUSHPULL,0

// And a button on PA0
#define USRBUT_PORT		A
#define USRBUT_BIT		0
#define USRBUT_GPIO_CONFIG	INPUT,FLOATING,X

// Use APB2 clock for USART1
#define USART1_CLOCK		APB2_CLOCK

// Example serial port setup
#define USART1_SETUP		(USART_BAUD (9600) | USART_CHARBITS_8 | USART_PARITY_NONE | USART_STOPBITS_1)
// Example other serial port
#define USART2_SETUP		(USART_BAUD (9600) | USART_CHARBITS_8 | USART_PARITY_NONE | USART_STOPBITS_1)

// USART IRQ priorities
#define USART1_IRQ_PRIO		255
#define USART2_IRQ_PRIO		255

// USART1 I/O ports
#define USART1_TX_PORT		A
#define USART1_TX_BIT		9
#define USART1_TX_GPIO_CONFIG	OUTPUT_2MHz,AF_PUSHPULL,1

// Experimental feature: use UART only for receiving, TX is 'muted'
#define USART1_TX_MUTE_PORT	A
#define USART1_TX_MUTE_BIT	9
#define USART1_TX_MUTE_GPIO_CONFIG INPUT,FLOATING,X

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

// USART2 I/O ports
#define USART2_TX_PORT		A
#define USART2_TX_BIT		2
#define USART2_TX_GPIO_CONFIG	OUTPUT_2MHz,AF_PUSHPULL,1

#define USART2_RX_PORT		A
#define USART2_RX_BIT		3
#define USART2_RX_GPIO_CONFIG	INPUT,FLOATING,X

// That's all we have, folks!

#endif // __HARDWARE_H__
