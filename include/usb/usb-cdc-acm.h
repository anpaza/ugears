/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

/*
 * This library was inspired by STM32F103-USB-CDC-CMSIS:
 *
 * https://github.com/saewave/STM32F103-USB-CDC-CMSIS).
 * Copyright (c) 2017 Samoilov Alexey.
 *
 * But was rewritten from scratch to make it possible to use
 * the Apache license.
 */

/*
 * You can use the following macros in your hardware configuration file
 * to influence the USB CDC ACM implementation:
 *
 * @li USB_CDC_VENDOR_ID, USB_CDC_PRODUCT_ID - default to 0x1209:0x0001.
 *     These VID:PID codes are kindly provided by http://pid.codes/
 *     for testing purposes only. If you're going to spread your device,
 *     please go to the above site and request a unique PID for your device.
 *     It's easy and it's FREE!
 * @li USB_CDC_STR_MANUFACTURER, USB_CDC_STR_PRODUCT, USB_CDC_STR_SERIALNUMBER -
 *     define these to the respective USB strings to be reported to the
 *     operating system. You may omit any (or all) of them.
 *     They must be in UTF-16 encoding, e.g. use the 'u' prefix:
 *     u"BogusCompany", u"ExclusiveProduct" and u"1234-5678".
 * @li USB_CDC_STR_LANGID - the language of your device strings. Currently
 *     only a single language is supported, and LANG_US is the default.
 * @li USB_CDC_POWER_MA - define this either to 0 if your device is self-powered,
 *     or to max amount of milliamperes consumed from the USB bus.
 * @li USB_CDC_LINE_CODING - define this to 1 if your device needs support for
 *     Get/SetLineConfig() to change serial port speed & byte format
 * @li USB_IRQ_PRIO - define this to NVIC priority of USB IRQ,
 *     if you don't like the default valaue.
 */

#ifndef _USBB_CDC_ACM_H
#define _USBB_CDC_ACM_H

#include <ugears/ugears.h>
#include <usb/usb-cdc.h>

/* Default VID:PID codes */
#ifndef USB_CDC_VENDOR_ID
#  define USB_CDC_VENDOR_ID		0x1209
#endif
#ifndef USB_CDC_PRODUCT_ID
#  define USB_CDC_PRODUCT_ID		0x0001
#endif

#define LANG_US				0x0409

#ifndef USB_CDC_STR_LANGID
#  define USB_CDC_STR_LANGID		LANG_US
#endif

/* Define in your hardware.h
#define USB_CDC_STR_MANUFACTURER	u"..."
#define USB_CDC_STR_PRODUCT		u"..."
#define USB_CDC_STR_SERIALNUMBER	u"..."
*/

/* Define to 0 (self-powered) or current in ma consumed from USB bus */
#ifndef USB_CDC_POWER_MA
#  define USB_CDC_POWER_MA              100
#endif

/* Define to device version number, encoded in BCD format */
#ifndef USB_CDC_VER
#  define USB_CDC_VER                   USB_BCD_VER (0,0)
#endif

/* Define to 1 to receive baud rate & byte format messages */
#ifndef USB_CDC_LINE_CONTROL
#  define USB_CDC_LINE_CONTROL          0
#endif

#ifndef USB_IRQ_PRIO
#  define USB_IRQ_PRIO                  128
#endif

#pragma pack(push,1)

/** Line data format as defined in "Universal Serial Bus
    Communications Class Subclass Specification for PSTN Devices" */
typedef struct
{
    /* Data Terminal Rate, bits per second */
    uint32_t dwDTERate;
    /* 0: 1 stop bit, 1: 1.5 stop bits, 2: 2 stop bits */
    uint8_t  bCharFormat;
    /* 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space */
    uint8_t  bParityType;
    /* Number of data bits: 5, 6, 7, 8 or 16 */
    uint8_t  bDataBits;
} uca_line_format_t;

#pragma pack(pop)

/** Transaction status: the values are same as shifted USB_EP_(RX|TX)_XXX */
typedef enum
{
    /** Endpoint is disabled */
    UCA_ST_DIS   = 0,
    /** Endpoint is temporarily unavailable */
    UCA_ST_STALL = 1,
    /** Endpoint cannot temporarily accept data, try later (flow control) */
    UCA_ST_NAK   = 2,
    /** Operation has been successfully completed */
    UCA_ST_VALID = 3,
} uca_status_t;

/**
 * Initialize the USB CDC ACM device.
 */
extern void uca_init ();

/**
 * Transmit data via the bulk endpoint.
 *
 * If transmitter is busy (previous uca_transmit() was not finished
 * yet with an uca_transmitted()), function will return false.
 * You may try transmitting your data at a later time.
 *
 * If data_size exceeds endpoint max transmission size, data will be
 * sent in several frames. Until data is transmitted the data should
 * stay in the buffer!
 *
 * At end of transmission uca_transmited() is called.
 *
 * @param data Pointer to transmitted data
 * @param data_size Transmitted data size
 * @return false if transmitter is busy, true if it is not,
 *      even if data is NULL
 */
extern bool uca_transmit (const void *data, unsigned data_size);

/**
 * Redirect printf, putc, puts via the USB CDC ACM device.
 * This uses the printf routines from libuseful.
 * Very useful for debugging.
 */
extern void uca_printf ();

/* ------------------------------------------------- *\
 * User handlers for USB CDC ACM events:             *
 * define functions with this signature in your code *
 *                                                   *
 * There are default weak implementations provided,  *
 * so only the needed callbacks may be defined.      *
\* ------------------------------------------------- */

/**
 * This callback is invoked when previous data was sent,
 * and you have a chance to send more.
 */
extern void uca_transmitted ();

/**
 * Called from interrupt context when data is received via the
 * bulk endpoints. This is invoked on every packet received
 * (USB CDC ACM data endpoint max pkt size is 64 bytes).
 * When last data packet is received, @a last will be set to true.
 * It is possible that @a data_size is 0, and @a last is true,
 * if unsplit data size is a multiple of max pkt size.
 *
 * @param data Pointer to received data
 * @param data_size Received data size
 * @param last True if this is last packet.
 * @return Transaction status. Return VALID on success, NAK to tell host
 *      device isn't ready to accept data yet. In the later case you have
 *      to call uca_check_receive later, when you're finally ready to
 *      accept data.
 */
extern uca_status_t uca_received (const void *data, unsigned data_size, bool last);

/** Packets feed to uca_received are guaranteed to be less than this size */
#define UCA_RECEIVED_MAX                64

/**
 * If your code can report NAK from uca_received, then you must call this
 * function when it is finally ready to accept more data. If you don't do this,
 * the endpoint will lock up either until you call it, or a packet for
 * another endpoint comes, in which case the DATA endpoint will be un-NAK-ed,
 * and data will be lost.
 *
 * This function may invoke uca_received (if buffer contains valid data).
 */
extern void uca_check_receive ();

/** USB CDC ACM events, sent to user code as bitmask */
typedef enum
{
    /// USB has been in/activated (value in uca_active)
    UCA_EVENT_ACTIVE = (1 << 0),
    /// USB has been suspended/resumed (state in uca_suspended)
    UCA_EVENT_SUSPEND = (1 << 1),
    /// Line state changed (value in uca_line_state)
    UCA_EVENT_LINE_STATE = (1 << 2),
    /// Line data format changed (value in uca_line_format)
    UCA_EVENT_LINE_FORMAT = (1 << 3),
    /// Users requests to send a BREAK metacharacter
    UCA_EVENT_BREAK = (1 << 4),
} uca_event_t;

/**
 * Called to notify user code of events.
 * @param flags A bitmask of ORed uca_event_t flags.
 */
extern void uca_event (unsigned flags);

// true if USB is active
extern bool uca_active;
// true if USB is suspended
extern bool uca_suspended;

#ifdef USB_CDC_LINE_CONTROL

/** Current line state (see USB_CDC_LINE_STATE_XXX flags) */
extern uint16_t uca_line_state;
/** Host-requested data format */
extern uca_line_format_t uca_line_format;

#endif

#endif /* _USBB_CDC_ACM_H */
