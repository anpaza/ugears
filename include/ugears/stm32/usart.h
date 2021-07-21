/*
    STM32 Universal Serial Asynchronous Receiver/Transmitter simplified
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_USART_H
#define _STM32_USART_H

#include "cmsis.h"

/**
 * @file usart.h
 *      This small library provides some functions for sending bytes
 *      via one of the USARTs available in the STM32 microcontrollers.
 *
 * The following macros are expected to be defined in your HARDWARE_H
 * in order to use USART functionality:
 *
 * @li HWFN_USART_NUM - defines the index of USART peripherial used for
 *      hardware feature HWFN (1, 2, ...).
 * @li HWFN_USART_IRQ_PRIO - defines the IRQ priority for USART IRQ used
 *      for hardware feature HWFN (0..255).
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

#include <useful/useful.h>
#include "rcc.h"

// ------ // USART macros // ----- //

#define __CMP_USART1		99980
#define __CMP_USART2		99981
#define __CMP_USART3		99982
#define __CMP_USART4		99983
#define __CMP_USART5		99984

/// Return the number of USART hardware feature
#define USART_NUM(x)		JOIN2 (x, _USART_NUM)
/// Return the USART instance corresponding to a hardware feature (e.g. USART(DEBUG) -> USART3)
#define USART(x)		JOIN2 (USART, USART_NUM (x))
/// Check if the USART of a hw feature has a specific number (p = 1, 2 etc)
#define USART_CMP(x,p)		(JOIN2 (__CMP_USART, USART_NUM (x)) == JOIN2 (__CMP_USART, p))
/// Return the IRQ number corresponding to USART
#define USART_IRQ(x)		JOIN3 (USART, USART_NUM (x), _IRQn)
/// Get the IRQ priority corresponding to a specific USART hardware feature
#define USART_IRQ_PRIO(x)	JOIN2 (x, _USART_IRQ_PRIO)
/// Compose the USART IRQ handler name
#define USART_IRQ_HANDLER(x)	JOIN3 (USART, USART_NUM (x), _IRQHandler)

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
#define USART_STOPBITS_BIT	(22 - USART_CR2_STOP_Pos)

/// No parity check/generation
#define USART_PARITY_NONE	0x00000000
/// Even parity check/generation
#define USART_PARITY_EVEN	0x02000000
/// Odd parity check/generation
#define USART_PARITY_ODD	0x03000000

/// The mask to select only parity mode selection from format
#define USART_PARITY_MASK	0x03000000
/// The amount to shift the bits right to get USART_CR1_PS & USART_CR1_PCE
#define USART_PARITY_BIT	(24 - USART_CR1_PS_Pos)

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

// Default setup for serial ports: 115200 8-N-1
#define USART_DEFAULT_SETUP	(USART_BAUD (115200) | USART_CHARBITS_8 | USART_PARITY_NONE | USART_STOPBITS_1)

/**
 * Initialize the serial transceiver.
 * It is supposed that the GPIO themselves (clocking, mode, config,
 * alternate mappings) are set up prioir to calling this function.
 * @param usart A pointer to the USART register block to be initialized.
 * @param bus_freq The clock on the bus used by the USART (AHB1 or AHB2).
 * @param fmt A combination of USART_XXX bits defined above to define
 *      the baud rate and data format.
 */
extern void usart_init (USART_TypeDef *usart, uint32_t bus_freq, uint32_t fmt);

/**
 * Redirect printf, putc, puts through the serial port.
 * This uses the printf routines from libuseful.
 * Very useful for debugging.
 * @param usart The USART to redirect stdio through
 */
extern void usart_printf (USART_TypeDef *usart);

/**
 * Send a single byte through the serial port.
 * @param usart The USART to send the character through
 * @param c The character to send
 */
extern void usart_putc (USART_TypeDef *usart, uint8_t c);

/**
 * Receive a single byte through the serial port.
 * Waits until the character is ready.
 * @param usart The USART to read the character from
 * @return The character read from the USART
 */
extern uint8_t usart_getc (USART_TypeDef *usart);

/**
 * Check if input is ready to be read from the port.
 * @param usart The USART to check data readiness in
 * @return true if there is data in the receive buffer
 */
INLINE_ALWAYS bool usart_rx_ready (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (usart->SR & USART_SR_RXNE) != 0; }
#elif defined USART_TYPE_2
{ return (usart->ISR & USART_ISR_RXNE) != 0; }
#endif

/**
 * Check if transmission data register is free
 * @param usart A pointer to USART to check
 * @return true if transmission data register is free
 */
INLINE_ALWAYS bool usart_tx_ready (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (usart->SR & USART_SR_TXE) != 0; }
#elif defined USART_TYPE_2
{ return (usart->ISR & USART_ISR_TXE) != 0; }
#endif

/**
 * Check if transmission is complete (both bytes in outgoing FIFO and
 * data register).
 * @param usart A pointer to USART to check
 * @return true if transmission is complete
 */
INLINE_ALWAYS bool usart_tx_complete (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (usart->SR & USART_SR_TC) != 0; }
#elif defined USART_TYPE_2
{ return (usart->ISR & USART_ISR_TC) != 0; }
#endif

/**
 * Return a reference to transmission data register
 * @param usart A pointer to USART
 * @return a reference to transmission data register
 */
INLINE_ALWAYS __IO uint16_t *usart_tdr (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (__IO uint16_t *)&usart->DR; }
#elif defined USART_TYPE_2
{ return &usart->TDR; }
#endif

/**
 * Return a reference to receiver data register
 * @param usart A pointer to USART
 * @return a reference to receiver data register
 */
INLINE_ALWAYS __IO uint16_t *usart_rdr (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ return (__IO uint16_t *)&usart->DR; }
#elif defined USART_TYPE_2
{ return &usart->RDR; }
#endif

/**
 * Send a BREAK metacharacter.
 * @param usart A pointer to USART
 */
INLINE_ALWAYS void usart_send_break (USART_TypeDef *usart)
#if defined USART_TYPE_1
{ usart->CR1 |= USART_CR1_SBK; }
#elif defined USART_TYPE_2
{ usart->RQR |= USART_RQR_SBKRQ; }
#endif

/**
 * Enable or disable data transmission to UART using DMA.
 * @param usart The USART to set DMA TX state
 * @param state true to enable transmission, false to disable
 */
INLINE_ALWAYS void usart_dma_tx (USART_TypeDef *usart, bool state)
{
    if (state)
        usart->CR3 |= USART_CR3_DMAT;
    else
        usart->CR3 &= ~USART_CR3_DMAT;
}

/**
 * Check if USART TX DMA is enabled
 *
 * @param usart The USART to check DMA state
 * @return true if TX DMA is enabled
 */
INLINE_ALWAYS bool usart_dma_tx_enabled (USART_TypeDef *usart)
{ return (usart->CR3 & USART_CR3_DMAT) != 0; }

/**
 * Enable or disable receiving data from UART using DMA.
 * @param usart The USART to set DMA RX state
 * @param state true to enable receiving, false to disable
 */
INLINE_ALWAYS void usart_dma_rx (USART_TypeDef *usart, bool state)
{
    if (state)
        usart->CR3 |= USART_CR3_DMAR;
    else
        usart->CR3 &= ~USART_CR3_DMAR;
}

/**
 * Check if USART RX DMA is enabled
 *
 * @param usart The USART to check DMA state
 * @return true if RX DMA is enabled
 */
INLINE_ALWAYS bool usart_dma_rx_enabled (USART_TypeDef *usart)
{ return (usart->CR3 & USART_CR3_DMAR) != 0; }

#ifdef __cplusplus
}
#endif

#endif // USART_TYPE_NONE

#endif // _STM32_USART_H
