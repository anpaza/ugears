/*
    STM32 External Interrupts controller handling
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_EXTI_H
#define _STM32_EXTI_H

/**
 * @file exti.h
 *      This library simplifies management of the EXTernal Interrupts
 *      controller (EXTI) unit.
 */

/// Generate a interrupt
#define _EXTI_INT		0x0001
/// Generate a event
#define _EXTI_EVENT		0x0002
/// Detect rising edge
#define _EXTI_RISING		0x0004
/// Detect falling edge
#define _EXTI_FALLING		0x0008
/// Select port to use for this EXTI line (A, B, C, ...)
#define EXTI_PORT(p)		JOIN2 (GPIO_PORT_,p)

/**
 * A shorter way to create a EXTI_XXX bitmask.
 * @arg p
 *      Port name to use on this line (A, B, C, ...)
 * @arg a1
 *      One of INT, EVENT, RISING, FALLING keywords
 */
#define EXTI_CONF1(p, a1) \
    EXTI_PORT (p) | \
    JOIN2 (_EXTI_, a1)

/**
 * A shorter way to create a EXTI_XXX bitmask.
 * @arg p
 *      Port name to use on this line (A, B, C, ...)
 * @arg a1, a2
 *      One of INT, EVENT, RISING, FALLING keywords
 */
#define EXTI_CONF2(p, a1, a2) \
    EXTI_PORT (p) | \
    JOIN2 (_EXTI_, a1) | JOIN2 (_EXTI_, a2)

/**
 * A shorter way to create a EXTI_XXX bitmask.
 * @arg p
 *      Port name to use on this line (A, B, C, ...)
 * @arg a1, a2, a3
 *      One of INT, EVENT, RISING, FALLING keywords
 */
#define EXTI_CONF3(p, a1, a2, a3) \
    EXTI_PORT (p) | \
    JOIN2 (_EXTI_, a1) | JOIN2 (_EXTI_, a2) | \
    JOIN2 (_EXTI_, a3)

/**
 * A shorter way to create a EXTI_XXX bitmask.
 * @arg p
 *      Port name to use on this line (A, B, C, ...)
 * @arg a1, a2, a3, a4
 *      One of INT, EVENT, RISING, FALLING keywords
 */
#define EXTI_CONF4(p, a1, a2, a3, a4) \
    EXTI_PORT (p) | \
    JOIN2 (_EXTI_, a1) | JOIN2 (_EXTI_, a2) | \
    JOIN2 (_EXTI_, a3) | JOIN2 (_EXTI_, a4)

/**
 * Configure a EXTI line.
 * Note that you have to configure NVIC separately if you want
 * to handle a IRQ from this EXTI line via the nvic_setup().
 * @arg exti
 *      EXTI line number (from 0, count depends on chip)
 * @arg conf
 *      A combination of EXTI_XXX flags (possibly created
 *      with the EXTI_CONFX() macros)
 */
extern void exti_config (uint32_t exti, uint32_t conf);

/**
 * Trigger EXTI line by software.
 * @arg exti
 *      EXTI line number (from 0, count depends on chip)
 */
INLINE_ALWAYS void exti_trigger (uint32_t exti)
{ EXTI->SWIER |= 1 << exti; }

/**
 * Check if given EXTI line is pending an interrupt or event.
 * @arg exti
 *      EXTI line number (from 0, count depends on chip)
 * @return
 *      Non-zero if a event or interrupt is pending
 */
INLINE_ALWAYS uint32_t exti_pending (uint32_t exti)
{ return EXTI->PR & (1 << exti); }

/**
 * Check if several EXTI lines are pending a interrupt/event
 * @arg mask
 *      A bitwise OR of event masks
 * @return
 *      Non-zero if a event or interrupt is pending
 */
INLINE_ALWAYS uint32_t exti_pending_mask (uint32_t mask)
{ return EXTI->PR & mask; }

/**
 * Clear the EXTI pending flag.
 * @arg exti
 *      EXTI line number (from 0, count depends on chip)
 */
INLINE_ALWAYS void exti_clear (uint32_t exti)
{ EXTI->PR |= (1 << exti); }

/**
 * Clear several EXTI pending events/interrupts
 * @arg mask
 *      A bitwise OR of events to clear
 */
INLINE_ALWAYS void exti_clear_mask (uint32_t mask)
{ EXTI->PR |= mask; }

/**
 * Clear all EXTI pending flags at once.
 */
INLINE_ALWAYS void exti_clear_all ()
{ EXTI->PR = 0xffffffff; }

#endif // _STM32_EXTI_H
