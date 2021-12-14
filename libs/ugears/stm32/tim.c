/*
    Hardware timer support library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears/ugears.h"

bool tim_init (TIM_TypeDef *tim, uint16_t prescaler, uint16_t period,
               uint32_t mode)
{
    tim_stop (tim);
    tim_set_prescaler (tim, prescaler);
    tim_set_period (tim, period);
    tim_set_direction (tim, (mode & TIM_CONF_DIR) == TIM_CONF_DIR_UP);
    tim_reset_counter (tim);

    switch (mode & TIM_CONF_MODE)
    {
        case TIM_CONF_MODE_REPEATED: tim_set_repeated (tim); break;
        case TIM_CONF_MODE_ONESHOT:  tim_set_oneshot (tim);  break;
    }

    tim_set_irq (tim, (mode & TIM_CONF_IRQ) != 0);

    return true;
}
