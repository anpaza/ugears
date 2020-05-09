/*
    serial I/O simplified
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears.h"

#ifdef USART_TYPE_2
# ifndef USART_CR1_M1
#   define  USART_CR1_M1                     ((uint32_t)0x10000000)            /*!< Word length */
# endif
#endif

// To avoid #ifdef mess we'll just set those to 0 if MCU doesn't have them
#ifndef USART_ICR_WUCF
#  define USART_ICR_WUCF 0
#endif
#ifndef USART_ICR_EOBCF
#  define USART_ICR_EOBCF 0
#endif
#ifndef USART_ICR_LBDCF
#  define USART_ICR_LBDCF 0
#endif

void usart_init (USART_TypeDef *usart, uint32_t bus_freq, uint32_t fmt)
{
    uint32_t tmp;

    // flow control bits
    tmp = usart->CR3 & ~(USART_CR3_CTSE | USART_CR3_RTSE);

    if (fmt & USART_CTS)
        tmp |= USART_CR3_CTSE;
    if (fmt & USART_RTS)
        tmp |= USART_CR3_RTSE;

    usart->CR3 = tmp;

    // stop bits
    usart->CR2 = (usart->CR2 & ~USART_CR2_STOP) |
        ((fmt & USART_STOPBITS_MASK) >> USART_STOPBITS_RSHIFT);

    // enable USART, set parity mode
    tmp = (usart->CR1 &
        ~(USART_CR1_UE | USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_RE | USART_CR1_TE
#ifdef USART_TYPE_2
          | USART_CR1_M1
#endif
         )) | ((fmt & USART_PARITY_MASK) >> USART_PARITY_RSHIFT);

    // Enable receiver & transmitter, if not prohibited
    if (!(fmt & USART_NO_RX))
        tmp |= USART_CR1_RE;
    if (!(fmt & USART_NO_TX))
        tmp |= USART_CR1_TE;

    // char bits
    if (fmt & USART_CHARBITS_9)
        tmp |= USART_CR1_M;
#ifdef USART_TYPE_2
    else if (fmt & USART_CHARBITS_7)
        tmp |= USART_CR1_M1;
#endif

#ifdef USART_OVER8
    if (fmt & USART_OVER8)
        tmp |= USART_CR1_OVER8;
#endif

    usart->CR1 = tmp;

    // Finally, set up the baud rate
    // (fmt contains the baud rate without the lower 2 bits)
    fmt = (fmt & USART_BAUD_MASK) << 2;

    // Won't check for zero baud rate, we aren't idiots, are we?
#ifdef USART_OVER8
    // It looks like STM32F1 (undocumentedly) supports OVER8 mode
    if (tmp & USART_CR1_OVER8)
    {
        fmt = (bus_freq * 2 + fmt / 2) / fmt;
        fmt = (fmt & 0xfffffff0) | ((fmt & 0x0f) >> 1);
    }
    else
#endif
        fmt = (bus_freq + fmt / 2) / fmt;

    usart->BRR = fmt;

    // Clear the USART status bits
#if defined USART_TYPE_1
    usart->SR &= ~(USART_SR_CTS | USART_SR_LBD | USART_SR_RXNE);
#elif defined USART_TYPE_2
    usart->RQR |= USART_RQR_RXFRQ;
    usart->ICR |= USART_ICR_WUCF | USART_ICR_EOBCF | USART_ICR_LBDCF |
        USART_ICR_CMCF | USART_ICR_RTOCF | USART_ICR_CTSCF |
        USART_ICR_TCCF | USART_ICR_IDLECF | USART_ICR_ORECF |
        USART_ICR_NCF |  USART_ICR_FECF | USART_ICR_PECF;
#endif

    // Finally, enable the USART
    usart->CR1 |= USART_CR1_UE;
}

uint8_t usart_getc (USART_TypeDef *usart)
{
    while (!usart_rx_ready (usart))
        ;
    return *usart_rdr (usart);
}

void usart_putc (USART_TypeDef *usart, uint8_t c)
{
    // Wait till transmission register empty
    while (!usart_tx_ready (usart))
        ;
    *usart_tdr (usart) = c;
}
