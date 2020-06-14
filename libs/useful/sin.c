/*
    A library of generally useful functions
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <useful/usefun.h>

static const uint8_t sin8_table [64] =
{
      0,   6,  12,  18,  25,  31,  37,  43,  49,  56,  62,  68,  74,  80,  86,  92,
     97, 103, 109, 115, 120, 126, 131, 136, 142, 147, 152, 157, 162, 167, 171, 176,
    181, 185, 189, 193, 197, 201, 205, 209, 212, 216, 219, 222, 225, 228, 231, 234,
    236, 238, 241, 243, 244, 246, 248, 249, 251, 252, 253, 254, 254, 255, 255, 255
};

int fp_sin_8 (uint8_t angle)
{
    int r;

    if ((angle & 0x7F) == 0x40)
        r = 256;
    else if (angle & 0x40)
        r = sin8_table [0x40 - (angle & 0x3F)];
    else
        r = sin8_table [angle & 0x3F];

    if (angle & 0x80)
        r = -r;

    return r;
}
