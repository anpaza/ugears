/*
    Useful small inline functions coded in ARM assembler
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _USEFUL_ARM_H
#define _USEFUL_ARM_H

// Keep IDEs happy
#include "useful.h"

#ifndef _ATOMIC_IRQ_STATE
#define _ATOMIC_IRQ_STATE

#include <cmsis/cmsis_compiler.h>

/// A datatype to hold the current IRQ enabled state
typedef uint32_t atomic_irq_state_t;

/// A value meaning "IRQs are enabled"
#define ATOMIC_IRQ_STATE_ENA 0
/// A value meaning "IRQs are disabled"
#define ATOMIC_IRQ_STATE_DIS 1

/**
 * Get current IRQ enabled state
 * @return a value of atomic_irq_state_t type
 */
INLINE_ALWAYS atomic_irq_state_t atomic_irq_get_state ()
{ return __get_PRIMASK (); }

/**
 * Set IRQ enabled state according to passed value.
 * @param state The new IRQ enabled state
 */
INLINE_ALWAYS void atomic_irq_set_state (atomic_irq_state_t state)
{ if (state & 1) __disable_irq (); else __enable_irq (); }

#endif // _ATOMIC_IRQ_STATE

#endif // _USEFUL_ARM_H
