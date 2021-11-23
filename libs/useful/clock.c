/*
    The global clock variable
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/time.h"

/**
 * This variable is usually incremented in SysTick handler
 * CLOCKS_PER_SEC times per second.
 */
volatile clock_t clock;
