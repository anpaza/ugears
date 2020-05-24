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

#include "ugears.h"
#include "usefun.h"
#include "usb-cdc.h"

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
#ifndef USB_CDC_LINE_CODING
#  define USB_CDC_LINE_CODING           0
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
 * Return the current state of the USB CDC ACM driver as a bitmask.
 * @return see UCA_STATE_XXX flags
 */
extern unsigned uca_state ();

/** Bitflags for uca_state() return value */
typedef enum
{
    /// Set if USB peripherial is enabled
    UCA_STATE_ENABLED = 0x00000001,
    /// Set if USB peripherial is suspended
    UCA_STATE_SUSPENDED = 0x00000002,
    /// Set if device is configured (used by host)
    UCA_STATE_CONFIGURED = 0x00000004,
    /// Set if transmit queue is empty
    UCA_STATE_TXEMPTY = 0x00000008,
} uca_state_t;

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
 * @return false if transmitter is busy
 */
extern bool uca_transmit(const void *data, unsigned data_size);

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
 * bulk endpoints. You can call ucaTransmit () to send data back
 * to the host in this time slot.
 *
 * @param data Pointer to received data
 * @param data_size Received data size
 * @return Transaction status. Return VALID on success, NAK to tell host
 *      device isn't ready to accept data yet and retry later.
 */
extern uca_status_t uca_received (const void *data, unsigned data_size);

#ifdef USB_CDC_LINE_CODING

/** Current line state (see USB_CDC_LINE_STATE_XXX flags) */
extern uint16_t uca_line_state;
/** Host-requested data format */
extern uca_line_format_t uca_line_format;

/**
 * Called when host changes uca_line_state
 */
extern void uca_line_state_changed ();

/**
 * Called when host changes uca_line_format
 */
extern void uca_line_format_changed ();

/**
 * Received a BREAK signal
 */
extern void uca_line_break ();

#endif

/**
 * Redirect printf, putc, puts via the USB CDC ACM device.
 * This uses the printf routines from libuseful.
 * Very useful for debugging.
 */
extern void uca_printf ();

#endif /* _USBB_CDC_ACM_H */
