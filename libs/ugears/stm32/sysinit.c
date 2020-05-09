/*
    System initialization
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

/**
 * System initialization process on STM32 goes as following:
 *
 * @li On boot, CPU jumps to Reset_Handler, which is located in
 *     cmsis/stm32/startup_[MCU-type].s.
 * @li Reset_Handler sets stack pointer, copies static data to RAM
 *     and fills uninitialized data with zeros.
 * @li Then it calls SystemInit function.
 * @li SystemInit initializes system clock by calling clock_init ().
 * @li SystemInit invokes a special weak function early_init (),
 *     which can be overriden by user, if desired.
 * @li Then, Reset_Handler invokes static object constructors
 * @li And, finally, it calls main()
 *
 * If desired, library user may intercept execution at very early stages
 * by defining an early_init function:
 *
 * @verbatim
 * void early_init () { ... }
 * @endverbatim
 *
 * Please note that early_init() is invoked before any static object constructors
 */

extern void clock_init ();

/*
 * User may override this function, if desired.
 *
 * The difference with __attribute__((constructor)) functions is that this is
 * guaranteedly called before all other constructors, while the order of constructor
 * invocation is unknown. Sometimes this matters.
 */
void early_init () {}

// Called from startup code written in assembler
void SystemInit ()
{
    clock_init ();
    early_init ();
}
