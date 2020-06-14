/*
    STM32 Nested Vector Interrupt Controller library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <ugears/ugears.h>

void nvic_setup (int irq, uint8_t priority)
{
    nvic_set_priority (irq, priority);
    if (irq >= 0)
        nvic_enable (irq);
}

void nvic_system_reset ()
{
    /* Ensure all outstanding memory accesses included
       buffered write are completed before reset */
    __DSB ();

    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) |
#ifdef SCB_AIRCR_PRIGROUP_Msk
                   /* Keep priority group unchanged */
                   (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
#endif
                   SCB_AIRCR_SYSRESETREQ_Msk);

    /* Ensure completion of memory access */
    __DSB ();

    /* wait until reset */
    for (;;) ;
}
