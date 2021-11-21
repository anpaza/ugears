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

// Microcontroller base definitions
#include "cmsis.h"

// System clock definitions
#include "clocks.h"

#include <useful/useful.h>
#include <useful/time.h>

// Peripherial libs
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

#endif // _STM32_UGEARS_H
