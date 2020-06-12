/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi
    printf() through USB CDC ACM

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "cdcacm-priv.h"
#include "printf.h"
#include "atomic.h"

#undef putc

// Must be power of two!
#define BUFF_SIZE 128
#define BUFF_MASK (BUFF_SIZE - 1)

static printf_backend_t backend;
static char buff [BUFF_SIZE];
static uint8_t buff_head = 0, buff_tail = 0;
static uint8_t buff_inflight = 0;

void uca_transmitted ()
{
    buff_tail = (buff_tail + buff_inflight) & BUFF_MASK;
    buff_inflight = 0;

    if (buff_head != buff_tail)
    {
        buff_inflight = ((buff_tail < buff_head) ? buff_head : BUFF_SIZE) - buff_tail;
        if (!uca_transmit (buff + buff_tail, buff_inflight))
            buff_inflight = 0;
    }
}

static void uca_putc (char c)
{
    if (buff_head != ((buff_tail - 1) & BUFF_MASK))
    {
        buff [buff_head] = c;
        buff_head = (buff_head + 1) & BUFF_MASK;
    }

    // If transmitter is idle, send now
    if ((buff_inflight == 0)
    #ifdef USB_CDC_LINE_CONTROL
        && (uca_line_state & USB_CDC_LINE_STATE_DTR)
    #endif
        )
        // prevent concurrent uca_transmitted invocation from IRQ handler
        ATOMIC_BLOCK (RESTORE)
        {
            uca_transmitted ();
        }
}

static void uca_backend_putc (printf_backend_t *backend, char c)
{
    (void)backend;

    // \n -> \r\n
    if (c == '\n')
        uca_putc ('\r');
    uca_putc (c);
}

void uca_printf ()
{
    backend.putc = uca_backend_putc;

    init_printf (&backend);
}
