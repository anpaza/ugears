/*
 * STM32VL-DISCOVERY specific functions.
 * Copyright (C) 2014 Andrey Zabolotnyi
 */

#ifndef _HW_H
#define _HW_H

#include <ugears/ugears.h>
#include <useful/clike.h>

extern void serial_init ();

extern void led_init ();

extern void systick_init ();

#endif // _HW_H
