/*
    Basic macros to deal with low-level hardware
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_UGEARS_H
#define _STM32_UGEARS_H

/**
 * @file ugears.h
 *      A top-level include file that just pulls in all the headers
 *      from the ugears library.
 */

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

// System clock definitions
#include "clocks.h"

#include <useful/useful.h>
#include <useful/time.h>

#include "flash.h"
#include "adc.h"
#include "can.h"
#include "rcc.h"
#include "dma.h"
#include "gpio.h"
#include "nvic.h"
#include "exti.h"
#include "i2c.h"
#include "iwdg.h"
#include "pwr.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "spi.h"
#include "systick.h"

#include "atomic.h"

#endif // _STM32_UGEARS_H
