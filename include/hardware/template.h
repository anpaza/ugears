/*
    Short description of your board here

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

/**
 * @file hardware.h
 *      This is just a template for your board-specific header file.
 *      It is supposed to include the MCU-specific header files, and also
 *      declare the on-board hardware connections.
 */

/**
 * A hardware definition is supposed to declare all the ports and other
 * resources used (timers, DMAs and so on). There are a lot of useful
 * macros in "macros.h" which can be later used to conveniently set up
 * and access every hardware feature.
 *
 * A "hardware feature" is supposed to be the minimal atomic hardware
 * unit - a LED, a button, a sensor and so on, usually connected to a
 * single MCU pin. Most macros use the following convention
 * ("HWFN" being the "hardware feature name"):
 *
 * @li HWFN_PORT defines the port name to which the feature is
 *      connected (A, B, C etc). Used by GPIO_PORT(HWFN) macro.
 * @li HWFN_BIT defines the bit number to which the feature is
 *      connected (0, 1, 2 ...). Used by GPIO_BIT(HWFN) macro.
 * @li HWFN_GPIO_CONFIG defines the parameters passed to the
 *      GPIO_CONFIGURE macro which returns a bitmask for GPIO
 *      setup (ex: OUTPUT_2MHz,OPENDRAIN,1).
 * @li HWFN_IRQ_PRIO defines the IRQ priority corresponding to
 *      this hardware feature. Used by IRQ_PRIO(HWFN) macro.
 * @li HWFN_DMA_NUM defines the DMA controller number used for
 *      the hardware feature Used by DMA_NUM(HWFN) macro.
 * @li HWFN_DMA_CHAN defines the DMA channel number used for
 *      the hardware feature. Used by DMA_CHAN(HWFN) macro.
 * @li HWFN_DMA_IRQ_PRIO defines the IRQ priority corresponding to
 *      the DMA channel associated with this hardware feature.
 *      Used by DMA_IRQ_PRIO(HWFN) macro.
 * @li HWFN_USART defines the index of the USART corresponding
 *      to this hardware feature.
 *
 * You can later use these declarations like this, for example:
 * @code
 *      // Set the pin to high level
 *      GPIO (HWFN)->BSRR = GPIO_BITV (HWFN);
 *      // Same but simpler
 *      GPIO_SET (HWFN);
 *      // Get port state
 *      if (GPIO (HWFN)->IDR & GPIO_BITV (HWFN)) ...
 *      // Set up the GPIO mode for the pin
 *      GPIO_CONFIG (HWFN, OUTPUT_2MHz, OPENDRAIN, 1);
 *      // Same but using the HWFN_GPIO_CONFIG macro
 *      GPIO_CONF (HWFN);
 *
 *      // DMA stuff ... clear global interrupt flag for the channel
 *      DMA->IFCR = DMA_IFCR (UART1_TX, CGIF);
 *      // Set up the DMA config register for channel
 *      DMAC (UART1_TX)->CCR = DMA_CCR (UART1_TX, MINC)
 *
 *      // Set up the DMA transmit complete IRQ ...
 *      nvic_setup (DMA_IRQ (USART1_TX), IRQ_PRIO (USART1_TX_DMA));
 * @endcode
 */

// Uncomment the line corresponding to your MCU
//#include "stm32f0xx.h"
//#include "stm32f1xx.h"
//#include "stm32f2xx.h"
//#include "stm32f3xx.h"
//#include "stm32f4xx.h"
//#include "stm32h7xx.h"

/**
 * Also this file defines the clock configuration for your board.
 * This is heavily dependant on MCU series, thus the details can
 * be found in the respective ugears/stm32/clock_* file, which is
 * included from ugears.h file (via intermediary clocks.h).
 *
 * The basic idea is that you define some macros before including the
 * respective file, and the header file will find out the rest for you.
 */

// System clock configuration - define before including clocks-*.h
//#define SYSCLK_SOURCE	HSE
//#define HSE_VALUE	8000000

// RTC crystal frequency, Hz
//#define LSE_VALUE	32768

#endif // __HARDWARE_H__
