/*
    stdout redirected via RS232
    Copyright (C) 2010 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears/ugears.h"
#include <useful/printf.h>
#undef putc

static struct usart_backend_t
{
    printf_backend_t be;
    USART_TypeDef *usart;
} usart_stdout;

static void usart_backend_putc (printf_backend_t *backend, char c)
{
    struct usart_backend_t *self = CONTAINER_OF (backend, struct usart_backend_t, be);

    // \n -> \r\n
    if (c == '\n')
        usart_putc (self->usart, '\r');
    usart_putc (self->usart, c);
}

void usart_printf (USART_TypeDef *usart)
{
    usart_stdout.be.putch = usart_backend_putc;
    usart_stdout.usart = usart;

    init_printf (&usart_stdout.be);
}
