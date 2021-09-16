/*
    STM32 DMA library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <ugears/ugears.h>

#ifdef DMA1_BASE
#include "dmafun.h"
#define _DMA_NUM 1
#include "dmafun.h"
#endif

#ifdef DMA2_BASE
#define _DMA_NUM 2
#include "dmafun.h"
#endif
