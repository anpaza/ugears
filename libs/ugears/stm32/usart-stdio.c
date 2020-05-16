/*
    stdout redirected via RS232
    Copyright (C) 2010 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears.h"
#include "printf.h"
#undef putc

static printf_backend_t usart_backend;

static void usart_backend_putc (printf_backend_t *backend, char c)
{
    USART_TypeDef *usart = (USART_TypeDef *)backend->data;

    // \n -> \r\n
    if (c == '\n')
        usart_putc (usart, '\r');
    usart_putc (usart, c);
}

void usart_printf (USART_TypeDef *usart)
{
    usart_backend.putc = usart_backend_putc;
    usart_backend.data = usart;

    init_printf (&usart_backend);
}
