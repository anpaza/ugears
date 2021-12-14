/*
    One-shot timers
    Copyright (C) 2014,2018 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/ost.h"

void ost32_arm (volatile ost32_t *timer, clock_t clocks)
{
    ost32_t t = clock + clocks;
    if (!t) t = 1;
    *timer = t;
}

bool ost32_expired (volatile ost32_t *timer)
{
    if (!ost_enabled (*timer))
        return true;

    if (((int32_t)(*timer - clock)) > 0)
        return false;

    ost_disable (timer);
    return true;
}

void ost16_arm (volatile ost16_t *timer, clock_t clocks)
{
    ost16_t t = clock + clocks;
    if (t == OST_DISABLED)
        t = OST_DISABLED + 1;
    *timer = t;
}

bool ost16_expired (volatile ost16_t *timer)
{
    if (!ost_enabled (*timer))
        return true;

    if (((int16_t)(*timer - clock)) > 0)
        return false;

    ost_disable (timer);
    return true;
}

clock_t ost32_remaining (ost32_t timer)
{
    if (!ost_enabled (timer))
        return CLOCK_MAX;

    int32_t r = timer - clock;
    return (r <= 0) ? 0 : r;
}

clock_t ost16_remaining (ost16_t timer)
{
    if (!ost_enabled (timer))
        return CLOCK_MAX;

    int16_t r = timer - clock;
    return (r <= 0) ? 0 : r;
}
