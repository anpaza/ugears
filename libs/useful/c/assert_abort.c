/*
    Simple assert() implementation for microcontrollers
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike.h"

void _assert_abort (const char *msg)
{
    _puts (msg);
    for (;;) DEBUG_BREAK;
}
