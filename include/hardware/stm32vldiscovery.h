/*
    STM32VL-Discovery board hardware definition
    Copyright (C) 2014 by Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _HARDWARE_H
#define _HARDWARE_H

// System clock configuration - define before including clocks-*.h
#define SYSCLK_SOURCE		HSE
#define HSE_VALUE               8000000
#define CLOCK_DYNAMIC

// RTC crystal frequency, Hz
#define LSE_VALUE		32768

/// Use a more or less high precision RTC clock
#define RTC_TICKS_PER_SEC       32

/// Use a 8Hz SysTick timer (stm32vldiscovery.c)
#define CLOCKS_PER_SEC          8

// A green LED on port PC9
#define GLED_PORT		C
#define GLED_PIN		9
#define GLED_GPIO_CONFIG	OUTPUT_2MHz,PUSHPULL,0

// And a blue one on PC8
#define BLED_PORT		C
#define BLED_PIN		8
#define BLED_GPIO_CONFIG	OUTPUT_2MHz,PUSHPULL,0

// And a button on PA0
#define USRBUT_PORT		A
#define USRBUT_PIN		0
#define USRBUT_GPIO_CONFIG	INPUT,FLOATING,X

// Default LED is green
#define LED_PORT                GLED_PORT
#define LED_PIN                 GLED_PIN
#define LED_GPIO_CONFIG         GLED_GPIO_CONFIG

// We use USART1 in all samples but you can change it
// to any other USART by modifying the values below
#define SERIAL_USART_NUM	1
// Serial port setup
#define SERIAL_SETUP		USART_DEFAULT_SETUP
// Serial port IRQ priority
#define SERIAL_USART_IRQ_PRIO	32

// USART1 I/O ports
#define SERIAL_TX_PORT		A
#define SERIAL_TX_PIN		9
#define SERIAL_TX_GPIO_CONFIG	OUTPUT_2MHz,AF_PUSHPULL,1

// Experimental feature: use UART only for receiving, TX is 'muted'
#define SERIAL_TX_MUTE_PORT	A
#define SERIAL_TX_MUTE_PIN	9
#define SERIAL_TX_MUTE_GPIO_CONFIG INPUT,FLOATING,X

#define SERIAL_RX_PORT		A
#define SERIAL_RX_PIN		10
#define SERIAL_RX_GPIO_CONFIG	INPUT,FLOATING,X

// USART1 DMA channels
#define SERIAL_TX_DMA_NUM	1
#define SERIAL_TX_DMA_CHAN	4
#define SERIAL_TX_DMA_IRQ_PRIO	0
#define SERIAL_RX_DMA_NUM	1
#define SERIAL_RX_DMA_CHAN	5
#define SERIAL_RX_DMA_IRQ_PRIO	0

// That's all we have, folks!

#endif // _HARDWARE_H
