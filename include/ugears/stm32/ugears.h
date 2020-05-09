/*
    Basic macros to deal with low-level hardware
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _UGEARS_H
#define _UGEARS_H

/**
 * @file ugears.h
 *      A top-level include file that just pulls in all the headers
 *      from the ugears library.
 */

// Always include hardware description file first
#include HARDWARE_H
// System clock definitions
#include "clocks.h"

#include "useful.h"

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

#endif // _UGEARS_H
