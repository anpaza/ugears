/*
    STM32 Nested Vector Interrupt Controller library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _NVIC_H
#define _NVIC_H

/**
 * @file nvic.h
 *      A set of routines to manipulate the Nested Vector
 *      Interrupt Controller (NVIC).
 *
 * The following macros can be defined in your HARDWARE_H
 * to make your code more portable:
 *
 * @li HWFN_IRQ_PRIO defines the IRQ priority corresponding to
 *      this hardware feature. Used by IRQ_PRIO(HWFN) macro.
 */

#include "useful.h"

/// Return the IRQ priority corresponding to this hardware feature
#define IRQ_PRIO(x)		JOIN2 (x, _IRQ_PRIO)

#if defined CORTEX_M0 || defined CORTEX_M1 || defined CORTEX_M0PLUS
#  define NVIC_TYPE_1
#else
#  define NVIC_TYPE_2
#endif

/**
 * Set up the interrupt controller to handle the given IRQ.
 * @arg irq
 *      IRQ number (one of the xxx_IRQn constants). If zero or positive,
 *      this sets up one of the peripherial interrupts, if negative
 *      this sets up one of the Cortex-M system interrupts.
 * @arg priority
 *      IRQ priority (0-255). Note that the actual number of distinct
 *      priorities differs from one core to another. Only __NVIC_PRIO_BITS
 *      top bits of the priority are used (usually 4 or 2).
 */
extern void nvic_setup (int irq, uint8_t priority);

/**
 * Set IRQ priority.
 * @arg irq
 *      IRQ number (one of the xxx_IRQn constants). If zero or positive,
 *      this sets up one of the peripherial interrupts, if negative
 *      this sets up one of the Cortex-M system interrupts.
 * @arg priority
 *      IRQ priority (0-255). Note that the actual number of distinct
 *      priorities differs from one core to another. Only __NVIC_PRIO_BITS
 *      top bits of the priority are used (usually 4 or 2).
 */
static inline void nvic_set_priority (int irq, uint8_t priority)
{
    priority &= ~(0xff >> __NVIC_PRIO_BITS);

    if (irq < 0)
        /* set Priority for Cortex-M  System Interrupts */
        SCB->SHP [(irq & 0xF) - 4] = priority;
    else
        /* set Priority for device specific Interrupts  */
        NVIC->IP [irq] = priority;
}

/**
 * Just enable the given IRQ. Also don't forget to __enable_irq()
 * to allow interrupt processing in general.
 * @arg irq
 *      IRQ number (0-67, one of the >=0 xxx_IRQn constants).
 */
static inline void nvic_enable (unsigned irq)
{ NVIC->ISER [irq / 32] |= 1 << (irq & 31); }

/**
 * Disable the given IRQ.
 * @arg irq
 *      IRQ number (0-67, one of the >=0 xxx_IRQn constants).
 */
static inline void nvic_disable (unsigned irq)
{ NVIC->ICER [irq / 32] |= 1 << (irq & 31); }

/**
 * Set the pending flag on given IRQ.
 * The processor will invoke the respective IRQ handler
 * as soon as possible, if IRQ is enabled.
 * @arg irq
 *      IRQ number (0-67, one of the >=0 xxx_IRQn constants).
 */
static inline void nvic_set_pending (unsigned irq)
{ NVIC->ISPR [irq / 32] |= 1 << (irq & 31); }

/**
 * Clear the pending flag on given IRQ.
 * This is useful for edge-triggered interrupts; to acknowledge
 * level-triggered interrupts you must clear the interrupt pending
 * status directly in the hardware connected to this IRQ.
 * @arg irq
 *      IRQ number (0-67, one of the >=0 xxx_IRQn constants).
 */
static inline void nvic_clear_pending (unsigned irq)
{ NVIC->ICPR [irq / 32] |= 1 << (irq & 31); }

#ifdef NVIC_TYPE_2

/**
 * Check if given IRQ is active.
 * @arg irq
 *      IRQ number (0-67, one of the >=0 xxx_IRQn constants).
 */
static inline unsigned nvic_is_active (unsigned irq)
{ return NVIC->IABR [irq / 32] & (1 << (irq & 31)); }

/**
 * Software trigger of given IRQ.
 * @arg irq
 *      IRQ number (0-67, one of the >=0 xxx_IRQn constants).
 */
static inline void nvic_irq_trigger (unsigned irq)
{ NVIC->STIR = irq; }

#endif

/**
 * Reset the CPU
 */
extern void nvic_system_reset ();

#endif // _NVIC_H
