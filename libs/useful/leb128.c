/*
    A library of generally useful functions
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <useful/usefun.h>

uint32_t uleb128 (const uint8_t **data)
{
    uint32_t d;
    uint32_t r = 0;
    unsigned shift = 0;

    do
    {
        d = *(*data)++;
        r |= (d & 0x7F) << shift;
        shift += 7;
    } while (d & 0x80);

    return r;
}

int32_t sleb128 (const uint8_t **data)
{
    uint32_t d;
    int32_t r = 0;
    unsigned shift = 0;

    do
    {
        d = *(*data)++;
        r |= (d & 0x7F) << shift;
        shift += 7;
    } while (d & 0x80);

    if ((d & 0x40) && (shift < 32))
        r |= 0xffffffffU << shift;

    return r;
}
