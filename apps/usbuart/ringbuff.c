/*
    Simple ring buffer
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ringbuff.h"
#include <useful/clike.h>

unsigned ringbuff_put (ringbuff_t *buff,
                       const uint8_t *data, unsigned size)
{
    unsigned initial_size = size;
    unsigned head = buff->head;
    unsigned tail = buff->tail;
    while (size)
    {
        unsigned free = (tail - 1 - head) & RINGBUFF_MASK;
        unsigned ahead = RINGBUFF_SIZE - head;
        unsigned c = MIN (MIN (free, ahead), size);
        if (c == 0)
            break;

        memcpy (buff->data + head, data, c);
        head = (head + c) & RINGBUFF_MASK;
        data += c;
        size -= c;
    }
    buff->head = head;
    buff->tail = tail;

    return initial_size - size;
}
