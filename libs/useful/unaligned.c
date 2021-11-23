/*
    Unaligned memory access
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/usefun.h"

uint16_t uget16le (const void *data)
{
    const uint8_t *tmp = (const uint8_t *)data;
    return ((uint32_t)tmp [0]) | (((uint32_t)tmp [1]) << 8);
}

uint16_t uget16be (const void *data)
{
    const uint8_t *tmp = (const uint8_t *)data;
    return ((uint32_t)tmp [1]) | (((uint32_t)tmp [0]) << 8);
}

uint32_t uget32le (const void *data)
{
    const uint8_t *tmp = (const uint8_t *)data;
    return ((uint32_t)tmp [0]) | (((uint32_t)tmp [1]) << 8) |
           (((uint32_t)tmp [2]) << 16) | (((uint32_t)tmp [3]) << 24);
}

uint32_t uget32be (const void *data)
{
    const uint8_t *tmp = (const uint8_t *)data;
    return ((uint32_t)tmp [3]) | (((uint32_t)tmp [2]) << 8) |
           (((uint32_t)tmp [1]) << 16) | (((uint32_t)tmp [0]) << 24);
}
