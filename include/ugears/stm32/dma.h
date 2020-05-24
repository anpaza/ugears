/*
    STM32 DMA library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _DMA_H
#define _DMA_H

/**
 * @file dma.h
 *      This is a set of routines that simplify DMA usage.
 *      Before starting using DMA you must enable DMA controller power
 *      by using the appropiate macros from rcc.h.
 *
 * The following macros are expected to be defined in your HARDWARE_H
 * in order to use DMA functionality:
 *
 * @li HWFN_DMA_NUM - defines the index of DMA controller used for
 *      hardware feature HWFN (1, 2, ...).
 *      Used by DMA_NUM(HWFN) macro.
 * @li HWFN_DMA_CHAN - defines the DMA channel number used for hardware
 *      feature HWFN (1, 2, ...).
 *      Used by DMA_CHAN(HWFN) macro.
 * @li HWFN_DMA_IRQ_PRIO - defines the IRQ priority for DMA IRQ used
 *      for hardware feature HWFN (0..255).
 *      Used by DMA_IRQ_PRIO(HWFN) macro.
 *
 * Example:
 * @code
 * // Definition for USART1 TX DMA transfers
 * #define SERIAL_TX_DMA_NUM		1
 * #define SERIAL_TX_DMA_CHAN		4
 * #define SERIAL_TX_DMA_IRQ_PRIO	0
 * @endcode
 */

#include HARDWARE_H
#include "useful.h"

// There are at least three different types of GPIO peripherial
#if defined STM32F0 || defined STM32F1 || defined STM32F3
#  define DMA_TYPE_1
#elif defined STM32F2 || defined STM32F4
#  define DMA_TYPE_2
#else
#  error "Please define the correct DMA type for your MCU"
#endif

/// Return the DMA controller number (1, 2, ...) given hardware feature name
#define DMA_NUM(x)		JOIN2 (x, _DMA_NUM)
/// Return the DMA channel number (1-7) given hardware feature name
#define DMA_CHAN(x)		JOIN2 (x, _DMA_CHAN)
/// Return the IRQ number corresponding to this DMA channel
#define DMA_IRQ(x)		JOIN5 (DMA, DMA_NUM(x), _Channel, DMA_CHAN (x), _IRQn)
/// Return the DMA IRQ priority corresponding to this hardware feature
#define DMA_IRQ_PRIO(x)		JOIN2 (x, _DMA_IRQ_PRIO)

/**
 * DMA IRQ handler function name for given hardware feature.
 *
 * Optionally, a second suffix argument may be used, in which case
 * DMA handler is shared and must be called from the "main" DMA IRQ
 * handler (which should be declared without suffix).
 */
#define DMA_IRQ_HANDLER(x,...) \
JOIN6 (DMA, DMA_NUM (x), _Channel, DMA_CHAN (x), _IRQHandler, __VA_ARGS__)

/**
 * Get a pointer to the DMA controller associated with given feature.
 * Example: DMA(USART1_TX) will expand into something like DMA1
 */
#define DMA(x)			JOIN2 (DMA, DMA_NUM (x))

/**
 * Get a pointer to DMA controller channel associated with given feature.
 * Example: DMAC(USART1_TX) will expand into something like DMA1_Channel4
 */
#define DMAC(x)			JOIN4 (DMA, DMA_NUM (x), _Channel, DMA_CHAN (x))

/**
 * Invoke dma_copy(1|2|...)() using hardware feature name to determine
 * DMA and channel number.
 */
#define DMA_COPY(x, ccr, src, dst, count) \
    JOIN3 (dma, DMA_NUM (x), _copy) (DMA_CHAN (x), ccr, src, dst, count)

/**
 * Stop the DMA channel associated with hw feature name.
 */
#define DMA_STOP(x) \
    JOIN3 (dma, DMA_NUM (x), _stop) (DMA_CHAN (x));

#if defined DMA_TYPE_1

#define  DMA_CCR_PSIZE_8	0				/*!< Peripheral size 8 bits */
#define  DMA_CCR_PSIZE_16	DMA_CCR_PSIZE_0			/*!< Peripheral size 16 bits */
#define  DMA_CCR_PSIZE_32	DMA_CCR_PSIZE_1			/*!< Peripheral size 32 bits */

#define  DMA_CCR_MSIZE_8	0				/*!< Memory size 8 bits */
#define  DMA_CCR_MSIZE_16	DMA_CCR_MSIZE_0			/*!< Memory size 16 bits */
#define  DMA_CCR_MSIZE_32	DMA_CCR_MSIZE_1			/*!< Memory size 32 bits */

#define  DMA_CCR_PL_LOW		0				/*!< Channel Priority level is low */
#define  DMA_CCR_PL_MED		DMA_CCR_PL_0			/*!< Channel Priority level is medium */
#define  DMA_CCR_PL_HIGH	DMA_CCR_PL_1			/*!< Channel Priority level is high */
#define  DMA_CCR_PL_VERYHIGH	(DMA_CCR_PL_1 | DMA_CCR_PL_0)	/*!< Channel Priority level is very high */

/**
 * DMA interrupt status flag (ISR global DMA register)
 * Example: DMA_ISR (USART1_TX, GIF) -> DMA_ISR_GIF4
 */
#define DMA_ISR(x,f)		JOIN3 (DMA_ISR_, f, DMA_CHAN (x))

/**
 * DMA interrupt status clear flag (IFCR global DMA register)
 * Example: DMA_IFCR (USART1_TX, CGIF) -> DMA_IFCR_CGIF4
 */
#define DMA_IFCR(x,f)		JOIN3 (DMA_IFCR_, f, DMA_CHAN (x))

/**
 * DMA channel configuration flag (DMA_CCR)
 * Example: DMA_CCR (USART1_TX, MEM2MEM) -> DMA_CCR4_MEM2MEM
 */
#define DMA_CCR(x,f)		JOIN4 (DMA_CCR, DMA_CHAN(x), _, f)

/**
 * Copy data using DMA controller.
 * You must set up the peripherial you're reading from/writing to prior to invoking
 * this function.
 * @arg dma
 *      The DMA controller to use (DMA1, DMA2, ...)
 * @arg chan
 *      DMA channel number (counting from 1)
 * @arg ccr
 *      A combination of DMA_CCR flags. You must set only *IE, CIRC, PSIZE*, MSIZE*
 *      and PL* bits, the rest are computed and set automatically.
 * @arg src
 *      The source address (memory or peripherial)
 * @arg dst
 *      The destination address (memory or peripherial)
 * @arg count
 *      Number of copied elements
 */
extern void dma_copy (DMA_TypeDef *dma, unsigned chan, uint32_t ccr,
    volatile void *src, volatile void *dst, unsigned count);

/**
 * This function is identical to @a dma_copy except that it works specifically with the
 * DMA1 controller. You may use it for microcontrollers with just one DMA controller
 * or if you don't need the second controller.
 * @arg chan
 *      DMA channel number (counting from 1)
 * @arg ccr
 *      A combination of DMA_CCR flags. You must set only *IE, CIRC, PSIZE*, MSIZE*
 *      and PL* bits, the rest are computed and set automatically.
 * @arg src
 *      The source address (memory or peripherial)
 * @arg dst
 *      The destination address (memory or peripherial)
 * @arg count
 *      Number of copied elements
 */
extern void dma1_copy (unsigned chan, uint32_t ccr,
    volatile void *src, volatile void *dst, unsigned count);

/**
 * This function is identical to @a dma_copy except that it works specifically with the
 * DMA2 controller. You may use it if you're using DMA2 extensively and you don't need
 * the general form, this will save a bit of code size by not passing a pointer to the
 * DMA controller.
 * @arg chan
 *      DMA channel number (counting from 1)
 * @arg ccr
 *      A combination of DMA_CCR flags. You must set only *IE, CIRC, PSIZE*, MSIZE*
 *      and PL* bits, the rest are computed and set automatically.
 * @arg src
 *      The source address (memory or peripherial)
 * @arg dst
 *      The destination address (memory or peripherial)
 * @arg count
 *      Number of copied elements
 */
extern void dma2_copy (unsigned chan, uint32_t ccr,
    volatile void *src, volatile void *dst, unsigned count);

/**
 * Stop the DMA channel.
 * @arg dma
 *      The DMA controller to stop (DMA1, DMA2, ...)
 * @arg chan
 *      DMA channel number (counting from 1)
 */
extern void dma_stop (DMA_TypeDef *dma, unsigned chan);

/**
 * Stop the DMA1 channel.
 * @arg chan
 *      DMA channel number (counting from 1)
 */
extern void dma1_stop (unsigned chan);

/**
 * Stop the DMA2 channel.
 * @arg chan
 *      DMA channel number (counting from 1)
 */
extern void dma2_stop (unsigned chan);

#endif // DMA_TYPE_1

#endif // _DMA_H
