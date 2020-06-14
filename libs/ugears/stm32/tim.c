/*
    Hardware timer support library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <ugears/ugears.h>

// convert timer base to a 8-bit value to shorten code
#define TIM_ID(tim)		((((uint32_t)tim) >> 10) & 0xff)

// timer features

// clock divisor for dead-time and sampling clock
#define F_CKD			0x00000001
// one-pulse-mode
#define F_OPM			0x00000002
// has repetition counter
#define F_RCR			0x00000004

/*
static uint32_t features (TIM_TypeDef *tim)
{
    uint32_t r = 0;
    uint32_t tid = TIM_ID (tim);

    if (tid != TIM_ID (TIM6) && tid != TIM_ID (TIM7))
        r |= F_CKD;

    if (tid == TIM_ID (TIM1) || tid == TIM_ID (TIM8) ||
        tid == TIM_ID (TIM15) || tid == TIM_ID (TIM16) || tid == TIM_ID (TIM17))
        r |= F_RCR;

    return r;
}
*/
