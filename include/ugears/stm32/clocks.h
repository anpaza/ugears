/*
    System clocks management
    Copyright (C) 2015 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_CLOCKS_H
#define _STM32_CLOCKS_H

#if defined STM32F0
#include "clocks-stm32f0.h"
#elif defined STM32F1
#include "clocks-stm32f1.h"
#elif defined STM32F2
#include "clocks-stm32f2.h"
#elif defined STM32F3
#include "clocks-stm32f3.h"
#elif defined STM32F4
#include "clocks-stm32f4.h"
#endif

#endif // _STM32_CLOCKS_H
