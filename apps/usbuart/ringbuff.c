/*
    Simple ring buffer
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ringbuff.h"
#include "usefun.h"

unsigned ringbuff_put (ringbuff_t *buff,
                       const uint8_t *data, unsigned size)
{
    unsigned initial_size = size;
    while (size)
    {
        unsigned c = (buff->tail <= buff->head) ?
            (RINGBUFF_SIZE - buff->head) :
            ((buff->tail - 1 - buff->head) & RINGBUFF_MASK);
        if (c > size)
            c = size;
        if (c == 0)
            break;

        memcpy (buff->data + buff->head, data, c);
        buff->head = (buff->head + c) & RINGBUFF_MASK;
        size -= c;
    }

    return initial_size - size;
}
