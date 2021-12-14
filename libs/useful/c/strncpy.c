/*
    C implementation for strcpy()
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"

char *CLIKE_P (strncpy) (char *dest, const char *src, size_t destlen)
{
    char *ret = dest;
    char *destend = dest + destlen;
    char c;

    while ((dest < destend) && (c = *src) != 0)
    {
        src++;
        *dest++ = c;
    }

    while (dest < destend)
        *dest++ = 0;

    return ret;
}
