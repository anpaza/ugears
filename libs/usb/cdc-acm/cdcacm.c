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
#define tracehex(n)         trace ((n) + (((n) < 10) ? '0' : ('a' - 10)))
#define traceif(c,cond)     { if (cond) trace (c); }

// Not every MCU has USB
#ifdef USB_BASE

// Notification flags
static uint8_t uca_change_flags;

// true if USB is active
bool uca_active = false;
// true if USB is suspended
bool uca_suspended = false;

#ifdef USB_CDC_LINE_CONTROL
// Current line state (see USB_CDC_LINE_STATE_XXX flags)
uint16_t uca_line_state = 0;
// Host-requested data format
uca_line_format_t uca_line_format = {115200, 0, 0, 8};
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

/* Static buffer for allocating rx buffers */
static uint8_t uca_alloc_buff [EP_CTL_PKT_SIZE + EP_INT_PKT_SIZE + EP_DATA_PKT_SIZE];
/* Saved last setup request */
static usb_setup_packet_t uca_setup;
static bool uca_setup_valid = false;

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
    uca_active = false;
    uca_suspended = false;

    uint16_t pmaddr = sizeof (USBEPBUF);
    uint8_t *alloc = uca_alloc_buff;
    unsigned epn;
    for (epn = 0; epn < EP_COUNT; epn++)
    {
        // Allocate PMA memory for transmission
        USBEPBUF [epn].tx.addr = pmaddr;
        USBEPBUF [epn].tx.count = 0;
        pmaddr += uca_ep_config [epn].tx_max;

        // Allocate PMA memory for reception
        unsigned max_size = uca_ep_config [epn].rx_max;
        USBEPBUF [epn].rx.addr = pmaddr;
        USBEPBUF [epn].rx.count = (max_size >= 64) ?
                    ((((max_size - 32) / 32) << USB_COUNT_RX_NUM_BLOCK_Pos) | USB_COUNT_RX_BLSIZE) :
                    ((max_size / 2) << USB_COUNT_RX_NUM_BLOCK_Pos);
        pmaddr += max_size;

        // Allocate memory for RX buffer
        uca_ep_status [epn].rx_buff = alloc;
        alloc += max_size;

        // Set endpoint configuration
        uint32_t EPxR = uca_ep_config [epn].epn_type;
        USBEPxR (epn) = (EPxR & USB_EPREG_MASK) |
                ((USBEPxR (epn) ^ EPxR) & (USB_EP_RX_VALID | USB_EP_TX_VALID));
    }

    // Disable all other USB peripherial endpoint machines
    for (; epn < 8; epn++)
        USBEPxR (epn) = USBEPxR (epn) ^ (USB_EP_RX_STALL | USB_EP_TX_STALL);

    // Notify user code
    uca_event (UCA_EVENT_ACTIVE | UCA_EVENT_SUSPEND);
}

static inline void uca_move_from_pma (unsigned epn, uca_ep_status_t *status)
{
    const uint16_t *src = (const uint16_t *)
            (USB_PMAADDR + USBEPBUF [epn].rx.addr * USB_PMA_ACCESS);
    uint16_t *dst = (uint16_t *)status->rx_buff;
    int count = USBEPBUF [epn].rx.count & 0x3FF;
    status->rx_len = count;
    // last packet if count < uca_ep_config [epn].rx_max
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
    {
        trace ('!');
        src = 0;
    }
    else
    {
        trace ('t');
        tracehex (count >> 4);
        tracehex (count & 15);
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
    uca_ep_status [epn].tx_active = true;

    traceif (']', !src);

    trace ('*');
    trace ('v');

    // Tell peripherial it may start to transmit data
    USBEPxR (epn) = (USBEPxR (epn) ^ USB_EP_TX_VALID) &
            (USB_EPREG_MASK | USB_EPTX_STAT);
}

static void uca_send_data (unsigned epn, const void *data, uint16_t size)
{
    trace ('[');
    uca_ep_status [epn].tx_len = size;
    uca_ep_status [epn].tx_buff = data;
    uca_kick_tx (epn);
}

static inline const void *uca_get_descriptor (unsigned *size)
{
    switch (uca_setup.wValue.u8h)
    {
        case USB_DT_DEVICE:
            *size = sizeof (usb_cdc_acm_desc);
            return &usb_cdc_acm_desc;

        case USB_DT_CONFIG:
            *size = sizeof (usb_cdc_acm_config);
            return &usb_cdc_acm_config;

        case USB_DT_STRING:
        {
            const usb_string_desc_t *desc = &usb_desc_strings;
            for (unsigned idx = uca_setup.wValue.u8l; idx != 0; idx--)
            {
                desc = (usb_string_desc_t *)((uint8_t *)desc + desc->bLength);
                if (desc->bLength == 0)
                    return NULL;
            }

            *size = desc->bLength;
            return desc;
        }
    }

    return NULL;
}

// Encode bRequest and bmRequestType into one 16-bit value
#define REQ_ENC(req,type) (((req) << 8) | (type))

static inline const void *uca_handle_setup (uca_ep_status_t *status, unsigned *size)
{
    trace ((uca_setup.bmRequestType & USB_REQ_TYPE_DIR) ? '<' : '>');
    trace (uca_setup.bRequest + 'A');

    if (uca_setup.bmRequestType & USB_REQ_TYPE_DIR)
        // IN requests should not be followed by a DATA packet
        uca_setup_valid = false;

    switch (REQ_ENC (uca_setup.bRequest, uca_setup.bmRequestType))
    {
        case REQ_ENC (USB_REQ_STD_SET_ADDRESS,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            // Set USB device address && enable on next TX IRQ
            uca_set_daddr = uca_setup.wValue.u8l;
            break;

        case REQ_ENC (USB_REQ_STD_SET_CONFIGURATION,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD):
            uca_device_config = uca_setup.wValue.u8l;
            break;

        case REQ_ENC (USB_REQ_STD_GET_CONFIGURATION,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DIR):
            *size = sizeof (uca_device_config);
            return &uca_device_config;

        case REQ_ENC (USB_REQ_STD_GET_DESCRIPTOR,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DIR):
            return uca_get_descriptor (size);

        case REQ_ENC (USB_REQ_STD_GET_STATUS,
                      USB_REQ_RECP_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DIR):
            *size = sizeof (uca_device_status);
            return &uca_device_status;

#if USB_CDC_LINE_CONTROL
        case REQ_ENC (USB_REQ_CDC_SET_LINE_CODING,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            if (status->rx_len == 0)
                // Don't invalidate uca_setup now,
                // will handle it again upon reception of a DATA packet
                return NULL;

            if (status->rx_len >= sizeof (uca_line_format_t))
            {
                memcpy (&uca_line_format, status->rx_buff, sizeof (uca_line_format));
                uca_change_flags |= UCA_EVENT_LINE_FORMAT;
            }
            break;

        case REQ_ENC (USB_REQ_CDC_GET_LINE_CODING,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_DIR):
            *size = sizeof (uca_line_format);
            return &uca_line_format;

        case REQ_ENC (USB_REQ_CDC_SET_CONTROL_LINE_STATE,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            uca_line_state = uca_setup.wValue.u16;
            uca_change_flags |= UCA_EVENT_LINE_STATE;
            break;

        case REQ_ENC (USB_REQ_CDC_SEND_BREAK,
                      USB_REQ_RECP_INTERFACE | USB_REQ_TYPE_CLASS):
            uca_change_flags |= UCA_EVENT_BREAK;
            break;
#endif
    }

    // mark uca_setup invalid
    uca_setup_valid = false;
    return NULL;
}

static inline void uca_handle_rx (uint32_t EPxR, unsigned epn, uca_ep_status_t *status)
{
    trace ('0' + epn);

    // Move data from packet buffers to SRAM
    uca_move_from_pma (epn, status);

    // Will respond with a VALID to this transaction by default.
    uca_status_t rxstat = UCA_ST_VALID;

    // Data for all endpoint types except control will be dispatched
    // immediately so that we can handle large transfers
    // (larger than uca_req.data)
    if ((EPxR & USB_EP_TYPE_MASK_Msk) == USB_EP_CONTROL)
    {
        /* !!!
         * 8.4.6.4 Function Response to a SETUP Transaction
         * ...
         * Upon receiving a SETUP token, a function must accept the data.
         * A function may not respond to a SETUP token with either STALL or NAK
         */

        // First SETUP is received, followed by DATA
        if (EPxR & USB_EP_SETUP)
        {
            uca_setup = *(usb_setup_packet_t *)status->rx_buff;
            uca_setup_valid = true;
            status->rx_len = 0;
        }

        if (uca_setup_valid)
        {
            unsigned size = 0;
            const void *data = uca_handle_setup (status, &size);

            // Reply length cannot exceed wLength (but can exceed max_pkt)
            if (size > uca_setup.wLength)
                size = uca_setup.wLength;

            // Send the answer
            uca_send_data (EP_CTL, data, size);
        }
    }
    else
    {
        if (EPxR & USB_EP_SETUP)
            // SETUP request to non-control endpoint
            rxstat = UCA_ST_STALL;
        else
        {
            trace ('r');
            tracehex (status->rx_len >> 4);
            tracehex (status->rx_len & 15);

            // Mark data as handled in the case uca_received()
            // somehow indirectly calls uca_check_receive().
            unsigned rx_len = status->rx_len;
            status->rx_len = 0;

            switch (epn)
            {
                case EP_DATA:
                    rxstat = uca_received (status->rx_buff, rx_len,
                                           rx_len < uca_ep_config [epn].rx_max);
                    //rxstat = uca_received (status->rx_buff, status->rx_len,
                    //                       status->rx_len < uca_ep_config [epn].rx_max);
                    break;

                case EP_INT:
                    // Respond with ACK (UCA_ST_VALID)
                    break;

                default:
                    // Should never happen
                    rxstat = UCA_ST_STALL;
                    break;
            }

            // If data was not accepted, restore it in our rx buffer
            if (rxstat == UCA_ST_NAK)
                status->rx_len = rx_len;
            //if (rxstat != UCA_ST_NAK)
            //    status->rx_len = 0;
        }
    }

    trace ('$');
    trace ((rxstat == UCA_ST_VALID) ? 'v' : (rxstat == UCA_ST_STALL) ? 's' : 'n');

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

static inline void uca_handle_tx (unsigned epn, uca_ep_status_t *status)
{
    trace ('0' + epn);

    // last initiated TX transaction is complete
    status->tx_active = false;

    // Fragment transmission complete, send next fragment if any
    if (status->tx_buff)
        uca_kick_tx (epn);
    else
    {
        // If there's a pending device address change, apply it now
        if (uca_set_daddr != 0)
        {
            trace ('a');
            USB->DADDR = uca_set_daddr | USB_DADDR_EF;
            uca_set_daddr = 0;
            uca_active = true;
            uca_change_flags |= UCA_EVENT_ACTIVE;
            // Do not issue any answer since the USB device address has changed
            return;
        }

        // If there's no more data to transmit, tell client to send more
        if (epn == EP_DATA)
            uca_transmitted ();
    }
}

static inline void uca_handle_ctr (unsigned epn)
{
    uca_ep_status_t *status = &uca_ep_status [epn];
    uint32_t EPxR = USBEPxR (epn);
    if (EPxR & USB_EP_CTR_RX)
    {
        uca_handle_rx (EPxR, epn, status);

        // Acknowledge RX interrupt
        EPxR = USBEPxR (epn);
        USBEPxR (epn) = EPxR & (USB_EPREG_MASK & ~USB_EP_CTR_RX);
    }

    if (EPxR & USB_EP_CTR_TX)
    {
        // Send more, if needed
        uca_handle_tx (epn, status);

        // Acknowledge TX interrupt
        USBEPxR (epn) = EPxR & (USB_EPREG_MASK & ~USB_EP_CTR_TX);
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

        trace ('r');
        uca_reset ();
    }

    else if (istr & USB_ISTR_SUSP)
    {
        USB->CNTR |= USB_CNTR_FSUSP;
        uca_suspended = true;
        uca_change_flags |= UCA_EVENT_SUSPEND;
        USB->ISTR = (uint16_t)~USB_ISTR_SUSP;
    }

    else if (istr & USB_ISTR_WKUP)
    {
        USB->CNTR &= ~USB_CNTR_FSUSP;
        uca_suspended = false;
        uca_change_flags |= UCA_EVENT_SUSPEND;
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

    // Now, after we handled high-priority tasks, we can notify user code
    if (uca_change_flags)
    {
        uca_event (uca_change_flags);
        uca_change_flags = 0;
    }
}

bool uca_transmit (const void *data, unsigned data_size)
{
    uca_ep_status_t *status = &uca_ep_status [EP_DATA];
    if (status->tx_buff || status->tx_active)
        return false;

    if (data)
        uca_send_data (EP_DATA, data, data_size);
    return true;
}

void uca_check_receive ()
{
    uca_ep_status_t *status = &uca_ep_status [EP_DATA];
    if (status->rx_len == 0)
        return;

    uca_status_t rxstat = uca_received (status->rx_buff, status->rx_len,
                                        status->rx_len < uca_ep_config [EP_DATA].rx_max);

    // If data was accepted or rejected, mark it invalid
    if (rxstat != UCA_ST_NAK)
        status->rx_len = 0;

    trace ('$');
    trace ((rxstat == UCA_ST_VALID) ? 'v' : (rxstat == UCA_ST_STALL) ? 's' : 'n');

    uint16_t mask = (rxstat << USB_EPRX_STAT_Pos);
    USBEPxR (EP_DATA) = (USBEPxR (EP_DATA) ^ mask) &
            (USB_EPREG_MASK | USB_EPRX_STAT);
}

/*
 * NOTE: The following functions should NOT be modified,
 * instead, just define your own callback in your source file.
 */

__WEAK uca_status_t uca_received (const void *data, unsigned length, bool last)
{
    (void)data;
    (void)length;
    (void)last;
    return UCA_ST_VALID;
}

__WEAK void uca_transmitted ()
{
}

__WEAK void uca_event (unsigned flags)
{
    (void)flags;
}

#endif // USB_BASE
