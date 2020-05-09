/*
    16-bit checksum computation as in IP header
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful.h"

uint32_t ip_crc_block (uint32_t sum, const void *data, unsigned len)
{
    while (len > 1)
    {
        sum += GET_UINT16_BE (data, 0);
        data = ((uint16_t *)data) + 1;
        len -= 2;
    }

    if (len > 0)
        sum += *(uint8_t *)data;

    return sum;
}

uint16_t ip_crc_fin (uint32_t sum)
{
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return UINT16_BE (~sum);
}
