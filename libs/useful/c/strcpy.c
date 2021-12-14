/*
    C implementation for strcpy()
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"

char *CLIKE_P (strcpy) (char *dest, const char *src)
{
    char *ret = dest;
    char c;

    do
    {
        c = *src++;
        *dest++ = c;
    } while (c);

    return ret;
}
