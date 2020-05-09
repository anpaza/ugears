/*
    STM32 GPIO helpers library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears.h"
#include "fpmath.h"

extern bool iwdg_init (uint32_t reload_ms
#ifdef IWDG_HAS_WINDOW
    , uint32_t window_ms
#endif
    )
{
#ifdef IWDG_HAS_WINDOW
    // Sanity check
    if (window_ms > reload_ms)
        return false;
#endif

    // calculate the optimum prescaler
    uint32_t clocks = umul_h32 (reload_ms * LSI_VALUE, FxPu32 (0.001));
    uint32_t prescaler;
    for (prescaler = 0; prescaler < 7; prescaler++)
        if (((4 * 0xfffU) << prescaler) > clocks)
            break;

    // max valid prescaler is 6 (256)
    if (prescaler > 6)
        return false;

    IWDG->KR = IWDG_KEY_START;

    // bring clocks to range 0..FFF
    clocks >>= (prescaler + 2);

#ifdef IWDG_HAS_WINDOW
    uint32_t win_clocks = umul_h32 (window_ms * LSI_VALUE, FxPu32 (0.001)) >> (prescaler + 2);
#endif

    // unprotect the IWDG registers
    IWDG->KR = IWDG_KEY_INIT;

    while (IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU
#ifdef IWDG_HAS_WINDOW
            | IWDG_SR_WVU
#endif
        ))
        ;
    IWDG->PR = prescaler;
    IWDG->RLR = clocks;
#ifdef IWDG_HAS_WINDOW
    IWDG->WINR = win_clocks;
#endif

    IWDG->KR = IWDG_KEY_RELOAD;

    return true;
}
