/*
 * STM32VL-DISCOVERY specific functions.
 * Copyright (C) 2014 Andrey Zabolotnyi
 */

#ifndef _HW_H
#define _HW_H

#include <ugears/ugears.h>
#include <useful/printf.h>
#include <useful/usefun.h>

extern void serial_init ();

extern void led_init ();

extern void systick_init ();

/* Normally, a program would set up everything needed to handle interrupt events
 * and then lock in a closed loop, waiting for interrupts using the WFI CPU opcode.
 * This has the benefit of lower power consumption and cleaner program design.
 * However, doing WFI in a tight loop seems to impact the ability to debug the
 * program via the SWD interface. So, if compiled in debug mode we'll just invoke
 * a NOP in a closed loop, and in release mode will invoke WFI as designed.
 */
#ifdef __DEBUG__
#  define WFI() __asm ("nop");
#else
#  define WFI() __WFI ();
#endif

#endif // _HW_H
