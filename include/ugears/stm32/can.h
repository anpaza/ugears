/*
    STM32 low-level CAN library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_CAN_H
#define _STM32_CAN_H

/**
 * @file can.h
 *      This library makes easier to initialize and use the bxCAN
 *      peripherial of STM32. Yes, we CAN!
 */

#include "cmsis.h"
#include <useful/useful.h>

/// Uniform CAN peripherial access by number
#if defined CAN_BASE && !defined CAN1
#define CAN1                    ((CAN_TypeDef *) CAN_BASE)
#endif

#if defined CAN1 || defined CAN2 || defined CAN3

#define HAS_CAN

#undef CAN

/**
 * Get a pointer to the CAN controller associated with given feature.
 * Example: CAN(SENSOR_BUS) will expand into something like CAN1
 */
#define CAN(x)			JOIN2 (CAN, CAN_NUM (x))
/// Return the CAN controller number (1, 2, ...) given hardware feature name
#define CAN_NUM(x)		JOIN2 (x, _CAN_NUM)
/// Return the CAN channel number (0-17) given hardware feature name
#define CAN_CHAN(x)		JOIN2 (x, _CAN_CHAN)
/// Guess CAN clock frequency by hardware feature name
#define CAN_CLOCK_FREQ(x)	CLOCK_FREQ (JOIN2 (_CAN, CAN_NUM (x)))

// Bit definitions for 'mode' parameter of can_init()

/// Declare the bus baud rate
#define CAN_MODE_BITRATE(x)	((x & CAN_MODE_BITRATE_MASK) << CAN_MODE_BITRATE_SHIFT)
#define CAN_MODE_BITRATE_MASK	0xfffff
#define CAN_MODE_BITRATE_SHIFT	0
/// We want valid timestamps (time triggered communication mode)
#define CAN_MODE_TIMESTAMPS	0x00100000
/// Set to automatically recover from bus-off error condition
#define CAN_MODE_BUSOFF_RECOV	0x00200000
/// Wake up from SLEEP mode when bus activity is detected
#define CAN_MODE_WAKEUP_ON_RECV	0x00400000
/// Do not retransmit message on errors or arbitration lost
#define CAN_MODE_NO_RETRANSMIT	0x00800000
/// If RX FIFO is full, next msg will be discarded. If not set, the oldest msg in FIFO is discarded.
#define CAN_MODE_RXFIFO_LOCKED	0x01000000
/// If set, send msgs in the order they are loaded into FIFO; otherwise, sort by msg priority
#define CAN_MODE_TX_SEQ		0x02000000
/// In loopback mode RX receives what's sent through TX
#define CAN_MODE_LOOPBACK	CAN_BTR_LBKM
/// In silent mode, TX pin does not output anything
#define CAN_MODE_SILENT		CAN_BTR_SILM

/**
 * Initialize the bxCAN hardware.
 * @arg can The CAN controller to initialize
 * @arg mode A combination of CAN_MODE_XXX bitflags which describe the exact
 *      bxCAN working mode you want. For example, (CAN_MODE_BITRATE (125000) |
 *      CAN_MODE_BUSOFF_RECOV | CAN_MODE_TX_SEQ) will request a speed of
 *      125k baud, automatic recovery from BUSOFF errors and sequential
 *      transmission of loaded TX FIFOs.
 * @param bus_freq The frequency on the bus SPI device is connected to
 *      (use the CAN_CLOCK_FREQ(x) to find it)
 * @return false if something goes wrong. Note that this function does not wait
 *      for the controller to detect bus validity at the end of initialization;
 *      use can_wait_bus() if you want a separate check.
 */
EXTERN_C bool can_init (CAN_TypeDef *can, uint32_t mode, uint32_t bus_freq);

/**
 * Wait for controller initialization to finish. The controller will wait for 11 consecutive
 * recessive bits to finish initialization.
 * @arg can The CAN controller to check bus status.
 * @return true if bus ok, false if it is not.
 */
EXTERN_C bool can_wait_bus (CAN_TypeDef *can);

#ifdef CAN2
/**
 * Set up how the filter banks are split between CAN1 and CAN2.
 * @arg fbn Filter Bank Number where filters for CAN2 begin (0-28).
 *      If 0, all filters are assigned to CAN2.
 *      If 28, all filters are assigned to CAN1.
 */
EXTERN_C void can_filter_split (uint32_t fbn);
#endif

/// 32-bit filter flag
#define CAN_FILTER_32BIT	0x00000001
/// use id/mask pairs rather than id/id
#define CAN_FILTER_MASK		0x00000002
/// packet matching filter goes to FIFO1 (to FIFO0 if flag not set)
#define CAN_FILTER_FIFO1	0x00000004

/**
 * Build a single 32-bit filter register value from a 29-bit identifier
 * and a "Remote Transmission Request" flag.
 * @arg id The 29-bit or 11-bit identifier
 * @arg ide Identifier Extension, true for 29-bit identifiers
 * @arg rtr true if the filter matches Remote Transmission Request frames
 *      (queries).
 */
#define CAN_FR32(id,ide,rtr) \
	(((ide) ? (((id) << 3) | 4) : ((id) << 21)) | ((rtr) ? 2 : 0))

/**
 * Build a single 32-bit filter register value from two 11-bit identifiers
 * and two "Remote Transmission Request" flags, one for every id.
 * @arg id1 The first 29-bit or 11-bit identifier. For 29-bit identifiers,
 *      only upper 14 bits are encoded.
 * @arg ide1 First Identifier Extension, true for 29-bit identifiers
 * @arg rtr1 true if the filter matches Remote Transmission Request frames
 *      (queries).
 * @arg id2 Second 29-bit or 11-bit identifier. For 29-bit identifiers,
 *      only upper 14 bits are encoded.
 * @arg ide2 Second Identifier Extension, true for 29-bit identifiers
 * @arg rtr2 true if the filter matches Remote Transmission Request frames
 *      (queries).
 */
#define CAN_FR16(id1,ide1,rtr1,id2,ide2,rtr2) \
	(((ide1) ? ((((id1) & 0x1ffc0000) >> 13) | (((id1) & 0x00038000) >> 15) | 0x00000008) : (((id1) & 0x07ff) << 5)) | ((rtr1) ? 0x00000010 : 0)) | \
	(((ide2) ? ((((id2) & 0x1ffc0000) << 3) | (((id2) & 0x00038000) << 1) | 0x00080000) : (((id2) & 0x07ff) << 21)) | ((rtr2) ? 0x00100000 : 0))

/**
 * Clear all filters at once.
 */
EXTERN_C void can_filter_reset ();

/**
 * Disable a filter bank (which may contain from 1 to 4 filters).
 * @arg n Filter bank number
 */
EXTERN_C void can_filter_disable (uint32_t n);

/**
 * A generic function to set a filter bank.
 * This function sets up and enables the filter.
 *
 * It is recommended to use the more specialized can_filter_XX and
 * can_filter_mask_XX functions, but if you use this one, don't forget to build
 * the proper fr1 and fr2 values using either CAN_FR16 or CAN_FR32 macros.
 *
 * @arg n Filter bank number
 * @arg fr1 The first filter register (CAN_FxR1)
 * @arg fr2 Second filter register (CAN_FxR2)
 * @arg flags Filter mode flags (CAN_FILTER_XXX)
 */
EXTERN_C void can_filter (uint32_t n, uint32_t fr1, uint32_t fr2, uint32_t flags);

/**
 * Set two 32-bit filters at once.
 * @arg n Filter bank number
 * @arg id1 The first CAN identifier
 * @arg id2 The second CAN identifier
 * @arg ide Identifier Extension, true for 29-bit identifiers
 * @arg rtr true if the filter matches Remote Transmission Request frames
 *      (queries).
 * @arg fifo 0 if filter puts matching packets in FIFO0, 1 in FIFO1
 */
INLINE_ALWAYS void can_filter_32 (uint32_t n, uint32_t id1, uint32_t id2,
    bool ide, bool rtr, uint32_t fifo)
{
    can_filter (n, CAN_FR32 (id1, ide, rtr), CAN_FR32 (id2, ide, rtr),
        CAN_FILTER_32BIT | (fifo ? CAN_FILTER_FIFO1 : 0));
}

/**
 * Set a 32-bit filter/mask.
 * @arg n Filter bank number
 * @arg id CAN identifier of the packet
 * @arg mask CAN identifier mask
 * @arg ide Identifier Extension, true for 29-bit identifiers
 * @arg rtr true if the filter matches Remote Transmission Request frames
 *      (queries).
 * @arg fifo 0 if filter puts matching packets in FIFO0, 1 in FIFO1
 */
INLINE_ALWAYS void can_filter_mask_32 (uint32_t n, uint32_t id, uint32_t mask,
    bool ide, bool rtr, uint32_t fifo)
{
    can_filter (n, CAN_FR32 (id, ide, rtr), CAN_FR32 (mask, true, true),
        CAN_FILTER_32BIT | CAN_FILTER_MASK | (fifo ? CAN_FILTER_FIFO1 : 0));
}

/**
 * Set four 16-bit filters at once.
 * @arg n Filter bank number
 * @arg id1 The first identifier
 * @arg id2 The second identifier
 * @arg id3 Third identifier
 * @arg id4 Fourth identifier
 * @arg ide Identifier Extension, true for 29-bit identifiers
 * @arg rtr true if the filter matches Remote Transmission Request frames
 *      (queries).
 * @arg fifo 0 if filter puts matching packets in FIFO0, 1 in FIFO1
 */
INLINE_ALWAYS void can_filter_16 (uint32_t n, uint32_t id1, uint32_t id2,
    uint32_t id3, uint32_t id4, bool ide, bool rtr, uint32_t fifo)
{
    can_filter (n, CAN_FR16 (id1, ide, rtr, id2, ide, rtr),
        CAN_FR16 (id3, ide, rtr, id4, ide, rtr),
        (fifo ? CAN_FILTER_FIFO1 : 0));
}

/**
 * Set two 16-bit filters/masks at once.
 * @arg n Filter bank number
 * @arg id1 The first identifier
 * @arg mask1 The mask for first identifier
 * @arg id2 The second identifier
 * @arg mask2 The mask for second identifier
 * @arg ide Identifier Extension, true for 29-bit identifiers
 * @arg rtr true if the filter matches Remote Transmission Request frames
 *      (queries).
 * @arg fifo 0 if filter puts matching packets in FIFO0, 1 in FIFO1
 */
INLINE_ALWAYS void can_filter_mask_16 (uint32_t n, uint32_t id1, uint32_t mask1,
    uint32_t id2, uint32_t mask2, bool ide, bool rtr, uint32_t fifo)
{
    can_filter (n, CAN_FR16 (id1, ide, rtr, mask1, ide, rtr),
        CAN_FR16 (id2, ide, rtr, mask2, ide, rtr),
        CAN_FILTER_MASK | (fifo ? CAN_FILTER_FIFO1 : 0));
}

#endif // CAN1 || CAN2 || CAN3

#endif // _STM32_CAN_H
