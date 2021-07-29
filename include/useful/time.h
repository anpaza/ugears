/*
    System time functions
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _TIME_H
#define _TIME_H

#include "useful.h"

/** System clock type */
typedef uint32_t clock_t;

/** The system clock variable */
EXTERN_C volatile clock_t clock;

/* CLOCKS_PER_SEC is expected to be defined in hardware.h */
#ifndef CLOCKS_PER_SEC
#  define CLOCKS_PER_SEC	32
#endif

/** Seconds to clocks */
#define CLOCKS(s)		((uint32_t)((s) * CLOCKS_PER_SEC))

#endif // _TIME_H
