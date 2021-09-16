/*
    STM32 DMA library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_DMA_H
#define _STM32_DMA_H

/**
 * @file dma.h
 *      This is a set of routines that simplify DMA usage.
 *      Before starting using DMA you must enable DMA controller power
 *      by using the appropiate macros from rcc.h.
 *
 * Note that there was a terminology change when DMA controller was
 * upgraded from earlier versions (F0, F1, F3): what was earlier named
 * "channels" later are named "streams", and later "channels" are
 * referencing to one of 8 selectable DMA request sources at the entry
 * of every stream.
 *
 * That is, in STM32F1 DMA1 had 7 channels (numbered from 1 to 7) and,
 * for example, USART1 TX request was connected to channel 4, and USART1 RX
 * was connected to channel 5.
 *
 * In STM32F4 DMA always have 8 streams (numbered from 0 to 7) and every stream
 * has 8 input channels from up to 8 different peripherials. You choose one
 * of the channels when setting up the stream, and then it works just like
 * in earlier versions.
 *
 * So, we will stick to the newer terminology and will call "channels" of the
 * earlier DMA models "streams". And "channels" refer to the selectable DMA
 * request at the entry of every DMA stream for later DMA versions.
 *
 * The following macros are expected to be defined in your HARDWARE_H
 * in order to use DMA functionality:
 *
 * @li HWFN_DMA_NUM - defines the index of DMA controller used for
 *      hardware feature HWFN (1, 2, ...).
 *      Used by DMA_NUM(HWFN) macro.
 * @li HWFN_DMA_STRM - defines the DMA stream number used for hardware
 *      feature HWFN (1, 2, ...).
 *      Used by DMA_STRM(HWFN) macro.
 * @li HWFN_DMA_CHAN - defines the DMA channel number used for hardware
 *      feature HWFN (0 to 7). This is used only in DMA_TYPE_2
 *      versions of the DMA controller.
 * @li HWFN_DMA_IRQ_PRIO - defines the IRQ priority for DMA IRQ used
 *      for hardware feature HWFN (0..255).
 *      Used by DMA_IRQ_PRIO(HWFN) macro.
 *
 * Example:
 * @code
 * // STM32F1: definition for USART1 TX DMA transfers
 * #define SERIAL_TX_DMA_NUM		1
 * #define SERIAL_TX_DMA_STRM		4
 * #define SERIAL_TX_DMA_IRQ_PRIO	0
 *
 * // STM32F4: definition for USART1 TX DMA transfers
 * #define SERIAL_TX_DMA_NUM		2
 * #define SERIAL_TX_DMA_STRM		7
 * #define SERIAL_TX_DMA_CHAN		4
 * #define SERIAL_TX_DMA_IRQ_PRIO	0
 * @endcode
 */

#include "cmsis.h"
#include <useful/useful.h>

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
/// Get the DMA stream number (0-7) given hardware feature name
#define DMA_STRM(x)		JOIN2 (x, _DMA_STRM)

/**
 * Get a pointer to the DMA controller associated with given feature.
 * Example: DMA(USART1_TX) will expand into something like DMA1
 */
#define DMA(x)			JOIN2 (DMA, DMA_NUM (x))

#if defined DMA_TYPE_1

/// Return the IRQ number corresponding to this DMA stream
#define DMA_IRQ(x)		JOIN5 (DMA, DMA_NUM(x), _Channel, DMA_STRM (x), _IRQn)
/// Return the IRQ handler name corresponding to this DMA channel
#define DMA_IRQ_HANDLER(x)	JOIN5 (DMA, DMA_NUM(x), _Channel, DMA_STRM (x), _IRQHandler)

/**
 * Get a pointer to DMA controller stream associated with given feature.
 * Example: DMAS(USART1_TX) will expand into something like DMA1_Channel4
 */
#define DMAS(x)			JOIN4 (DMA, DMA_NUM (x), _Channel, DMA_STRM (x))

#elif defined DMA_TYPE_2

/// Get the DMA channel number (0-7) given hardware feature name
#define DMA_CHAN(x)		JOIN2 (x, _DMA_CHAN)
/// Return the IRQ number corresponding to this DMA channel
#define DMA_IRQ(x)		JOIN5 (DMA, DMA_NUM(x), _Stream, DMA_STRM (x), _IRQn)
/// Return the IRQ handler name corresponding to this DMA channel
#define DMA_IRQ_HANDLER(x)	JOIN5 (DMA, DMA_NUM(x), _Stream, DMA_STRM (x), _IRQHandler)

/**
 * Get a pointer to DMA controller stream associated with given feature.
 * Example: DMAS(USART1_TX) will expand into something like DMA1_Stream5
 */
#define DMAS(x)			JOIN4 (DMA, DMA_NUM (x), _Stream, DMA_STRM (x))

#endif

/// Return the DMA IRQ priority corresponding to this hardware feature
#define DMA_IRQ_PRIO(x)		JOIN2 (x, _DMA_IRQ_PRIO)
/// Guess DMA clock frequency by hardware feature name
#define DMA_CLOCK_FREQ(x)	CLOCK_FREQ (JOIN2 (_DMA, DMA_NUM (x)))

/**
 * Invoke dma_copy(1|2|...)() using hardware feature name to determine
 * DMA and stream number.
 */
#define DMA_COPY(x, ccr, src, dst, count) \
    JOIN3 (dma, DMA_NUM (x), _copy) (DMA_STRM (x), ccr, src, dst, count)

/**
 * Stop the DMA stream associated with hw feature name.
 */
#define DMA_STOP(x) \
    JOIN3 (dma, DMA_NUM (x), _stop) (DMA_STRM (x));

/* Since DMA transfer functions require specifying Control Register (CR),
 * the following defines can be used to specify transfer modes simpler
 * and comprehensible.
 */

#if defined DMA_TYPE_1

// Peripheral size

#define  DMA_CCR_PSIZE_8	0				/*!< Peripheral size 8 bits */
#define  DMA_CCR_PSIZE_16	DMA_CCR_PSIZE_0			/*!< Peripheral size 16 bits */
#define  DMA_CCR_PSIZE_32	DMA_CCR_PSIZE_1			/*!< Peripheral size 32 bits */

// Memory size

#define  DMA_CCR_MSIZE_8	0				/*!< Memory size 8 bits */
#define  DMA_CCR_MSIZE_16	DMA_CCR_MSIZE_0			/*!< Memory size 16 bits */
#define  DMA_CCR_MSIZE_32	DMA_CCR_MSIZE_1			/*!< Memory size 32 bits */

// Channel priority level

#define  DMA_CCR_PL_LOW		0				/*!< Channel Priority level is low */
#define  DMA_CCR_PL_MED		DMA_CCR_PL_0			/*!< Channel Priority level is medium */
#define  DMA_CCR_PL_HIGH	DMA_CCR_PL_1			/*!< Channel Priority level is high */
#define  DMA_CCR_PL_VERYHIGH	(DMA_CCR_PL_1 | DMA_CCR_PL_0)	/*!< Channel Priority level is very high */

/**
 * DMA interrupt status flag (ISR global DMA register)
 * Example: DMA_ISR_IF (USART1_TX, G) -> DMA_ISR_GIF4
 * @param f One of G, TC, HT, TE
 * @param x Hardware feature name
 */
#define DMA_ISR_IF(f,x)		JOIN4 (DMA_ISR_, f, IF, DMA_STRM (x))

/**
 * DMA interrupt status clear flag (IFCR global DMA register)
 * Example: DMA_IFCR_IF (USART1_TX, G) -> DMA_IFCR_CGIF4
 * @param f One of G, TC, HT, TE
 * @param x Hardware feature name
 */
#define DMA_IFCR_IF(f,x)	JOIN4 (DMA_IFCR_C, f, IF, DMA_STRM (x))

#elif defined DMA_TYPE_2

// Memory burst transfer configuration

#define DMA_SxCR_MBURST_SINGLE	0				/*!< single transfer */
#define DMA_SxCR_MBURST_INCR4	DMA_SxCR_MBURST_0		/*!< incremental burst of 4 beats */
#define DMA_SxCR_MBURST_INCR8	DMA_SxCR_MBURST_1		/*!< incremental burst of 8 beats */
#define DMA_SxCR_MBURST_INCR16	(DMA_SxCR_MBURST_0 | DMA_SxCR_MBURST_1) /*!< incremental burst of 16 beats */

// Peripheral burst transfer configuration

#define DMA_SxCR_PBURST_SINGLE	0				/*!< single transfer */
#define DMA_SxCR_PBURST_INCR4	DMA_SxCR_PBURST_0		/*!< incremental burst of 4 beats */
#define DMA_SxCR_PBURST_INCR8	DMA_SxCR_PBURST_1		/*!< incremental burst of 8 beats */
#define DMA_SxCR_PBURST_INCR16	(DMA_SxCR_PBURST_0 | DMA_SxCR_PBURST_1) /*!< incremental burst of 16 beats */

// Priority level

#define  DMA_SxCR_PL_LOW	0				/*!< Channel Priority level is low */
#define  DMA_SxCR_PL_MED	DMA_SxCR_PL_0			/*!< Channel Priority level is medium */
#define  DMA_SxCR_PL_HIGH	DMA_SxCR_PL_1			/*!< Channel Priority level is high */
#define  DMA_SxCR_PL_VERYHIGH	(DMA_SxCR_PL_1 | DMA_SxCR_PL_0)	/*!< Channel Priority level is very high */

// Memory data size

#define DMA_SxCR_MSIZE_8	0				/*!< byte (8-bit) */
#define DMA_SxCR_MSIZE_16	DMA_SxCR_MSIZE_0		/*!< half-word (16-bit) */
#define DMA_SxCR_MSIZE_32	DMA_SxCR_MSIZE_1		/*!< word (32-bit) */

// Peripheral data size

#define DMA_SxCR_PSIZE_8	0				/*!< byte (8-bit) */
#define DMA_SxCR_PSIZE_16	DMA_SxCR_PSIZE_0		/*!< half-word (16-bit) */
#define DMA_SxCR_PSIZE_32	DMA_SxCR_PSIZE_1		/*!< word (32-bit) */

// Data transfer direction

#define DMA_SxCR_DIR_P2M	0				/*!< Peripheral-to-memory */
#define DMA_SxCR_DIR_M2P	DMA_SxCR_DIR_0			/*!< Memory-to-peripheral */
#define DMA_SxCR_DIR_M2M	DMA_SxCR_DIR_1			/*!< Memory-to-memory */

#undef DMA_SxCR_CHSEL
/**
 * Return channel selector for a DMA stream.
 * This is useful to build the CR value when invoking dma_start().
 *
 * @param n Active channel number (0-7)
 */
INLINE_ALWAYS uint32_t dma_SxCR_chsel (uint32_t n)
{ return ((n) << DMA_SxCR_CHSEL_Pos) & DMA_SxCR_CHSEL_Msk; }

/**
 * Similar to DMA_SxCR_CHSEL_, but uses hardware feature name to pick the
 * respective HWFN_DMA_CHAN value.
 *
 * @param x Hardware feature name
 */
#define DMA_SxCR_CHSEL(x)	dma_SxCR_chsel (JOIN2 (x, _DMA_CHAN))

/**
 * Returns the address of either DMA_LISR or DMA_HISR depending on stream number
 *
 * @param dma DMA controller (DMA_TypeDef *)
 * @param strm Stream number (0-7)
 */
INLINE_ALWAYS __IO uint32_t *dma_isr (DMA_TypeDef *dma, uint32_t strm)
{ return ((strm < 4) ? &dma->LISR : &dma->HISR); }

/**
 * Returns reference to DMA_?ISR, used when hardware feature name is known.
 *
 * @param x Hardware feature name
 */
#define DMA_ISR(x)		*dma_isr (DMA (x), DMA_STRM (x))

/**
 * Expands either to DMA_LIFCR or DMA_HIFCR depending on stream number
 * @param dma DMA controller (DMA_TypeDef *)
 * @param strm Stream number (0-7)
 */
INLINE_ALWAYS __IO uint32_t *dma_ifcr (DMA_TypeDef *dma, uint32_t strm)
{ return (strm < 4) ? &dma->LIFCR : &dma->HIFCR; }

/**
 * Returns reference to DMA_*IFCR, used when hardware feature name is known.
 *
 * @param x Hardware feature name
 */
#define DMA_IFCR(x)		*dma_ifcr (DMA (x), DMA_STRM (x))

// Helper macro to shift bit n left by 0,6,16,22 bits depending on strm
#define _DMA_IF(n,strm)		((n) << (((strm) & 2) * 8 + ((strm) & 1) * 6))

/**
 * Expands to one of the DMA_(L|H)ISR_(TC|HT|TE|DME|FE)IF(0-7) flags.
 * @param f One of TC, HT, TE, DME, FE.
 * @param s Stream number (0-7)
 */
#define DMA_ISR_IF_(f,s)	_DMA_IF (JOIN3 (DMA_LISR_,f,IF0), s)

/**
 * Same as DMA_ISR_IF_, but uses hardware feature name instead of stream number.
 * @param f One of TC, HT, TE, DME, FE.
 * @param x Hardware feature name
 */
#define DMA_ISR_IF(f,x)	DMA_ISR_IF_ (f, DMA_STRM (x))

/**
 * Expands to one of the DMA_(L|H)IFCR_C(TC|HT|TE|DME|FE)IF(0-7) flags.
 * @param f One of TC, HT, TE, DME, FE.
 * @param s Stream number (0-7)
 */
#define DMA_IFCR_IF_(f,s)	_DMA_IF (JOIN3 (DMA_LIFCR_C,f,IF0), s)

/**
 * Same as DMA_IFCR_IF_, but uses hardware feature name instead of stream number.
 * @param f One of TC, HT, TE, DME, FE.
 * @param x Hardware feature name
 */
#define DMA_IFCR_IF(f,x)	DMA_IFCR_IF_ (f, DMA_STRM (x))

#endif // DMA_TYPE_X

#if defined DMA_TYPE_1 || defined DMA_TYPE_2

/**
 * Copy data using DMA controller.
 * You must set up the peripherial you're reading from/writing to prior to
 * invoking this function.
 *
 * The following CR flags should not be specified in the @a ccr parameter,
 * as they are automatically computed from input parameters:
 *
 * @li DIR (transfer direction, memory->device, device->memory or
 *      memory->memory) will be computed based on src and dst address range.
 * @li MINC (memory address increment) will be always set
 * @li PINC (peripherial address increment) will be set for mem->mem transfers.
 *
 * @arg dma
 *      The DMA controller to use (DMA1, DMA2, ...)
 * @arg strm
 *      F0,F1,F3: DMA channel number (counting from 1)
 *      F2,F4: DMA stream number (counting from 0)
 * @arg ccr
 *      A combination of DMA_CCR (F0,F1,F3) or DMA_SxCR (F2,F4) flags
 *      (including DMA_SxCR_CHSEL() to specify channel number).
 * @arg src
 *      The source address (memory or peripherial)
 * @arg dst
 *      The destination address (memory or peripherial)
 * @arg count
 *      Number of copied elements
 */
EXTERN_C void dma_copy (DMA_TypeDef *dma, unsigned strm, uint32_t ccr,
                        volatile void *src, volatile void *dst, unsigned count);

/**
 * This function is identical to @a dma_copy except that it works specifically with the
 * DMA1 controller. You may use it for microcontrollers with just one DMA controller
 * or if you don't need the second controller.
 * @arg strm
 *      F0,F1,F3: DMA channel number (counting from 1)
 *      F2,F4: DMA stream number (counting from 0)
 * @arg ccr
 *      A combination of DMA_CCR (F0,F1,F3) or DMA_SxCR (F2,F4) flags
 *      (including DMA_SxCR_CHSEL() to specify channel number).
 * @arg src
 *      The source address (memory or peripherial)
 * @arg dst
 *      The destination address (memory or peripherial)
 * @arg count
 *      Number of copied elements
 */
EXTERN_C void dma1_copy (unsigned strm, uint32_t ccr,
                         volatile void *src, volatile void *dst, unsigned count);

/**
 * This function is identical to @a dma_copy except that it works specifically with the
 * DMA2 controller. You may use it if you're using DMA2 extensively and you don't need
 * the general form, this will save a bit of code size by not passing a pointer to the
 * DMA controller.
 * @arg strm
 *      F0,F1,F3: DMA channel number (counting from 1)
 *      F2,F4: DMA stream number (counting from 0)
 * @arg ccr
 *      A combination of DMA_CCR (F0,F1,F3) or DMA_SxCR (F2,F4) flags
 *      (including DMA_SxCR_CHSEL() to specify channel number).
 * @arg src
 *      The source address (memory or peripherial)
 * @arg dst
 *      The destination address (memory or peripherial)
 * @arg count
 *      Number of copied elements
 */
EXTERN_C void dma2_copy (unsigned strm, uint32_t ccr,
                         volatile void *src, volatile void *dst, unsigned count);

/**
 * Stop the DMA stream.
 * @arg dma
 *      The DMA controller to stop (DMA1, DMA2, ...)
 * @arg strm
 *      F0,F1,F3: DMA channel number (counting from 1)
 *      F2,F4: DMA stream number (counting from 0)
 */
EXTERN_C void dma_stop (DMA_TypeDef *dma, unsigned strm);

/**
 * Stop the DMA1 stream.
 * @arg strm
 *      F0,F1,F3: DMA channel number (counting from 1)
 *      F2,F4: DMA stream number (counting from 0)
 */
EXTERN_C void dma1_stop (unsigned strm);

/**
 * Stop the DMA2 stream.
 * @arg strm
 *      F0,F1,F3: DMA channel number (counting from 1)
 *      F2,F4: DMA stream number (counting from 0)
 */
EXTERN_C void dma2_stop (unsigned strm);

#endif // DMA_TYPE_1 || DMA_TYPE_2

#endif // _STM32_DMA_H
