/*
    Atomic code execution
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _ATOMIC_H
#define _ATOMIC_H

/**
 * @file atomic.h
 *      Atomically and Non-Atomically Executed Code Blocks
 *
 * The idea for these functions were blatanly stolen from AVR-libc. They will allow
 * you to block interrupts (PRIMASK) for some sections of code, with
 * guaranteed exit action, not matter how you leave the code section.
 * Additionaly, since this is implemented internally with a 'for' loop,
 * you can use the 'break' operator as an alias to 'goto atomic block end'.
 */

#include "useful.h"

/**
 * Creates a block of code that is executed atomically.
 * Upon entering the block the PRIMASK bit in PRIMASK register is set,
 * blocking any exceptions and interrupts except NMI.
 *
 * When leaving the block exceptions and interrupts are either restored 
 * to the state they were before entering the block (if ATOMIC_RESTORESTATE
 * keyword is used as argument to ATOMIC_BLOCK), or unconditionally enabled,
 * if ATOMIC_FORCEON is used as argument to ATOMIC_BLOCK.
 *
 * Since this is effectively a 'for' loop you may use 'break' to jump
 * to the end of the block, if needed.
 *
 * Usage example:
 * @verbatim
 * ATOMIC_BLOCK (ATOMIC_FORCEON)
 * {
 *      // Access var without possible interference from a interrupt handler
 *      if (flags & 0x100)
 *      {
 *          flag_8_count++;
 *          flags &= ~0x100;
 *      }
 *      if (flags & 0x200)
 *          break;
 *      if (flags & 0x400)
 *      {
 *          flag_10_count++;
 *          flags &= 0x400;
 *      }
 * }
 * @endverbatim
 *
 * @param type ATOMIC_RESTORESTATE or ATOMIC_FORCEON
 */
#define ATOMIC_BLOCK(type) \
    for (type | (__disable_irq (), 0), __pass = 1; __pass ; __pass = 0)

/**
 * Creates a block of code that is executed non-atomically.
 * Upon entering the block the PRIMASK bit in PRIMASK register is cleared,
 * allowing any exceptions and interrupts.
 *
 * When leaving the block exceptions and interrupts are either restored 
 * to the state they were before entering the block (if NONATOMIC_RESTORESTATE
 * keyword is used as argument to NONATOMIC_BLOCK), or unconditionally disabled,
 * if NONATOMIC_FORCEOFF is used as argument to NONATOMIC_BLOCK.
 *
 * @param type NONATOMIC_RESTORESTATE or NONATOMIC_FORCEOFF
 */
#define NONATOMIC_BLOCK(type) \
    for (type | (__enable_irq (), 0), __pass = 1; __pass ; __pass = 0)

static __inline void __set_primask (uint32_t *val)
{
    if (*val & 1)
        __disable_irq ();
    else
        __enable_irq ();
}

#define ATOMIC_RESTORESTATE \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = __get_PRIMASK ()
#define ATOMIC_FORCEON \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = 0
#define NONATOMIC_RESTORESTATE \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = __get_PRIMASK ()
#define NONATOMIC_FORCEOFF \
    uint32_t primask_save __attribute__((__cleanup__(__set_primask))) = 1

#endif // _ATOMIC_H
