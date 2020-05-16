/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "cdcacm-priv.h"

/// Control endpoint number
#define EP_CTRL     0
/// Transmit (from device point of view) endpoint number
#define EP_TX       2
/// Receive (from device point of view) endpoint number
#define EP_RX       3

#ifdef USB_CDC_LINE_CODING
uint16_t uca_line_state = 0;
uca_line_format_t uca_line_format = {115200, 0, 0, 8};
#endif

// Current device config number: (SET|GET)_CONFIGURATION
static uint8_t uca_device_config = 0;
// Set DADDR before next DATA transmission
static uint8_t uca_set_daddr = 0;
// The size of DATA packet associated with SETUP packet
static uint8_t uca_setup_data_size = 0;
// Last SETUP request (used when processing following DATA packets)
static uint16_t uca_setup_req = 0;
// Current device status: GET_STATUS
static uint16_t uca_device_status =
        (USB_CDC_POWER_MA ? 0 : USB_REQ_STATUS_SELFPOWERED);

/* Per-endpoint allocated buffers */
static uca_ep_status_t uca_ep_status [EP_COUNT];
static const uca_ep_config_t uca_ep_config [EP_COUNT] =
{
    // Control endpoint (part of COMM interface)
    {0 | USB_EP_CONTROL   | USB_EP_RX_VALID | USB_EP_TX_NAK, EP0_PKT_SIZE, EP0_RXB_SIZE},
    // Interrupt endpoint (part of COMM interface)
    {1 | USB_EP_INTERRUPT | USB_EP_RX_VALID | USB_EP_TX_NAK, EP1_PKT_SIZE, EP1_RXB_SIZE},
    // BULK IN endpoint (part of DATA interface)
    {2 | USB_EP_BULK      | USB_EP_RX_VALID | USB_EP_TX_NAK, EP2_PKT_SIZE, EP2_RXB_SIZE},
    // BULK OUT endpoint (part of DATA interface)
    {3 | USB_EP_BULK      | USB_EP_RX_VALID | USB_EP_TX_NAK, 0, EP3_RXB_SIZE},
};

/* Statically allocated buffer for copying received data from
 * packet buffer memory. The total size must be equal to sum
 * of all rx_max fields from ucaEpConfig above.
 */
static uint8_t uca_ep_buffer [EP0_RXB_SIZE + EP1_RXB_SIZE + EP2_RXB_SIZE + EP3_RXB_SIZE];

void uca_init ()
{
    nvic_disable (USB_LP_CAN1_RX0_IRQn);

    // Enable USB GPIO pins
    // Bring USB+ down for a few milliseconds in order to force
    // the USB Host to re-enumerate the device
    RCC_BEGIN;
        RCC_ENA (RCC_GPIO (USB_DP));
        RCC_ENA (RCC_GPIO (USB_DM));
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

    nvic_enable (USB_LP_CAN1_RX0_IRQn);
}

static void uca_reset ()
{
    USB->BTABLE = 0x00;
    USB->CNTR   = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_ERRM |
            USB_CNTR_SUSPM | USB_CNTR_WKUPM;
    USB->DADDR  = USB_DADDR_EF;

    /* We set USB->BTABLE (USB buffer descriptor table address)
     * to zero and never change it, so we statically allocate
     * endpoint buffers starting after USBEPBUF.
     */

    uint8_t *alloc = uca_ep_buffer;
    uint16_t pmaddr = sizeof (USBEPBUF);
    unsigned i;
    for (i = 0; i < EP_COUNT; i++)
    {
        USBEPBUF [i].tx.addr = pmaddr;
        USBEPBUF [i].tx.count = 0;
        pmaddr += uca_ep_config [i].tx_max;

        unsigned max_size = uca_ep_config [i].rx_max;
        USBEPBUF [i].rx.addr = pmaddr;
        USBEPBUF [i].rx.count = (max_size >= 64) ?
                    ((((max_size - 32) / 32) << 10) | 0x8000) :
                    ((max_size / 2) << 10);
        pmaddr += max_size;

        if (!uca_ep_status [i].rx_buff)
        {
            uca_ep_status [i].rx_buff = (uint16_t *)alloc;
            alloc += max_size;
        }

        USBEPxR (i) = uca_ep_config [i].epn_type;
    }

    // Disable all other EPs
    while (i < 8)
       USBEPxR (i++) = 0;
}

static inline void uca_move_from_pma (uint8_t epn, void **data, unsigned *data_size)
{
    const uint16_t *src = (const uint16_t *)
            (USB_PMAADDR + USBEPBUF [epn].rx.addr * USB_PMA_ACCESS);
    uint16_t *dst = uca_ep_status [epn].rx_buff;
    int count = uca_ep_status [epn].rx_len = (USBEPBUF [epn].rx.count & 0x3FF);
    *data = dst;
    *data_size = count;
    for (; count > 0; count -= 2)
    {
        *dst++ = *src;
        src += USB_PMA_ACCESS;
    }
}

static inline void uca_kick_tx (uint8_t epn)
{
    // Move next piece of data to PMA
    const uint16_t *src;
    uint16_t *dst;
    unsigned tx_max = uca_ep_config [epn].tx_max;
    int count = MIN (tx_max, uca_ep_status [epn].tx_len);
    uca_ep_status [epn].tx_len -= count;
    USBEPBUF [epn].tx.count = count;

    // If last transfer is tx_max size, send one more 0-byte DATAx packet
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

static void uca_send_data (uint8_t epn, const void *data, uint16_t size)
{
    uca_ep_status [epn].tx_len = size;
    uca_ep_status [epn].tx_buff = data;
    uca_kick_tx (epn);
}

static void uca_get_descriptor (unsigned wValue, const void **data, unsigned *data_size)
{
    switch (wValue >> 8)
    {
        case USB_DT_DEVICE:
            *data = &usb_cdc_acm_desc;
            *data_size = sizeof (usb_cdc_acm_desc);
            break;

        case USB_DT_CONFIG:
            *data = &usb_cdc_acm_config;
            *data_size = sizeof (usb_cdc_acm_config);
            break;

        case USB_DT_STRING:
        {
            const usb_string_desc_t *desc = &usb_desc_strings;
            for (unsigned i = wValue & 255; i != 0; i--)
            {
                desc = (usb_string_desc_t *)((uint8_t *)desc + desc->bLength);
                if (desc->bLength == 0)
                    break;
            }

            *data = desc;
            *data_size = desc->bLength;
            break;
        }
    }
}

// Encode bRequest and bmRequestType into one 16-bit value
#define REQ_ENC(req,type) ((req) | ((type) << 8))

static inline void uca_handle_setup (const usb_setup_packet_t *setup)
{
    const void *reply = 0;
    unsigned reply_size = 0;

    uca_setup_req = REQ_ENC (setup->bRequest,
                             setup->bmRequestType & (USB_REQ_TYPE_RECP_MASK |
                                                     USB_REQ_TYPE_TYPE_MASK));
    uca_setup_data_size = MIN (EP0_RXB_SIZE, setup->wLength);

    switch (uca_setup_req)
    {
        case REQ_ENC (USB_REQ_STD_SET_ADDRESS, USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            // Set USB device address && enable on next TX IRQ
            uca_set_daddr = setup ->wValue.u8l;
            break;

        case REQ_ENC (USB_REQ_STD_GET_DESCRIPTOR, USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            uca_get_descriptor (setup->wValue.u16, &reply, &reply_size);
            break;

        case REQ_ENC (USB_REQ_STD_GET_STATUS, USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            reply = &uca_device_status;
            reply_size = 2;
            break;

        case REQ_ENC (USB_REQ_STD_SET_CONFIGURATION, USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            uca_device_config = setup->wValue.u8l;
            break;

        case USB_REQ_STD_GET_CONFIGURATION:
            reply = &uca_device_config;
            reply_size = 1;
            break;

#if USB_CDC_LINE_CODING
        // SET_LINE_CODING is handled at DATA stage

        case REQ_ENC (USB_REQ_CDC_GET_LINE_CODING, USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            reply = &uca_line_format;
            reply_size = sizeof (uca_line_format);
            break;

        case REQ_ENC (USB_REQ_CDC_SET_CONTROL_LINE_STATE, USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            uca_line_state = setup->wValue.u16;
            uca_line_state_changed ();
            break;
#endif
    }

    // Send even if reply_size == 0
    uca_send_data (0, reply, MIN (reply_size, setup->wLength));
}

static inline void uca_handle_setup_data (const void *data, unsigned data_size)
{
    switch (uca_setup_req)
    {
        case REQ_ENC (USB_REQ_CDC_SET_LINE_CODING, USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            if (data_size >= sizeof (uca_line_format))
            {
                memcpy (&uca_line_format, data, sizeof (uca_line_format));
                uca_line_format_changed ();
            }
            break;
    }
}

static inline void uca_handle_tx (unsigned epn)
{
    // If there's a pending device address change, apply it now
    if (uca_set_daddr != 0)
    {
        USB->DADDR = uca_set_daddr | USB_DADDR_EF;
        uca_set_daddr = 0;
    }

    // Fragment transmission complete, send next fragment if any
    if (uca_ep_status [epn].tx_buff)
        uca_kick_tx (epn);

    // If there's no more data to transmit, tell client
    if ((epn == EP_TX) && !uca_ep_status [epn].tx_buff)
        uca_transmitted ();
}

static inline void uca_handle_ctr (unsigned epn)
{
    uint32_t EPxR = USBEPxR (epn);

    if (EPxR & USB_EP_CTR_RX)
    {
        // Move data from packet buffers to SRAM
        void *data;
        unsigned data_size;
        uca_move_from_pma (epn, &data, &data_size);

        // Ready to receive next packet, set EP receiver state to VALID
        //
        // USB_EP_RX_(DIS|STALL|NAK|VALID) receiver state bits should
        // be toggled by writing '1' to corresponding location, and '0'
        // will leave the bit in the same state (see refman).
        //
        // The XOR operation ensures that '1' will be written only
        // to the bits that must change their value, and '0' to bits that
        // should keep their state.
        //
        // Compare with PCD_SET_EP_(RX|TX|RXTX)_STATUS from SPDL/CUBE, hehe.
        USBEPxR (epn) = (EPxR ^ USB_EP_RX_VALID) &
                ((USB_EPREG_MASK | USB_EPRX_STAT) & ~USB_EP_CTR_RX);

        switch (epn)
        {
            case EP_CTRL:
                if (EPxR & USB_EP0R_SETUP)
                {
                    if (data_size >= sizeof (usb_setup_packet_t))
                        uca_handle_setup ((usb_setup_packet_t *)data);
                }
                else
                    uca_handle_setup_data (data, data_size);
                break;

            case EP_RX:
                // Got data? Send to user function.
                uca_received (data, data_size);
                break;
        }
    }

    else if (EPxR & USB_EP_CTR_TX)
    {
        // Acknowledge reception of 'TX finished' condition
        USBEPxR (epn) = EPxR & (USB_EPREG_MASK & ~USB_EP_CTR_TX);

        // Send more, if needed
        uca_handle_tx (epn);
    }
}

void USB_LP_CAN1_RX0_IRQHandler ()
{
    unsigned istr = USB->ISTR;

    if (istr & USB_ISTR_CTR)
    {
        // The CTR bit is readonly, will be dropped as soon as
        // pending state bits in EPxR will be cleared
        uca_handle_ctr (istr & USB_ISTR_EP_ID);
    }

    else if (istr & USB_ISTR_RESET)
    {
        // To avoid spurious clearing of some bits, it is recommended
        // to clear them with a load instruction where all bits which
        // must not be altered are written with 1, and all bits to be
        // cleared are written with 0.
        USB->ISTR = (uint16_t)~USB_ISTR_RESET;

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

    // Clear all other interrupts we don't handle yet
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
           (uca_ep_status [EP_TX].tx_buff ? 0 : UCA_STATE_TXEMPTY);
}

bool uca_transmit (const void *data, unsigned data_size)
{
    if (uca_ep_status [EP_TX].tx_buff)
        return false;

    uca_send_data (EP_TX, data, data_size);
    return true;
}

/*
 * NOTE: The following functions should NOT be modified,
 * instead, just define your own callback in your source file.
 */

__WEAK void uca_received (const void *data, unsigned length)
{
    (void)data;
    (void)length;
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

#endif
