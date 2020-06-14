/*
    STM32 Backup domain library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <ugears/ugears.h>

#ifdef RTC_TYPE_1

void bkp_save (uint32_t idx, const void *data, uint32_t size)
{
    const uint16_t *d16 = (const uint16_t *)data;
    while (size > 1)
    {
        bkp_set_data (idx, *d16);
        d16++;
        idx++;
        size -= 2;
    }
    if (size)
        bkp_set_data (idx, *(const uint8_t *)d16);
}

void bkp_load (uint32_t idx, void *data, uint32_t size)
{
    uint16_t *d16 = (uint16_t *)data;
    while (size > 1)
    {
        *d16 = bkp_data (idx);
        d16++;
        idx++;
        size -= 2;
    }
    if (size)
        *(uint8_t *)d16 = bkp_data (idx);
}

#endif // RTC_TYPE_1
