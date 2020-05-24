/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "cdcacm-priv.h"

// Set to '#if 1' for USB debugging on USART1
#if 0
#  define trace(c)          usart_putc (USART1, c)
#else
#  define trace(c)          (void)0
#endif

// Not every MCU has USB
#ifdef USB_BASE

#ifdef USB_CDC_LINE_CODING
uint16_t uca_line_state = 0;
uca_line_format_t uca_line_format = {115200, 0, 0, 8};
static uint8_t uca_change_flags;
#endif

// Current device config number: (SET|GET)_CONFIGURATION
static uint8_t uca_device_config = 0;
// Set DADDR before next DATA transmission
static uint8_t uca_set_daddr = 0;
// Current device status: GET_STATUS
static uint16_t uca_device_status =
        (USB_CDC_POWER_MA ? 0 : USB_REQ_STATUS_SELFPOWERED);

/* Per-endpoint allocated buffers */
static uca_ep_status_t uca_ep_status [EP_COUNT];
static const uca_ep_config_t uca_ep_config [EP_COUNT] =
{
    // Control endpoint (part of COMM interface)
    {EP_CTL | USB_EP_CONTROL   | USB_EP_RX_VALID | USB_EP_TX_NAK, EP_CTL_PKT_SIZE, EP_CTL_PKT_SIZE},
    // Interrupt endpoint (part of COMM interface)
    {EP_INT | USB_EP_INTERRUPT | USB_EP_RX_VALID | USB_EP_TX_NAK, EP_INT_PKT_SIZE, EP_INT_PKT_SIZE},
    // BULK IN/OUT endpoints (part of DATA interface)
    {EP_DATA | USB_EP_BULK     | USB_EP_RX_VALID | USB_EP_TX_NAK, EP_DATA_PKT_SIZE, EP_DATA_PKT_SIZE},
};

/* Static buffer for receiving USB packets.
 * Out device can't receive from more than one endpoint at once,
 * so buffer will be shared between all OUT endpoints.
 */
static uint8_t uca_rx_buffer [EP_DATA_PKT_SIZE];

// Currently active request
static usb_request_status_t uca_req;

void uca_init ()
{
    nvic_disable (USB_LP_CAN1_RX0_IRQn);

    // Enable USB GPIO pins
    // Bring USB+ down for a few milliseconds in order to force
    // the USB Host to re-enumerate the device
    RCC_BEGIN;
        RCC_ENA_GPIO (USB_DP);
        RCC_ENA_GPIO (USB_DM);
        RCC_DIS (_USB);
    RCC_END;

    GPIO_SETUP (USB_DP_INIT);
    GPIO_SETUP (USB_DM);
    // Wait ~5ms: delay() requires SysTick timer to be running!
    delay (5E-3);
    GPIO_SETUP (USB_DP);
    RCC_ENABLE (_USB);

    USB->CNTR   = USB_CNTR_FRES;    /* Force USB Reset */
    USB->CNTR   = USB_CNTR_RESETM;  /* Generate IRQ after reset */
    USB->ISTR   = 0;

    uca_req.epn = 0xff;

    nvic_setup (USB_LP_CAN1_RX0_IRQn, USB_IRQ_PRIO);
}

static void uca_reset ()
{
    USB->CNTR   = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_ERRM |
            USB_CNTR_SUSPM | USB_CNTR_WKUPM;
    USB->DADDR  = USB_DADDR_EF;

    /* We set USB->BTABLE (USB buffer descriptor table address)
     * to zero and never change it, so we statically allocate
     * endpoint buffers starting after USBEPBUF.
     */
    USB->BTABLE = 0x00;

    memset (&uca_ep_status, 0, sizeof (uca_ep_status));

    uint16_t pmaddr = sizeof (USBEPBUF);
    unsigned i;
    for (i = 0; i < EP_COUNT; i++)
    {
        // Allocate PMA memory for transmission
        USBEPBUF [i].tx.addr = pmaddr;
        USBEPBUF [i].tx.count = 0;
        pmaddr += uca_ep_config [i].tx_max;

        // Allocate PMA memory for reception
        unsigned max_size = uca_ep_config [i].rx_max;
        USBEPBUF [i].rx.addr = pmaddr;
        USBEPBUF [i].rx.count = (max_size >= 64) ?
                    ((((max_size - 32) / 32) << USB_COUNT_RX_NUM_BLOCK_Pos) | USB_COUNT_RX_BLSIZE) :
                    ((max_size / 2) << USB_COUNT_RX_NUM_BLOCK_Pos);
        pmaddr += max_size;

        // Set endpoint configuration
        USBEPxR (i) = uca_ep_config [i].epn_type;
    }

    // Disable all other USB peripherial endpoint machines
    while (i < 8)
        USBEPxR (i++) = 0;
}

static inline void uca_move_from_pma (unsigned epn)
{
    const uint16_t *src = (const uint16_t *)
            (USB_PMAADDR + USBEPBUF [epn].rx.addr * USB_PMA_ACCESS);
    uint16_t *dst = (uint16_t *)(uca_rx_buffer + uca_req.data_size);
    int count = USBEPBUF [epn].rx.count & 0x3FF;
    uca_req.data_size += count;
    uca_req.complete = (count < uca_ep_config [epn].rx_max);
    for (; count > 0; count -= 2)
    {
        *dst++ = *src;
        src += USB_PMA_ACCESS;
    }
}

static inline void uca_kick_tx (unsigned epn)
{
    // Move next piece of data to PMA
    const uint16_t *src;
    uint16_t *dst;
    unsigned tx_max = uca_ep_config [epn].tx_max;
    int count = MIN (tx_max, uca_ep_status [epn].tx_len);
    uca_ep_status [epn].tx_len -= count;
    USBEPBUF [epn].tx.count = count;

    // If last transfer was tx_max size, send one more ZLP
    if (count == 0)
        src = 0;
    else
    {
        tx_max -= count;
        src = uca_ep_status [epn].tx_buff;
        dst = (uint16_t *)(USB_PMAADDR + USBEPBUF[epn].tx.addr * USB_PMA_ACCESS);
        for (; count > 0; count -= 2)
        {
            *dst = *src++;
            dst += USB_PMA_ACCESS;
        }

        // If this is last transfer, and count is less than tx_max, clear the buffer
        if (tx_max > 0)
            src = 0;
    }
    uca_ep_status [epn].tx_buff = src;

    // Tell peripherial we're ready to transmit data
    USBEPxR (epn) = (USBEPxR (epn) ^ USB_EP_TX_VALID) &
            (USB_EPREG_MASK | USB_EPTX_STAT);
}

static void uca_send_data (unsigned epn, const void *data, uint16_t size)
{
    trace (size ? 'D' : 'd');
    uca_ep_status [epn].tx_len = size;
    uca_ep_status [epn].tx_buff = data;
    uca_kick_tx (epn);
}

static inline uca_status_t uca_get_descriptor (usb_data_t *reply)
{
    usb_setup_packet_t *setup = (usb_setup_packet_t *)uca_rx_buffer;

    switch (setup->wValue.u8h)
    {
        case USB_DT_DEVICE:
            reply->data = &usb_cdc_acm_desc;
            reply->size = sizeof (usb_cdc_acm_desc);
            break;

        case USB_DT_CONFIG:
            reply->data = &usb_cdc_acm_config;
            reply->size = sizeof (usb_cdc_acm_config);
            break;

        case USB_DT_STRING:
        {
            const usb_string_desc_t *desc = &usb_desc_strings;
            for (unsigned idx = setup->wValue.u8l; idx != 0; idx--)
            {
                desc = (usb_string_desc_t *)((uint8_t *)desc + desc->bLength);
                if (desc->bLength == 0)
                    return UCA_ST_STALL;
            }

            reply->data = desc;
            reply->size = desc->bLength;
            break;
        }

        default:
            return UCA_ST_STALL;
    }

    return UCA_ST_VALID;
}

// Encode bRequest and bmRequestType into one 16-bit value
#define REQ_ENC(req,type) (((req) << 8) | (type))

static inline uca_status_t uca_handle_setup (usb_data_t *reply)
{
    usb_setup_packet_t *setup = (usb_setup_packet_t *)uca_rx_buffer;
    trace (setup->bRequest + '0');
    trace ((setup->bmRequestType & USB_REQ_TYPE_DIR) ? '<' : '>');

    switch (REQ_ENC (setup->bRequest, setup->bmRequestType))
    {
        case REQ_ENC (USB_REQ_STD_SET_ADDRESS,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            // Set USB device address && enable on next TX IRQ
            uca_set_daddr = setup ->wValue.u8l;
            break;

        case REQ_ENC (USB_REQ_STD_SET_CONFIGURATION,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            uca_device_config = setup->wValue.u8l;
            break;

        case REQ_ENC (USB_REQ_STD_GET_CONFIGURATION,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DIR):
            reply->data = &uca_device_config;
            reply->size = 1;
            break;

        case REQ_ENC (USB_REQ_STD_GET_DESCRIPTOR,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DIR):
            return uca_get_descriptor (reply);

        case REQ_ENC (USB_REQ_STD_GET_STATUS,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DIR):
            reply->data = &uca_device_status;
            reply->size = 2;
            break;

#if USB_CDC_LINE_CODING
        case REQ_ENC (USB_REQ_CDC_SET_LINE_CODING,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            if (uca_req.data_size < sizeof (usb_setup_packet_t) + sizeof (uca_line_format_t))
                return UCA_ST_STALL;

            memcpy (&uca_line_format, (uint8_t *)setup + sizeof (*setup),
                    sizeof (uca_line_format));
            uca_change_flags |= 2;
            break;

        case REQ_ENC (USB_REQ_CDC_GET_LINE_CODING,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_DIR):
            reply->data = &uca_line_format;
            reply->size = sizeof (uca_line_format);
            break;

        case REQ_ENC (USB_REQ_CDC_SET_CONTROL_LINE_STATE,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            uca_line_state = setup->wValue.u16;
            uca_change_flags |= 1;
            break;

        case REQ_ENC (USB_REQ_CDC_SEND_BREAK,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            uca_change_flags |= 4;
            break;
#endif

        default:
            return UCA_ST_STALL;
    }

    return UCA_ST_VALID;
}

static inline void uca_handle_rx (uint32_t EPxR, unsigned epn)
{
    // Reset current request data if endpoint changes, or a SETUP packet
    if ((uca_req.epn != epn) || (EPxR & USB_EP0R_SETUP))
    {
        memset (&uca_req, 0, sizeof (uca_req));
        uca_req.epn = epn;
    }
    else
        // Note: this will be printed BEFORE the request
        // as many times as there are data fragments for SETUP
        trace ('.');

    // Move data from packet buffers to SRAM
    uca_move_from_pma (epn);

    // Will respond with a VALID to this transaction by default.
    uca_status_t rxstat = UCA_ST_VALID;

    if (EPxR & USB_EP0R_SETUP)
    {
        // This is a SETUP packet
        uca_req.setup = true;

        if (uca_req.data_size < sizeof (usb_setup_packet_t))
        {
            // broken setup packet
            uca_req.epn = 0xff;
            uca_req.complete = false;
            rxstat = UCA_ST_STALL;
        }
        else
        {
            // And we're possibly waiting for DATA packet(-s) to follow
            usb_setup_packet_t *setup = (usb_setup_packet_t *)uca_rx_buffer;
            uca_req.complete = ((setup->bmRequestType & USB_REQ_TYPE_DIR) != 0)
                    || (setup->wLength == 0);
        }
    }

    if (uca_req.complete)
    {
        trace ('c');
        switch (uca_req.epn)
        {
            case EP_CTL:
                if (uca_req.setup)
                {
                    usb_data_t reply;
                    rxstat = uca_handle_setup (&reply);

                    if (rxstat == UCA_ST_VALID)
                    {
                        // Reply length cannot exceed wLength (but can exceed max_pkt)
                        usb_setup_packet_t *setup = (usb_setup_packet_t *)uca_rx_buffer;
                        if (reply.size > setup->wLength)
                            reply.size = setup->wLength;

                        // Send the answer
                        uca_send_data (EP_CTL, reply.data, reply.size);
                    }
                }
                else
                    rxstat = UCA_ST_STALL;
                break;

            case EP_DATA:
                // Got data? Send to user function.
                rxstat = uca_received (uca_rx_buffer, uca_req.data_size);
                break;

            default:
                // Unknown endpoint
                rxstat = UCA_ST_STALL;
                break;
        }

        // Mark the request as handled
        uca_req.epn = 0xff;
    }

    // Set EP receiver & transmitter state
    uint16_t mask = (rxstat << USB_EPRX_STAT_Pos);

    // USB_EP_(RX|TX)_(DIS|STALL|NAK|VALID) receiver state bits should
    // be toggled by writing '1' to corresponding location, and '0'
    // will leave the bit in the same state (see refman).
    //
    // The XOR operation ensures that '1' will be written only
    // to the bits that must change their value, and '0' to bits that
    // should keep their state.
    //
    // Compare with PCD_SET_EP_(RX|TX|RXTX)_STATUS from SPDL/CUBE, hehe.
    USBEPxR (epn) = (USBEPxR (epn) ^ mask) &
            (USB_EPREG_MASK | USB_EPRX_STAT);
}

static inline void uca_handle_tx (unsigned epn)
{
    // Fragment transmission complete, send next fragment if any
    if (uca_ep_status [epn].tx_buff)
        uca_kick_tx (epn);

    // If there's no more data to transmit, tell client
    if ((epn == EP_DATA) && !uca_ep_status [epn].tx_buff)
        uca_transmitted ();
}

static inline void uca_handle_ctr (unsigned epn)
{
    uint32_t EPxR = USBEPxR (epn);
    if (EPxR & USB_EP_CTR_RX)
    {
        // Acknowledge RX interrupt
        USBEPxR (epn) = EPxR & (USB_EPREG_MASK & ~USB_EP_CTR_RX);

        uca_handle_rx (EPxR, epn);
    }

    if (EPxR & USB_EP_CTR_TX)
    {
        // Acknowledge TX interrupt
        USBEPxR (epn) = EPxR & (USB_EPREG_MASK & ~USB_EP_CTR_TX);

        // If there's a pending device address change, apply it now
        if (uca_set_daddr != 0)
        {
            trace ('A');
            USB->DADDR = uca_set_daddr | USB_DADDR_EF;
            uca_set_daddr = 0;
        }
        else
            trace ('T');

        // Send more, if needed
        uca_handle_tx (epn);
    }

    // Now, when we handled the immediate tasks, we can notify user code
    if (uca_change_flags & 1)
    {
        uca_change_flags &= ~1;
        uca_line_state_changed ();
    }

    if (uca_change_flags & 2)
    {
        uca_change_flags &= ~2;
        uca_line_format_changed ();
    }

    if (uca_change_flags & 4)
    {
        uca_change_flags &= ~4;
        uca_line_break ();
    }
}

void USB_LP_CAN1_RX0_IRQHandler ()
{
    uint32_t istr = USB->ISTR;

    if (istr & USB_ISTR_RESET)
    {
        // To avoid spurious clearing of some bits, it is recommended
        // to clear them with a load instruction where all bits which
        // must not be altered are written with 1, and all bits to be
        // cleared are written with 0.
        USB->ISTR = (uint16_t)~USB_ISTR_RESET;

        trace ('R');
        uca_reset ();
    }

    else if (istr & USB_ISTR_SUSP)
    {
        USB->CNTR |= USB_CNTR_FSUSP;
        USB->ISTR = (uint16_t)~USB_ISTR_SUSP;
    }

    else if (istr & USB_ISTR_WKUP)
    {
        USB->CNTR &= ~USB_CNTR_FSUSP;
        USB->ISTR = (uint16_t)~USB_ISTR_WKUP;
    }

    else if (istr & USB_ISTR_CTR)
    {
        // The CTR bit is readonly, will be dropped as soon as
        // pending state bits in EPxR will be cleared
        uca_handle_ctr (istr & USB_ISTR_EP_ID);
    }

    // Clear all other interrupts we don't handle
    else if (istr & (USB_ISTR_PMAOVR | USB_ISTR_ERR |
                     USB_ISTR_SOF | USB_ISTR_ESOF))
        USB->ISTR = (uint16_t)~(USB_ISTR_PMAOVR | USB_ISTR_ERR |
                                USB_ISTR_SOF | USB_ISTR_ESOF);
}

unsigned uca_state ()
{
    return (RCC_ENABLED (_USB) && (USB->DADDR & USB_DADDR_EF) ? UCA_STATE_ENABLED : 0) |
           ((USB->CNTR & USB_CNTR_FSUSP) ? UCA_STATE_SUSPENDED : 0) |
           ((USB->DADDR & USB_DADDR_ADD) ? UCA_STATE_CONFIGURED : 0) |
           (uca_ep_status [EP_DATA].tx_buff ? 0 : UCA_STATE_TXEMPTY);
}

bool uca_transmit (const void *data, unsigned data_size)
{
    if (uca_ep_status [EP_DATA].tx_buff)
        return false;

    uca_send_data (EP_DATA, data, data_size);
    return true;
}

/*
 * NOTE: The following functions should NOT be modified,
 * instead, just define your own callback in your source file.
 */

__WEAK uca_status_t uca_received (const void *data, unsigned length)
{
    (void)data;
    (void)length;
    return UCA_ST_NAK;
}

__WEAK void uca_transmitted ()
{
}

#ifdef USB_CDC_LINE_CODING

__WEAK void uca_line_state_changed ()
{
}

__WEAK void uca_line_format_changed ()
{
}

__WEAK void uca_line_break ()
{
}

#endif

#endif // USB_BASE
