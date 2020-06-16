/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi
    printf() through USB CDC ACM

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "cdcacm-priv.h"
#include <useful/printf.h>

// Must be power of two!
#define BUFF_SIZE 128
#define BUFF_MASK (BUFF_SIZE - 1)

static struct cdcacm_printf_backend_t
{
    printf_backend_t be;
    char buff [BUFF_SIZE];
    uint8_t buff_head, buff_tail;
    uint8_t buff_inflight;
} backend;

void uca_transmitted ()
{
    backend.buff_tail = (backend.buff_tail + backend.buff_inflight) & BUFF_MASK;
    backend.buff_inflight = 0;

    if (backend.buff_head != backend.buff_tail)
    {
        backend.buff_inflight = ((backend.buff_tail < backend.buff_head) ?
            backend.buff_head : BUFF_SIZE) - backend.buff_tail;
        if (!uca_transmit (backend.buff + backend.buff_tail, backend.buff_inflight))
            backend.buff_inflight = 0;
    }
}

static void uca_putc (char c)
{
    if (backend.buff_head != ((backend.buff_tail - 1) & BUFF_MASK))
    {
        backend.buff [backend.buff_head] = c;
        backend.buff_head = (backend.buff_head + 1) & BUFF_MASK;
    }

    // If transmitter is idle, send now
    if ((backend.buff_inflight == 0)
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
    backend.be.putc = uca_backend_putc;

    init_printf (&backend.be);
}
