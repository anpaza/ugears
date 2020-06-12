/*
    Simple ring buffer
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _RINGBUFF_H
#define _RINGBUFF_H

#include "useful.h"

/**
 * @file ringbuff.h
 *      Simple FIFO ring buffers with fixed size.
 *
 * Data is put after the 'head' pointer.
 *
 * Stored data is extracted by the 'tail' pointer.
 *
 * You may add data to buffer and extract data from buffer
 * without using locks. But if 'data adder' or 'data extractor'
 * has a chance to be interrupted by a IRQ that will also invoke
 * same operation, lock the buffer with ATOMIC_BLOCK.
 *
 * So, the code pattern is:
 * @li Place data into (or extract data from) ring buffer without locking,
 *      if you do it from one context only (no matter IRQ or main thread).
 * @li Protect data placement (or extraction) from main thread with
 *      ATOMIC_BLOCK if there's IRQ code that does the same.
 *      The IRQ code doesn't need locking as nobody may interrupt it.
 */

// Must be power of two!
enum
{
    RINGBUFF_SIZE = 256,
    RINGBUFF_MASK = RINGBUFF_SIZE - 1,
};

typedef struct
{
    // Points to free space
    unsigned head;
    // Points to next buffered byte
    unsigned tail;
    // Number of bytes starting from 'tail' that are currently consumed
    unsigned inflight;

    uint8_t data [RINGBUFF_SIZE];
} ringbuff_t;

/**
 * Initialize the ring buffer to empty state
 *
 * @param buff A pointer to the ring buffer
 */
static inline void ringbuff_init (ringbuff_t *buff)
{
    buff->head = buff->tail = buff->inflight = 0;
}

/**
 * Check if ring buffer is empty.
 *
 * @param buff A pointer to the ring buffer
 * @return true if buffer is empty
 */
static inline bool ringbuff_empty (ringbuff_t *buff)
{
    return buff->head == buff->tail;
}

/**
 * Return the available space in the buffer
 *
 * @param buff A pointer to the ring buffer
 */
static inline unsigned ringbuff_free (ringbuff_t *buff)
{
    return (buff->tail - 1 - buff->head) & RINGBUFF_MASK;
}

/**
 * Put data in ring buffer, as many as free space permits.
 * This modifies only 'head' and can be invoked unprotected
 * from concurrent filling ringbuff with data.
 *
 * @param buff Pointer to the ring buffer
 * @param data Pointer to data
 * @param size Data size
 * @return Number of data bytes actually copied to ring buffer
 */
extern unsigned ringbuff_put (ringbuff_t *buff,
                              const uint8_t *data, unsigned size);

/**
 * Acknowledge that some bytes from the buffer has been consumed.
 *
 * @param buff Pointer to the ring buffer
 * @param size The size of consumed data
 */
static inline void ringbuff_ack (ringbuff_t *buff, unsigned size)
{
    buff->tail = (buff->tail + size) & RINGBUFF_MASK;
}

/**
 * Acknowledge that 'inflight' data has been consumed.
 *
 * @param buff Pointer to the ring buffer
 */
static inline void ringbuff_ack_inflight (ringbuff_t *buff)
{
    ringbuff_ack (buff, buff->inflight);
    buff->inflight = 0;
}

/**
 * Returns pointer to next data in buffer and the size of continuous data.
 *
 * @param buff Pointer to the ring buffer
 * @param data Address of a pointer to receive the address of data in buffer
 * @return Size of contiguous data pointed by @a data pointer
 */
static inline unsigned ringbuff_next_data (ringbuff_t *buff, void **data)
{
    unsigned head = buff->head;
    unsigned tail = buff->tail;
    *data = buff->data + tail;
    return ((tail <= head) ? head : RINGBUFF_SIZE) - tail;
}

/**
 * This will return the size of contiguous free space in ring buffer,
 * and a pointer to that space.
 *
 * @param buff Pointer to the ring buffer
 * @param free Address of a pointer to receive the address of free space in buffer
 * @return Size of the free area pointed by @a free
 */
static inline unsigned ringbuff_next_free (ringbuff_t *buff, void **free)
{
    unsigned head = buff->head;
    unsigned tail = buff->tail;
    *free = buff->data + head;
    return MIN ((tail - 1 - head) & RINGBUFF_MASK, RINGBUFF_SIZE - head);
}

/**
 * Account for bytes that were inserted into the buffer not with
 * ringbuff_put(). First call ringbuff_next_free(), then fill data
 * at the address returned, and finally call ringbuff_fill().
 *
 * @param buff Pointer to the ring buffer
 * @param size Size of data inserted at buffer head
 */
static inline void ringbuff_fill (ringbuff_t *buff, unsigned size)
{
    buff->head = (buff->head + size) & RINGBUFF_MASK;
}

#endif /* _RINGBUFF_H */
