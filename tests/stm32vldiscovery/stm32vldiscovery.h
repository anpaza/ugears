/*
 * STM32VL-DISCOVERY specific functions.
 * Copyright (C) 2014 Andrey Zabolotnyi
 */

#ifndef __STM32VL_DISCOVERY_H__
#define __STM32VL_DISCOVERY_H__

#include "ugears.h"
#include "time.h"
#include "printf.h"

extern void usart1_init ();

extern void led_init ();

extern void systick_init ();

#endif // __STM32VL_DISCOVERY_H__
