/*
    STM32 Universal Serial Asynchronous Receiver/Transmitter simplified
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _USART_H
#define _USART_H

/**
 * @file usart.h
 *      This small library provides some functions for sending bytes
 *      via one of the USARTs available in the STM32 microcontrollers.
 */

#ifdef __cplusplus
extern "C" {
#endif

// F0 and F3 series uses a little different USART from F1, F2, F4
#if defined STM32F1 || defined STM32F2 || defined STM32F4
#  define USART_TYPE_1
#elif defined STM32F0 || defined STM32F3
#  define USART_TYPE_2
#else
#  define USART_TYPE_NONE
#endif

#ifndef USART_TYPE_NONE

#include "useful.h"

#define __CMP_USART1		99980
#define __CMP_USART2		99981
#define __CMP_USART3		99982
#define __CMP_USART4		99983
#define __CMP_USART5		99984

/// Return the USART instance corresponding to a hardware feature (e.g. USART(DEBUG) -> USART3)
#define USART(x)		JOIN2 (USART, JOIN2 (x, _USART))
/// Check if the USART of a hw feature is same as expected (p = 1, 2 etc)
#define USART_CMP(x,p)		(JOIN2 (__CMP_USART, JOIN2 (x, _USART)) == JOIN2 (__CMP_USART, p))

#ifdef USART_TYPE_2
/// 7 bit characters
#define USART_CHARBITS_7	0x00100000
#endif
/// 8 bit characters
#define USART_CHARBITS_8	0x00000000
/// 9 bit characters
#define USART_CHARBITS_9	0x00200000

/// 1 stop bits
#define USART_STOPBITS_1	0x00000000

#ifdef USART_TYPE_1
/// 0.5 stop bits
#define USART_STOPBITS_0_5	0x00400000
#endif

/// 2 stop bits
#define USART_STOPBITS_2	0x00800000
/// 1.5 stop bits
#define USART_STOPBITS_1_5	0x00c00000
/// The mask to select only stop bits selection from format
#define USART_STOPBITS_MASK	0x00c00000
/// The amount to shift stopbits right to compute USART_CR2_STOP bitflags
#define USART_STOPBITS_RSHIFT	10		// USART_CR2_STOP=0x3000

/// No parity check/generation
#define USART_PARITY_NONE	0x00000000
/// Even parity check/generation
#define USART_PARITY_EVEN	0x02000000
/// Odd parity check/generation
#define USART_PARITY_ODD	0x03000000

/// The mask to select only parity mode selection from format
#define USART_PARITY_MASK	0x03000000
/// The amount to shift the bits right to get USART_CR1_PS & USART_CR1_PCE
#define USART_PARITY_RSHIFT	15		// USART_CR1_PS=0x0200, USART_CR1_PCE=0x0400

/// Use CTS for outbound flow control
#define USART_CTS		0x04000000
/// Signal RTS when input buffer is full
#define USART_RTS		0x08000000

/// Disable receiver
#define USART_NO_RX		0x10000000
/// Disable transmitter
#define USART_NO_TX		0x20000000

// It looks like STM32F1 (undocumentedly) supports OVER8 mode
#ifdef USART_CR1_OVER8
/// Use 8 sub-clocks per UART tick instead of default 16
# define USART_OVER8		0x40000000
#endif

/// Assume lower 2 bits of baud rate are zero
#define USART_BAUD(n)		(n >> 2)
/// The mask to separate baud rate bits from format
#define USART_BAUD_MASK		0x000FFFFF	// max 1048575 which gives up to 4194303 baud

/**
 * Initialize the serial transceiver.
 * It is supposed that the GPIO themselves (clocking, mode, config,
 * alternate mappings) are set up prioir to calling this function.
 * @arg usart
 *      A pointer to the USART register block to be initialized.
 * @arg bus_freq
 *      The clock on the bus used by the USART (AHB1 or AHB2).
 * @arg fmt
 *      A combination of USART_XXX bits defined above to define
 *      the baud rate and data format.
 */
extern void usart_init (USART_TypeDef *usart, uint32_t bus_freq, uint32_t fmt);

/**
 * Redirect printf, putc, puts from libuseful via the passed serial port.
 * This uses the printf routines from libuseful.
 * Very useful for debugging.
 * @arg usart
 *      The USART to redirect stdio through
 */
extern void usart_printf (USART_TypeDef *usart);

/**
 * Send a single byte through the serial port.
 * @arg usart
 *      The USART to send the character through
 * @arg c
 *      The character to send
 */
extern void usart_putc (USART_TypeDef *usart, uint8_t c);

/**
 * Receive a single byte through the serial port.
 * Waits until the character is ready.
 * @arg usart
 *      The USART to read the character from
 * @return
 *      The character read from the USART
 */
extern uint8_t usart_getc (USART_TypeDef *usart);

/**
 * Check if input is ready to be read from the port.
 * @arg usart
 *      The USART to check data readiness in
 * @return
 *      true if there is data in the receive buffer
 */
static inline bool usart_rx_ready (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (usart->SR & USART_SR_RXNE) != 0; }
#elif defined USART_TYPE_2
{ return (usart->ISR & USART_ISR_RXNE) != 0; }
#endif

/**
 * Check if transmission data register is free
 * @arg usart
 *      The USART to check
 * @return
 *      true if transmission data register is free
 */
static inline bool usart_tx_ready (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (usart->SR & USART_SR_TXE) != 0; }
#elif defined USART_TYPE_2
{ return (usart->ISR & USART_ISR_TXE) != 0; }
#endif

/**
 * Check if transmission is complete (both bytes in outgoing FIFO and
 * data register).
 * @arg usart
 *      The USART to check
 * @return
 *      true if transmission is complete
 */
static inline bool usart_tx_complete (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (usart->SR & USART_SR_TC) != 0; }
#elif defined USART_TYPE_2
{ return (usart->ISR & USART_ISR_TC) != 0; }
#endif

/**
 * Return a reference to transmission data register
 * @arg usart
 *      The USART structure
 * @return
 *      a reference to transmission data register
 */
static inline __IO uint16_t *usart_tdr (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (__IO uint16_t *)&usart->DR; }
#elif defined USART_TYPE_2
{ return &usart->TDR; }
#endif

/**
 * Return a reference to receiver data register
 * @arg usart
 *      The USART structure
 * @return
 *      a reference to receiver data register
 */
static inline __IO uint16_t *usart_rdr (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (__IO uint16_t *)&usart->DR; }
#elif defined USART_TYPE_2
{ return &usart->RDR; }
#endif

#ifdef __cplusplus
}
#endif

#endif // USART_TYPE_NONE

#endif // _USART_H
