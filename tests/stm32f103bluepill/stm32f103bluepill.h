/*
 * STM32VL-DISCOVERY specific functions.
 * Copyright (C) 2014 Andrey Zabolotnyi
 */

#ifndef _STM32F103BLUEPILL_H
#define _STM32F103BLUEPILL_H

#include "ugears.h"
#include "printf.h"

extern void usart1_init ();

extern void led_init ();

extern void systick_init ();

#endif // _STM32F103BLUEPILL_H
