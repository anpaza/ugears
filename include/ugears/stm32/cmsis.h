/*
    Basic microcontroller-specific definitions
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_CMSIS_H
#define _STM32_CMSIS_H

/**
 * @file cmsis.h
 *      This file will include the microcontroller and board definition
 *      files.
 */

// User's hardware description file
#include HARDWARE_H

// Include CMSIS MCU-specific header file
#if defined STM32F0
#include <cmsis/stm32/stm32f0xx.h>
#elif defined STM32F1
#include <cmsis/stm32/stm32f1xx.h>
#elif defined STM32F2
#include <cmsis/stm32/stm32f2xx.h>
#elif defined STM32F3
#include <cmsis/stm32/stm32f3xx.h>
#elif defined STM32F4
#include <cmsis/stm32/stm32f4xx.h>
#elif defined STM32H7
#include <cmsis/stm32/stm32h7xx.h>
#else
#error "Unknown MCU type"
#endif

#endif // _STM32_CMSIS_H
