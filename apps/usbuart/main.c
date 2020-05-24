/*
    Transmutate your STM32F1 BluePill into a cheap USB-UART dongle!
    Copyright (C) 2020 by Andrew Zabolotny

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

/*
 * This sample application uses:
 *
 * @li USB port for PC connection
 * @li One of the USARTx ports for target device connection
 * @li A LED to display activity
 *
 * Data from USART is received using DMA, using the IDLE USART interrupt
 * to detect if we should prematurely end the DMA transfer.
 *
 * Data to USART is also sent via DMA. There's nothing special about it.
 *
 * Both direction use separate memory buffers.
 */

#include "usb-cdc-acm.h"
#include "atomic.h"
#include "ringbuff.h"

// SysTick IRQ priority
#define SYSTICK_IRQ_PRIO 0

// ----- // Global variables // --------------------------------------------- //

// Current USART line format
static unsigned ser_fmt = SERIAL_SETUP;
// Change line format to this
static unsigned chg_ser_fmt = SERIAL_SETUP;
// Serial port is enabled?
static bool ser_ena = false;

// USB transmission buffer
static ringbuff_t usb_buff;
// Serial transmission buffer
static ringbuff_t ser_tx_buff;
// Serial receiving buffer
static ringbuff_t ser_rx_buff;

// Last clock when we received or transmitted data
static clock_t last_activity;

// ----- // Forward declarations // ----------------------------------------- //

// Start serial port for reception & transmission
static void serial_init ();
// Stop the serial port
static void serial_stop ();
// If USART DMA is idle, start transmitting data from USART buffer
static void serial_tx ();
// Start receiving using DMA
static void serial_rx ();
// Put some data into USART transmission buffers
static unsigned serial_put (const void *text, unsigned size);

// ----- // USB // ---------------------------------------------------------- //

static inline void usb_init ()
{
    ringbuff_init (&usb_buff);
    uca_init ();
}

// Copy a string to USB output buffer.
// Returns number of bytes that were actually copied to output.
// Does not need protection from IRQ.
static inline unsigned usb_put (const uint8_t *text, unsigned size)
{
    unsigned ret = ringbuff_put (&usb_buff, text, size);

    // If there's data in buffer and transmitter is idle, send now
    if (!ringbuff_empty (&usb_buff) && (usb_buff.inflight == 0))
        // prevent concurrent uca_transmitted invocation from IRQ handler
        ATOMIC_BLOCK (FORCEON) uca_transmitted ();

    if (ret > 0)
        last_activity = clock;

    return ret;
}

// Must be always invoked in exclusive mode,
// e.g. either from IRQ or with interrupts disabled
// When invoked it supposes previous transfer has completed.
void uca_transmitted ()
{
    // Acknowledge 'inflight' data is gone
    ringbuff_ack_inflight (&usb_buff);

    // Kick next data block
    void *data;
    if ((usb_buff.inflight = ringbuff_next_data (&usb_buff, &data)) != 0)
        if (!uca_transmit (data, usb_buff.inflight))
            usb_buff.inflight = 0;
}

uca_status_t uca_received (const void *data, unsigned data_size)
{
    return serial_put (data, data_size);
}

void uca_line_state_changed ()
{
    bool state = !!(uca_line_state & USB_CDC_LINE_STATE_DTR);
    if (state == ser_ena)
        return;

    if ((ser_ena = state))
        serial_init ();
    else
        serial_stop ();
}

void uca_line_format_changed ()
{
    // Invalid line formats are ignored

    uint32_t fmt = USART_BAUD (uca_line_format.dwDTERate);
    if (fmt > USART_BAUD_MASK)
        return;

    // 5, 6, 7, 8 or 16
    switch (uca_line_format.bDataBits)
    {
        // no 5,6 or 7 bits support in STM32F1 :(
        case 8: fmt |= USART_CHARBITS_8; break;
        default: return;
    }

    // 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space
    switch (uca_line_format.bParityType)
    {
        case 0: fmt |= USART_PARITY_NONE; break;
        case 1: fmt |= USART_PARITY_ODD; break;
        case 2: fmt |= USART_PARITY_EVEN; break;
        default: return;
    }

    // 0: 1 stop bit, 1: 1.5 stop bits, 2: 2 stop bits
    switch (uca_line_format.bCharFormat)
    {
        case 0: fmt |= USART_STOPBITS_1; break;
        case 1: fmt |= USART_STOPBITS_1_5; break;
        case 2: fmt |= USART_STOPBITS_2; break;
        default: return;
    }

    chg_ser_fmt = fmt;
}

// ----- // USART // -------------------------------------------------------- //

void DMA_IRQ_HANDLER (SERIAL_TX) ()
{
    if (DMA (SERIAL_TX)->ISR & DMA_ISR (SERIAL_TX, GIF))
    {
        // Acknowledge the interrupt
        DMA (SERIAL_TX)->IFCR = DMA_IFCR (SERIAL_TX, CGIF);

        // Disable USART -> DMA transmission
        USART (SERIAL)->CR3 &= ~USART_CR3_DMAT;

        // Notify the 'inflight' data is gone
        ringbuff_ack_inflight (&ser_tx_buff);
        // Send moar of it!
        serial_tx ();
    }
}

void DMA_IRQ_HANDLER (SERIAL_RX) ()
{
    uint32_t isr = DMA (SERIAL_RX)->ISR;

    // Transfer error?
    if (isr & DMA_ISR (SERIAL_RX, TEIF))
    {
        // Acknowledge the interrupt
        DMA (SERIAL_RX)->IFCR = DMA_IFCR (SERIAL_RX, CTEIF);

        // Resume receiving
        serial_rx ();
    }

    else if (isr & DMA_ISR (SERIAL_RX, GIF))
    {
        // Acknowledge the interrupt
        DMA (SERIAL_RX)->IFCR = DMA_IFCR (SERIAL_RX, CGIF);

        // Disable USART -> DMA transmission
        USART (SERIAL)->CR3 &= ~USART_CR3_DMAR;

        // Resume receiving
        serial_rx ();
    }
}

void USART_IRQ_HANDLER (SERIAL) ()
{
    uint32_t sr = USART (SERIAL)->SR;
    if (sr & USART_SR_IDLE)
    {
        // Clear the IDLE bit with a void read from DR
        (void)USART (SERIAL)->DR;

        serial_rx ();
    }
}

/**
 * This can be invoked from DMA IRQ or from USART IRQ contexts
 * (and once from serial_init but that does not count).
 * Access to ring buffer should be locked.
 */
static void serial_rx ()
{
    ATOMIC_BLOCK (FORCEON)
    {
        // Acknowledge data received yet by DMA
        if (ser_rx_buff.inflight)
        {
            // Determine how many bytes were actually received
            unsigned remaining = DMAC (SERIAL_RX)->CNDTR;
            unsigned gone = (ser_rx_buff.inflight > remaining) ?
                ser_rx_buff.inflight - remaining : 0;
            ser_rx_buff.inflight -= gone;
            ringbuff_fill (&ser_rx_buff, gone);
        }

        // Count how much data ready for transmission over USB we have
        void *data;
        unsigned count = ringbuff_next_data (&ser_rx_buff, &data);
        // Push received bytes through USB
        count = usb_put (data, count);
        // Free bytes we just passed to usb
        ringbuff_ack (&ser_rx_buff, count);

        // If there are more pending DMA transfers, leave now
        if (ser_rx_buff.inflight != 0)
            break;

        // Get a pointer to next contiguous area in receive buffer
        ser_rx_buff.inflight = ringbuff_next_free (&ser_rx_buff, &data);
        if (ser_rx_buff.inflight == 0)
            // No space in serial buffer
            break;

        // Enable DMA transfer from serial port to buffer
        DMA_COPY (SERIAL_RX,
                  DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8 | DMA_CCR_MINC | DMA_CCR_PL_LOW | DMA_CCR_TCIE,
                  (void *)&USART (SERIAL)->DR, data, ser_rx_buff.inflight);

        // Enable DMA -> USART transmission
        USART (SERIAL)->CR3 |= USART_CR3_DMAR;
    }
}

static void serial_init ()
{
    // Clear buffers
    ringbuff_init (&ser_rx_buff);
    ringbuff_init (&ser_tx_buff);

    usart_init (USART (SERIAL), CLOCK_USART (SERIAL), ser_fmt);
    // Enable IDLE interrupts
    USART (SERIAL)->CR1 |= USART_CR1_IDLEIE;
    nvic_setup (USART_IRQ (SERIAL), USART_IRQ_PRIO (SERIAL));

    // Set up and enable DMA interrupts
    nvic_setup (DMA_IRQ (SERIAL_TX), DMA_IRQ_PRIO (SERIAL_TX));
    nvic_setup (DMA_IRQ (SERIAL_RX), DMA_IRQ_PRIO (SERIAL_RX));

    // Set up DMA & USART for receiving
    serial_rx ();
}

static void serial_stop ()
{
    DMA_STOP (SERIAL_TX);
    DMA_STOP (SERIAL_RX);
    nvic_disable (DMA_IRQ (SERIAL_TX));
    nvic_disable (DMA_IRQ (SERIAL_RX));
}

/**
 * Put data into ser_tx_buff.
 * This is invoked only from USB IRQ context, so it doesn't need locking.
 */
static uca_status_t serial_put (const void *text, unsigned size)
{
    // If whole data cannot be put into serial buffer, tell host to retry
    if (ringbuff_free (&ser_tx_buff) < size)
        return UCA_ST_NAK;

    ringbuff_put (&ser_tx_buff, text, size);

    // If there's data in buffer and transmitter is idle, send now
    if (!ringbuff_empty (&ser_tx_buff) && (ser_tx_buff.inflight == 0))
        serial_tx ();

    return UCA_ST_VALID;
}

/**
 * This is invoked from both DMA IRQ and USB IRQ contexts,
 * so it runs with IRQs disabled.
 *
 * This function does not check if there are inflight bytes!
 * Please do it before calling it (or make sure it is 0).
 */
static void serial_tx ()
{
    ATOMIC_BLOCK (FORCEON)
    {
        void *data;
        ser_tx_buff.inflight = ringbuff_next_data (&ser_tx_buff, &data);
        if (ser_tx_buff.inflight == 0)
            // Nothing to transmit
            return;

        last_activity = clock;

        // Set up DMA for transfer from memory to USART
        DMA_COPY (SERIAL_TX,
                  DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8 | DMA_CCR_MINC | DMA_CCR_PL_VERYHIGH | DMA_CCR_TCIE,
                  (void *)data, &USART (SERIAL)->DR, ser_tx_buff.inflight);

        // Enable USART -> DMA transmission
        USART (SERIAL)->CR3 |= USART_CR3_DMAT;
    }
}

// ----- // SysTick // ------------------------------------------------------ //

void SysTick_Handler ()
{
    clock++;

    if ((clock_t)(clock - last_activity) <= (CLOCKS_PER_SEC / 16))
        GPIO_RESET (LED);
    else
    {
        GPIO_SET (LED);
        if (ser_ena)
            switch (clock & (CLOCKS_PER_SEC - 1))
            {
                case 0:
                case CLOCKS_PER_SEC / 4:
                    GPIO_RESET (LED);
                    break;

                case CLOCKS_PER_SEC - 1:
                    // Try restarting RX if DMA stopped due to buffer overflow
                    // Should never happen (USB speed >> USART speed) but who knows.
                    if ((USART (SERIAL)->CR3 & USART_CR3_DMAR) == 0)
                        serial_rx ();
                    break;
            }
    }

    if (chg_ser_fmt != ser_fmt)
    {
        ser_fmt = chg_ser_fmt;
        /*
        if (ser_ena)
        {
            serial_stop ();
            serial_init ();
        }
        */
    }
}

void systick_init ()
{
    // Set up system timer for CLOCKS_PER_SEC interrupts per second
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);
    // Enable interrupts from SysTick counter
    systick_int_enable (SYSTICK_IRQ_PRIO);
}

int main (void)
{
    // Enable used peripherials
    RCC_BEGIN;
        // Status LED
        RCC_ENA_GPIO (LED);

        // USART & DMA
        RCC_ENA_USART (SERIAL);
        RCC_ENA_GPIO (SERIAL_TX);
        RCC_ENA_GPIO (SERIAL_RX);
        RCC_ENA_DMA (SERIAL_TX);
        RCC_ENA_DMA (SERIAL_RX);

        // USB peripherial & pins will be enabled in uca_init()
    RCC_END;

    // Initialize activity LED
    GPIO_SETUP (LED);

    // Initialize serial port GPIOs
    GPIO_SETUP (SERIAL_TX);
    GPIO_SETUP (SERIAL_RX);

    systick_init ();
    usb_init ();

    __enable_irq ();

    for (;;)
    {
        // Do nothing :-D
        __WFI ();
    }
}
