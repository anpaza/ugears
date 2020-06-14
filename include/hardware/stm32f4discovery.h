/*
    STM32F4-Discovery board hardware definition
    Copyright (C) 2014 by Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

// System clock configuration - define before including clocks-*.h
#define SYSCLK_SOURCE		HSE
#define HSE_VALUE		8000000

//#define SYSCLK_SOURCE		HSI

//#define SYSCLK_SOURCE		PLL
//#define PLL_SOURCE		HSE
//#define PLL_M			8
//#define PLL_N			100
//#define PLL_P			4
//#define PLL_Q			2

// Uncomment to test dynamic clock changing functions
//#define CLOCK_DYNAMIC

// User LD3: orange LED is a user LED connected to the I/O PD13 of the STM32F407VGT6.
#define LED3_PORT		D
#define LED3_PIN		13
#define LED3_GPIO_CONFIG	PUSHPULL,X,LOW,X,0

// User LD4: green LED is a user LED connected to the I/O PD12 of the STM32F407VGT6.
#define LED4_PORT		D
#define LED4_PIN		12
#define LED4_GPIO_CONFIG	PUSHPULL,X,LOW,X,0

// User LD5: red LED is a user LED connected to the I/O PD14 of the STM32F407VGT6.
#define LED5_PORT		D
#define LED5_PIN		14
#define LED5_GPIO_CONFIG	PUSHPULL,X,LOW,X,0

// User LD6: blue LED is a user LED connected to the I/O PD15 of the STM32F407VGT6.
#define LED6_PORT		D
#define LED6_PIN		15
#define LED6_GPIO_CONFIG	PUSHPULL,X,LOW,X,0

// USB LD7: green LED indicates when VBUS is present on CN5 and is connected to PA9 of the STM32F407VGT6.
#define LED7_PORT		A
#define LED7_PIN		9
#define LED7_GPIO_CONFIG	PUSHPULL,X,LOW,X,0

// USB LD8: red LED indicates an overcurrent from VBUS of CN5 and is connected to the I/O PD5 of the STM32F407VGT6.
#define LED8_PORT		D
#define LED8_PIN		5
#define LED8_GPIO_CONFIG	PUSHPULL,X,LOW,X,0

// B1 USER: User and Wake-Up buttons are connected to the I/O PA0 of the STM32F407VGT6.
#define BUT1_PORT		A
#define BUT1_PIN		0
#define BUT1_GPIO_CONFIG	INPUT,X,LOW,X,X

// We use USART1 in all samples but you can change it
// to any other USART by modifying the values below
#define SERIAL_USART_NUM	1
// Serial port setup
#define SERIAL_SETUP		USART_DEFAULT_SETUP
// Serial IRQ priority
#define SERIAL_IRQ_PRIO		0

// USART1 TX
#define SERIAL_TX_PORT		B
#define SERIAL_TX_PIN		6
#define SERIAL_TX_GPIO_CONFIG	AF_PUSHPULL,X,MEDIUM,7,1

// USART1 RX
#define SERIAL_RX_PORT		B
#define SERIAL_RX_PIN		7
#define SERIAL_RX_GPIO_CONFIG	AF,X,MEDIUM,7,X

// That's all we have, folks!

#endif // __HARDWARE_H__
