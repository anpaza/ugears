/*
    Analog-to-Digital Convertor library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_ADC_H
#define _STM32_ADC_H

/**
 * @file adc.h
 *      This set of functions helps the use of ADC. There are at least
 *      three different types of ADCs used in STM32 family, all with
 *      a different API.
 */

#include <useful/useful.h>
#include <useful/fpmath.h>
#include "rcc.h"

// There are at least four different types of ADC peripherial
#if defined STM32F1
#  define ADC_TYPE_1
#elif defined STM32F0
#  define ADC_TYPE_2
#elif defined STM32F3
#  define ADC_TYPE_3
#elif defined STM32F2 || defined STM32F4
#  define ADC_TYPE_4
#else
#  error "Please define the correct ADC type for your MCU"
#endif

// Forget legacy macro from cmsis
#undef ADC

/**
 * Get a pointer to the ADC controller associated with given feature.
 * Example: ADC(LIGHT_SENS) will expand into something like ADC1
 */
#define ADC(x)			JOIN2 (ADC, ADC_NUM (x))
/// Return the ADC controller number (1, 2, ...) given hardware feature name
#define ADC_NUM(x)		JOIN2 (x, _ADC_NUM)
/// Return the ADC channel number (0-17) given hardware feature name
#define ADC_CHAN(x)		JOIN2 (x, _ADC_CHAN)

/**
 * Reset given ADC.
 * @arg adc
 *      ADC to reset
 */
static inline void adc_reset (ADC_TypeDef *adc)
{
#ifdef RCC_APB2RSTR_ADC1RST
    if (adc == ADC1) { RCC_RESET (_ADC1); } else
#endif
#ifdef RCC_APB2RSTR_ADC2RST
    if (adc == ADC2) { RCC_RESET (_ADC2); } else
#endif
#ifdef RCC_APB2RSTR_ADC3RST
    if (adc == ADC3) { RCC_RESET (_ADC3); } else
#endif
#ifdef RCC_APB2RSTR_ADC4RST
    if (adc == ADC4) { RCC_RESET (_ADC4); } else
#endif
    {}
}

/**
 * Disable clocking the ADC
 * @arg adc
 *      ADC to reset
 */
static inline void adc_clock_disable (ADC_TypeDef *adc)
{
#ifdef RCC_APB2RSTR_ADC1RST
    if (adc == ADC1) { RCC_DISABLE (_ADC1); } else
#endif
#ifdef RCC_APB2RSTR_ADC2RST
    if (adc == ADC2) { RCC_DISABLE (_ADC2); } else
#endif
#ifdef RCC_APB2RSTR_ADC3RST
    if (adc == ADC3) { RCC_DISABLE (_ADC3); } else
#endif
#ifdef RCC_APB2RSTR_ADC4RST
    if (adc == ADC4) { RCC_DISABLE (_ADC4); } else
#endif
    {}
}

/**
 * Enable clocking the ADC
 * @arg adc
 *      ADC to initialize
 */
static inline void adc_clock_enable (ADC_TypeDef *adc)
{
#ifdef RCC_APB2RSTR_ADC1RST
    if (adc == ADC1) { RCC_ENABLE (_ADC1); } else
#endif
#ifdef RCC_APB2RSTR_ADC2RST
    if (adc == ADC2) { RCC_ENABLE (_ADC2); } else
#endif
#ifdef RCC_APB2RSTR_ADC3RST
    if (adc == ADC3) { RCC_ENABLE (_ADC3); } else
#endif
#ifdef RCC_APB2RSTR_ADC4RST
    if (adc == ADC4) { RCC_ENABLE (_ADC4); } else
#endif
    {}
}

/**
 * Get a pointer to ADCx_COMMON controlling given ADC
 */
static inline ADC_Common_TypeDef *adc_common (ADC_TypeDef *adc)
{
    (void)adc;
#if defined ADC12_COMMON && defined ADC34_COMMON
    if ((adc == ADC1) || (adc == ADC2))
        return ADC12_COMMON;
    else
        return ADC34_COMMON;
#elif defined ADC12_COMMON && defined ADC3_COMMON
    if (adc == ADC3)
        return ADC3_COMMON;
    else
        return ADC12_COMMON;
#elif defined ADC1_COMMON
    return ADC1_COMMON;
#elif defined ADC12_COMMON
    return ADC12_COMMON;
#elif defined ADC123_COMMON
    return ADC123_COMMON;
#endif
}

#if defined ADC_TYPE_1 || defined ADC_TYPE_4

/**
 * Enable or disable ADC
 * @arg adc
 *      The ADC to change state
 * @arg enable
 *      Enable ADC if non-zero, disable otherwise
 */
static inline void adc_enable (ADC_TypeDef *adc, bool enable)
{
    if (enable)
        adc->CR2 |= ADC_CR2_ADON;
    else
        adc->CR2 &= ~ADC_CR2_ADON;
}

#endif

#if defined ADC_TYPE_1

/*
 * This type of ADC uses the concept of "regular" and "injected" sequence
 * of ADC channels. The regular sequence is a group of channels which are
 * converted one by one when a specific trigger occurs. The injected group
 * is similar, but it will interrupt the regular sequence if it is running,
 * convert all channels set up in the injected sequence, then resume
 * regular sequence conversion. In some sense, it is similar to how a IRQ
 * interrupts the regular program execution, then resumes when the IRQ work
 * is done. Injected sequence stores conversion results in separate special
 * registers.
 */

#define ADC_CR2_EXTSEL_T1_CC1	0			/*!< Timer 1 CC1 event */
#define ADC_CR2_EXTSEL_T1_CC2	ADC_CR2_EXTSEL_0	/*!< Timer 1 CC2 event */
#define ADC_CR2_EXTSEL_T1_CC3	ADC_CR2_EXTSEL_1	/*!< Timer 1 CC3 event */
#define ADC_CR2_EXTSEL_T2_CC2	(ADC_CR2_EXTSEL_0|ADC_CR2_EXTSEL_1)	/*!< Timer 2 CC2 event */
#define ADC_CR2_EXTSEL_T3_TRGO	ADC_CR2_EXTSEL_2	/*!< Timer 3 TRGO event */
#define ADC_CR2_EXTSEL_T4_CC4	(ADC_CR2_EXTSEL_2|ADC_CR2_EXTSEL_0)	/*!< Timer 4 CC4 event */
#define ADC_CR2_EXTSEL_EXTI11	(ADC_CR2_EXTSEL_2|ADC_CR2_EXTSEL_1)	/*!< EXTI line 11 */
#define ADC_CR2_EXTSEL_SWSTART	(ADC_CR2_EXTSEL_2|ADC_CR2_EXTSEL_1|ADC_CR2_EXTSEL_0)	/*!< SWSTART */

#define ADC_CR2_JEXTSEL_T1_TRGO	0			/*!< Timer 1 TRGO event */
#define ADC_CR2_JEXTSEL_T1_CC4	ADC_CR2_EXTSEL_0	/*!< Timer 1 CC4 event */
#define ADC_CR2_JEXTSEL_T2_TRGO	ADC_CR2_EXTSEL_1	/*!< Timer 2 TRGO event */
#define ADC_CR2_JEXTSEL_T2_CC1	(ADC_CR2_EXTSEL_0|ADC_CR2_EXTSEL_1)	/*!< Timer 2 CC1 event */
#define ADC_CR2_JEXTSEL_T3_CC4	ADC_CR2_EXTSEL_2	/*!< Timer 3 CC4 event */
#define ADC_CR2_JEXTSEL_T4_TRGO	(ADC_CR2_EXTSEL_2|ADC_CR2_EXTSEL_0)	/*!< Timer 4 TRGO event */
#define ADC_CR2_JEXTSEL_EXTI15	(ADC_CR2_EXTSEL_2|ADC_CR2_EXTSEL_1)	/*!< EXTI line 15 */
#define ADC_CR2_JEXTSEL_JSWSTART (ADC_CR2_EXTSEL_2|ADC_CR2_EXTSEL_1|ADC_CR2_EXTSEL_0)	/*!< JSWSTART */

/**
 * Enable or disable Temperature Sensor & VREF on ADC1 channel 16 or 18
 * (depending on device).
 * @arg adc
 *      The ADC to set up
 * @arg enable
 *      Enable TS & VREF if non-zero, disable otherwise
 */
static inline void adc_tsvref (ADC_TypeDef *adc, bool enable)
{
    if (enable)
        adc->CR2 |= ADC_CR2_TSVREFE;
    else
        adc->CR2 &= ~ADC_CR2_TSVREFE;
}

/// Regular sequence conversion start trigger
typedef enum
{
    /// Disable external trigger
    aextNone = 0,
    /// Timer 1 CC1 event
    aextT1CC1 = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_T1_CC1,
    /// Timer 1 CC2 event
    aextT1CC2 = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_T1_CC2,
    /// Timer 1 CC3 event
    aextT1CC3 = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_T1_CC3,
    /// Timer 2 CC2 event
    aextT2CC2 = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_T2_CC2,
    /// Timer 3 TRGO event
    aextT3TRGO = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_T3_TRGO,
    /// Timer 4 CC4 event
    aextT4CC4 = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_T4_CC4,
    /// EXTI line 11
    aextEXTI11 = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_EXTI11,
    /// SWSTART
    aextSWSTART = ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_SWSTART,
} adc_extsel_t;

/**
 * Select external trigger for starting regular sequence conversion
 * @arg adc
 *      ADC to set external trigger
 * @arg mode
 *      one of the aextXXX triggers
 */
static inline void adc_extsel (ADC_TypeDef *adc, adc_extsel_t mode)
{
    if (mode)
        adc->CR2 = (adc->CR2 & ~ADC_CR2_EXTSEL) | mode;
    else
        adc->CR2 &= ~(ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL);
}

/// Injected sequence conversion start trigger
typedef enum
{
    /// Disable external trigger
    ajextNone = 0,
    /// Timer 1 TRGO event
    ajextT1TRGO = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_T1_TRGO,
    /// Timer 1 CC4 event
    ajextT1CC4 = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_T1_CC4,
    /// Timer 2 TRGO event
    ajextT2TRGO = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_T2_TRGO,
    /// Timer 2 CC1 event
    ajextT2CC1 = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_T2_CC1,
    /// Timer 3 CC4 event
    ajextT3CC4 = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_T3_CC4,
    /// Timer 4 TRGO event
    ajextT4TRGO = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_T4_TRGO,
    /// EXTI line 15
    ajextEXTI15 = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_EXTI15,
    /// JSWSTART
    ajextSWSTART = ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL_JSWSTART,
} adc_jextsel_t;

/**
 * Select external trigger for starting injected sequence conversion
 * @arg adc
 *      ADC to set external trigger
 * @arg mode
 *      one of the ajextXXX triggers
 */
static inline void adc_jextsel (ADC_TypeDef *adc, adc_jextsel_t mode)
{
    if (mode)
        adc->CR2 = (adc->CR2 & ~ADC_CR2_JEXTSEL) | mode;
    else
        adc->CR2 &= ~(ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL);
}

/**
 * Calibrate given ADC.
 * This will power off ADC, wait 2 cycles, power it on and then
 * run calibration. ADC state will be lost!
 * @arg adc
 *      ADC to calibrate
 */
extern void adc_calibrate (ADC_TypeDef *adc);

/**
 * Constants to set ADC sampling time, in ADC cycles.
 */
typedef enum
{
    /// 000: 1.5 cycles
    asc1_5 = 0,
    /// 001: 7.5 cycles
    asc7_5 = 1,
    /// 010: 13.5 cycles
    asc13_5 = 2,
    /// 011: 28.5 cycles
    asc28_5 = 3,
    /// 100: 41.5 cycles
    asc41_5 = 4,
    /// 101: 55.5 cycles
    asc55_5 = 5,
    /// 110: 71.5 cycles
    asc71_5 = 6,
    /// 111: 239.5 cycles
    asc239_5 = 7,
} adc_sampling_t;

/**
 * Set ADC channel sampling time, in ADC clocks, see adc_sampling_t constants.
 * @arg adc
 *      The ADC to set up
 * @arg chan
 *      ADC channel number
 * @arg bits
 *      Channel sampling time
 */
static inline void adc_sampling_clocks (ADC_TypeDef *adc, unsigned chan, adc_sampling_t bits)
{
    if (chan < 10)
        adc->SMPR2 = (adc->SMPR2 & ~(7 << (chan * 3))) | (((uint32_t)bits) << (chan * 3));
    else
        adc->SMPR1 = (adc->SMPR2 & ~(7 << ((chan - 10) * 3))) | (((uint32_t)bits) << ((chan - 10) * 3));
}

/**
 * Convert ADC clocks into ADC sampling time bits
 * @arg clocks
 *      ADC clocks
 * @return
 *      Largest sampling time bits which define a sampling time
 *      not shorter than given amount of ADC clocks.
 */
static inline adc_sampling_t adc_clk2bits (uint32_t clocks)
{
    if (clocks < 2)
        return asc1_5;
    else if (clocks < 8)
        return asc7_5;
    else if (clocks < 14)
        return asc13_5;
    else if (clocks < 29)
        return asc28_5;
    else if (clocks < 42)
        return asc41_5;
    else if (clocks < 56)
        return asc55_5;
    else if (clocks < 72)
        return asc71_5;
    else
        return asc239_5;
}

#ifdef CLOCK_ADC_DIV

/**
 * Convert time to ADC sampling time bits and set ADC channel sampling time.
 * @arg adc
 *      The ADC to set up
 * @arg chan
 *      ADC channel number
 * @arg time
 *      Time in 0.32 fixed-point format (2^32 = 1 second), use
 *      FP32() macro to convert seconds to 0.32 fixed-point.
 */
static inline void adc_sampling_time (ADC_TypeDef *adc,
    unsigned chan, uint32_t time)
{
    /* Convert microseconds to ADC clocks */
    uint32_t adc_clock = PCLK2_FREQ / CLOCK_ADC_DIV;
    uint32_t clocks = umul_h32 (adc_clock, time);

    adc_sampling_clocks (adc, chan, adc_clk2bits (clocks));
}

/**
 * Set up sampling timings for all channels at once.
 * This function is meant to be heavily optimized by the compiler.
 * @arg adc
 *      The ADC to set up
 * @arg time0, time1, ... time17
 *      ADC channel sampling time in seconds in 0.32 fixed-point format.
 */
static inline void adc_sampling_time_all (ADC_TypeDef *adc,
    uint32_t time0, uint32_t time1, uint32_t time2, uint32_t time3,
    uint32_t time4, uint32_t time5, uint32_t time6, uint32_t time7,
    uint32_t time8, uint32_t time9, uint32_t time10, uint32_t time11,
    uint32_t time12, uint32_t time13, uint32_t time14, uint32_t time15,
    uint32_t time16, uint32_t time17)
{
    uint32_t adc_clock = PCLK2_FREQ / CLOCK_ADC_DIV;
    adc->SMPR2 =
        (adc_clk2bits (umul_h32 (adc_clock, time0)) << 0) |
        (adc_clk2bits (umul_h32 (adc_clock, time1)) << 3) |
        (adc_clk2bits (umul_h32 (adc_clock, time2)) << 6) |
        (adc_clk2bits (umul_h32 (adc_clock, time3)) << 9) |
        (adc_clk2bits (umul_h32 (adc_clock, time4)) << 12) |
        (adc_clk2bits (umul_h32 (adc_clock, time5)) << 15) |
        (adc_clk2bits (umul_h32 (adc_clock, time6)) << 18) |
        (adc_clk2bits (umul_h32 (adc_clock, time7)) << 21) |
        (adc_clk2bits (umul_h32 (adc_clock, time8)) << 24) |
        (adc_clk2bits (umul_h32 (adc_clock, time9)) << 27);
    adc->SMPR1 = 
        (adc_clk2bits (umul_h32 (adc_clock, time10)) << 0) |
        (adc_clk2bits (umul_h32 (adc_clock, time11)) << 3) |
        (adc_clk2bits (umul_h32 (adc_clock, time12)) << 6) |
        (adc_clk2bits (umul_h32 (adc_clock, time13)) << 9) |
        (adc_clk2bits (umul_h32 (adc_clock, time14)) << 12) |
        (adc_clk2bits (umul_h32 (adc_clock, time15)) << 15) |
        (adc_clk2bits (umul_h32 (adc_clock, time16)) << 18) |
        (adc_clk2bits (umul_h32 (adc_clock, time17)) << 21);
}

#endif

/**
 * Regular channel Start flag.
 * This bit is set by hardware when regular channel conversion starts.
 * It is cleared by software.
 */
#define ADC_STATUS_START	ADC_SR_STRT
/**
 * Injected channel Start flag.
 * This bit is set by hardware when injected channel group conversion starts.
 * It is cleared by software.
 */
#define ADC_STATUS_JSTART	ADC_SR_JSTRT
/**
 * Injected channel end of conversion.
 * This bit is set by hardware at the end of all injected group channel
 * conversion. It is cleared by software.
 */
#define ADC_STATUS_JEOC		ADC_SR_JEOC
/**
 * End of conversion.
 * This bit is set by hardware at the end of a group channel conversion
 * (regular or injected). It is cleared by software or by reading the ADC_DR.
 */
#define ADC_STATUS_EOC		ADC_SR_EOC
/**
 * Analog watchdog flag.
 * This bit is set by hardware when the converted voltage crosses
 * the values programmed with the @a adc_watchdog() function.
 * It is cleared by software.
 */
#define ADC_STATUS_AWD		ADC_SR_AWD

/**
 * Get ADC status register.
 * @arg adc
 *      The ADC to query
 * @return
 *      A combination of bit flags, see ADC_STATUS_XXX
 */
static inline uint32_t adc_status (ADC_TypeDef *adc)
{ return adc->SR; }

/**
 * Clear bits in the ADC status register.
 * @arg adc
 *      The ADC to query
 * @arg flags
 *      The flags to clear, a combination of the ADC_STATUS_XXX bit flags
 */
static inline void adc_status_clear (ADC_TypeDef *adc, uint32_t flags)
{ adc->SR &= ~flags; }

/// Generate IRQ from the analog watchdog
#define ADC_IRQ_AWD		ADC_CR1_AWDIE
/// Generate IRQ at the end of regular conversion
#define ADC_IRQ_EOC		ADC_CR1_EOCIE
/// Generate IRQ at the end of injected sequence conversion
#define ADC_IRQ_JEOC		ADC_CR1_JEOCIE
/// All IRQ bits
#define ADC_IRQ_ALL		(ADC_CR1_AWDIE | ADC_CR1_EOCIE | ADC_CR1_JEOCIE)

/**
 * Enable or disable generating interrupts on specific events.
 * @arg adc
 *      The ADC to set interrupt generation for
 * @arg irqs
 *      The events on which to generate IRQs (a combination of
 *      ADC_IRQ_XXX bit flags)
 */
static inline void adc_irqs (ADC_TypeDef *adc, uint32_t irqs)
{
    adc->CR1 = (adc->CR1 & ~ADC_IRQ_ALL) | (irqs & ADC_IRQ_ALL);
}

/**
 * Set regular sequence channel number
 * @arg adc
 *      The ADC to set sequence channel number
 * @arg idx
 *      Channel index (0-15)
 * @arg chan
 *      ADC channel number (0-17)
 */
extern void adc_regseq_chan (ADC_TypeDef *adc, uint32_t idx, uint32_t chan);

/**
 * Set regular sequence channel count
 * @arg adc
 *      The ADC to set regular sequence channel count
 * @arg len
 *      Number of channels to convert (ADC will convert channels
 *      with indices 0 .. len-1).
 */
static inline void adc_regseq_len (ADC_TypeDef *adc, uint32_t len)
{ adc->SQR1 = (adc->SQR1 & ~ADC_SQR1_L) | ((len - 1) << 20); }

/**
 * Optimized function to set regular sequence to 1 to 6 channels at once.
 * @arg adc
 *      The ADC to set regular sequence
 * @arg len
 *      Number of channels to convert (ADC will convert channels
 *      with indices 0 .. len-1).
 * @arg chans
 *      ADC channel numbers (0-17) stuffed into their respective bit position
 *      with the ADC_REGSEQ_X() macro
 */
static inline void adc_regseq (ADC_TypeDef *adc, uint32_t len, uint32_t chans)
{
    adc->SQR3 = chans;
    adc_regseq_len (adc, len);
}

/**
 * Optimized function to set regular sequence to 7 to 12 channels at once.
 * @arg adc
 *      The ADC to set regular sequence
 * @arg len
 *      Number of channels to convert (ADC will convert channels
 *      with indices 0 .. len-1).
 * @arg chans1
 *      ADC channel numbers (0-17) stuffed into their respective bit position
 *      with the ADC_REGSEQ_X() macro
 * @arg chans2
 *      ADC channel numbers (0-17) stuffed into their respective bit position
 *      with the ADC_REGSEQ_X() macro
 */
static inline void adc_regseq2 (ADC_TypeDef *adc, uint32_t len,
    uint32_t chans1, uint32_t chans2)
{
    adc->SQR3 = chans1;
    adc->SQR2 = chans2;
    adc_regseq_len (adc, len);
}

// Helper macros, usually you don't need to use them directly
#define __ADC_REGSEQ_1(c1) \
    (((c1 & 31) << 0))
#define __ADC_REGSEQ_2(c1,c2) \
    (((c1 & 31) << 0) | ((c2 & 31) << 5))
#define __ADC_REGSEQ_3(c1,c2,c3) \
    (((c1 & 31) << 0) | ((c2 & 31) << 5) | ((c3 & 31) << 10))
#define __ADC_REGSEQ_4(c1,c2,c3,c4) \
    (((c1 & 31) << 0) | ((c2 & 31) << 5) | ((c3 & 31) << 10) | \
     ((c4 & 31) << 15))
#define __ADC_REGSEQ_5(c1,c2,c3,c4,c5) \
    (((c1 & 31) << 0) | ((c2 & 31) << 5) | ((c3 & 31) << 10) | \
     ((c4 & 31) << 15) | ((c5 & 31) << 20))
#define __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6) \
    (((c1 & 31) << 0) | ((c2 & 31) << 5) | ((c3 & 31) << 10) | \
     ((c4 & 31) << 15) | ((c5 & 31) << 20) | ((c6 & 31) << 25))

/**
 * The _ADC_REGSEQ_X series of macros provides a easy-to-use way
 * to specify the len,chans arguments to the @a adc_regseq (for X <= 6)
 * and @a adc_regseq2 (for X > 6) functions.
 *
 * Usage example:
 * @verbatim
 * adc_regseq (ADC1, _ADC_REGSEQ_3 (3, 8, 4));
 * adc_regseq2 (ADC1, _ADC_REGSEQ_7 (3, 8, 4, 1, 9, 16, 17));
 * @endverbatim
 */
#define _ADC_REGSEQ_1(c1) \
    1, __ADC_REGSEQ_1(c1)
#define _ADC_REGSEQ_2(c1,c2) \
    2, __ADC_REGSEQ_2(c1,c2)
#define _ADC_REGSEQ_3(c1,c2,c3) \
    3, __ADC_REGSEQ_3(c1,c2,c3)
#define _ADC_REGSEQ_4(c1,c2,c3,c4) \
    4, __ADC_REGSEQ_4(c1,c2,c3,c4)
#define _ADC_REGSEQ_5(c1,c2,c3,c4,c5) \
    5, __ADC_REGSEQ_5(c1,c2,c3,c4,c5)
#define _ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6) \
    6, __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6)
#define _ADC_REGSEQ_7(c1,c2,c3,c4,c5,c6,c7) \
    7, __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6), \
       __ADC_REGSEQ_1(c7)
#define _ADC_REGSEQ_8(c1,c2,c3,c4,c5,c6,c7,c8) \
    8, __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6), \
       __ADC_REGSEQ_2(c7,c8)
#define _ADC_REGSEQ_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) \
    9, __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6), \
       __ADC_REGSEQ_3(c7,c8,c9)
#define _ADC_REGSEQ_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) \
   10, __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6), \
       __ADC_REGSEQ_4(c7,c8,c9,c10)
#define _ADC_REGSEQ_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) \
   11, __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6), \
       __ADC_REGSEQ_5(c7,c8,c9,c10,c11)
#define _ADC_REGSEQ_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) \
   12, __ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6), \
       __ADC_REGSEQ_6(c7,c8,c9,c10,c11,c12)

/**
 * The ADC_REGSEQ_X series of macros does the same as _ADC_REGSEQ_X,
 * but uses symbolic ADC channel names rather than numbers.
 *
 * Usage example:
 * @verbatim
 * adc_regseq (ADC1, ADC_REGSEQ_3 (TEMP1, TEMP2, TEMP3));
 * adc_regseq2 (ADC1, ADC_REGSEQ_7 (TEMP1, TEMP2, TEMP3, THR_IN, PWR_DET, HALL, VREF));
 * @endverbatim
 */
#define ADC_REGSEQ_1(c1) _ADC_REGSEQ_1(\
    ADC_CHAN(c1))
#define ADC_REGSEQ_2(c1,c2) _ADC_REGSEQ_2(\
    ADC_CHAN(c1), ADC_CHAN(c2))
#define ADC_REGSEQ_3(c1,c2,c3) _ADC_REGSEQ_3(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3))
#define ADC_REGSEQ_4(c1,c2,c3,c4) _ADC_REGSEQ_4(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4))
#define ADC_REGSEQ_5(c1,c2,c3,c4,c5) _ADC_REGSEQ_5(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5))
#define ADC_REGSEQ_6(c1,c2,c3,c4,c5,c6) _ADC_REGSEQ_6(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5),ADC_CHAN(c6))
#define ADC_REGSEQ_7(c1,c2,c3,c4,c5,c6,c7) _ADC_REGSEQ_7(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5),ADC_CHAN(c6),\
    ADC_CHAN(c7))
#define ADC_REGSEQ_8(c1,c2,c3,c4,c5,c6,c7,c8) _ADC_REGSEQ_8(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5),ADC_CHAN(c6),\
    ADC_CHAN(c7),ADC_CHAN(c8))
#define ADC_REGSEQ_9(c1,c2,c3,c4,c5,c6,c7,c8,c9) _ADC_REGSEQ_9(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5),ADC_CHAN(c6),\
    ADC_CHAN(c7),ADC_CHAN(c8),ADC_CHAN(c9))
#define ADC_REGSEQ_10(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10) _ADC_REGSEQ_10(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5),ADC_CHAN(c6),\
    ADC_CHAN(c7),ADC_CHAN(c8),ADC_CHAN(c9),ADC_CHAN(c10))
#define ADC_REGSEQ_11(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11) _ADC_REGSEQ_11(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5),ADC_CHAN(c6),\
    ADC_CHAN(c7),ADC_CHAN(c8),ADC_CHAN(c9),ADC_CHAN(c10),ADC_CHAN(c11))
#define ADC_REGSEQ_12(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12) _ADC_REGSEQ_12(\
    ADC_CHAN(c1),ADC_CHAN(c2),ADC_CHAN(c3),ADC_CHAN(c4),ADC_CHAN(c5),ADC_CHAN(c6),\
    ADC_CHAN(c7),ADC_CHAN(c8),ADC_CHAN(c9),ADC_CHAN(c10),ADC_CHAN(c11),ADC_CHAN(c12))

/**
 * Set injected sequence to 1 to 4 channels at once.
 * @arg adc
 *      The ADC to set regular sequence
 * @arg len
 *      Number of channels to convert
 * @arg chans
 *      ADC channel numbers (0-17) stuffed into their respective bit position
 *      with the ADC_JREGSEQ_X() macro
 */
static inline void adc_jregseq (ADC_TypeDef *adc, uint32_t len, uint32_t chans)
{
    adc->JSQR = (chans &
        (ADC_JSQR_JSQ1 | ADC_JSQR_JSQ2 | ADC_JSQR_JSQ3 | ADC_JSQR_JSQ4)) |
        (((len - 1) & 3) << 20);
}

/**
 * The ADC_JREGSEQ_X series of macros provides a easy-to-use way
 * to specify the len,chans arguments to the @a adc_jregseq function.
 *
 * Usage example:
 * @verbatim
 * adc_jregseq (ADC1, ADC_JREGSEQ_2 (5, 3));
 * @endverbatim
 */
#define ADC_JREGSEQ_1(c1) \
    1, (((c1 & 31) << 15))
#define ADC_JREGSEQ_2(c1) \
    2, (((c1 & 31) << 10) | ((c2 & 31) << 15))
#define ADC_JREGSEQ_3(c1) \
    3, (((c1 & 31) <<  5) | ((c2 & 31) << 10) | ((c3 & 31) << 15))
#define ADC_JREGSEQ_4(c1) \
    4, (((c1 & 31) <<  0) | ((c2 & 31) <<  5) | ((c3 & 31) << 10) | ((c4 & 31) << 15))

/**
 * Set up ADC watchdog
 * @arg adc
 *      The ADC to set watchdog
 * @arg min
 *      The lower ADC threshold boundary (in lower 12 bits, independent on alignment)
 * @arg max
 *      The upper ADC threshold boundary (in lower 12 bits, independent on alignment)
 */
static inline void adc_watchdog_bounds (ADC_TypeDef *adc, uint32_t min, uint32_t max)
{
    adc->LTR = min;
    adc->HTR = max;
}

/// Watchdog modes
typedef enum
{
    /// Watcgdog disabled
    awdmNone = 0,
    /// Watchdog enabled on all regular sequence channels
    awdmAllReg = ADC_CR1_AWDEN,
    /// Watchdog enabled on all injected sequence channels
    awdmAllInj = ADC_CR1_JAWDEN,
    /// Watchdog enabled on all regular and injected sequence channels
    awdmAllRegInj = ADC_CR1_AWDEN | ADC_CR1_JAWDEN,
    /// Watchdog enabled on a single regular channel
    awdmSingleReg = ADC_CR1_AWDSGL | ADC_CR1_AWDEN,
    /// Watchdog enabled on a single injected channel
    awdmSingleInj = ADC_CR1_AWDSGL | ADC_CR1_JAWDEN,
    /// Watchdog enabled on a single regular or injected channel
    awdmSingleRegInj = ADC_CR1_AWDSGL | ADC_CR1_AWDEN | ADC_CR1_JAWDEN,
    /// All ADC mode bits
    awdmAll = ADC_CR1_AWDEN | ADC_CR1_JAWDEN | ADC_CR1_AWDSGL
} adc_watchdog_mode_t;

/**
 * Enable the watchdog 
 * @arg adc
 *      The ADC to set watchdog
 * @arg mode
 *      Watchdog mode (one of the awdmXXX values)
 * @arg chan
 *      ADC channel number to watch on (valid only if mode & ADC_WATCHDOG_SINGLE)
 */
static inline void adc_watchdog_enable (ADC_TypeDef *adc,
    adc_watchdog_mode_t mode, uint32_t chan)
{
    adc->CR1 = (adc->CR1 & ~(ADC_CR1_AWDCH | awdmAll)) |
        (chan & ADC_CR1_AWDCH) | mode;
}

/**
 * ADC discontinuous modes
 */
typedef enum
{
    /// Disable discontinuous mode
    admDisable = 0,
    /// Enable discontinuous mode for regular sequence
    admReg = ADC_CR1_DISCEN,
    /// Enable discontinuous mode for injected sequence
    admInj = ADC_CR1_JDISCEN,
    /// Enable discontinuous mode for regular and injected sequences
    admRegInj = ADC_CR1_DISCEN | ADC_CR1_JDISCEN,
    /// All bits set
    admAll = ADC_CR1_DISCEN | ADC_CR1_JDISCEN
} adc_discmode_t;

/**
 * Set ADC discontinuous mode. In this mode on each external trigger
 * only a subgroup of the regular sequence will be converted, on next
 * trigger next @a count channels will be converted and so on, until
 * the subgroups will start again from the first channel in regular sequence.
 * @arg adc
 *      The ADC to set discontinuous mode
 * @arg count
 *      Number of channels in the discontinuous group (0-8). If set to 0,
 *      discontinuous mode is disabled.
 */
static inline void adc_discmode (ADC_TypeDef *adc,
    adc_discmode_t mode, uint32_t count)
{
    adc->CR1 = (adc->CR1 & ~(ADC_CR1_DISCNUM | admAll)) |
        (count ? (((count - 1) & 7) << 13) | mode : 0);
}

/**
 * Enable or disable Scan mode. If this mode is enabled, the whole
 * group of channels set with adc_regseq() and adc_jregseq() functions
 * will be converted after conversion is started.
 * @arg adc
 *      The ADC to set scan mode
 * @arg enable
 *      If non-zero, enable scan mode, otherwise disable
 */
static inline void adc_scanmode (ADC_TypeDef *adc, bool enable)
{
    if (enable)
        adc->CR1 |= ADC_CR1_SCAN;
    else
        adc->CR1 &= ~ADC_CR1_SCAN;
}

/**
 * Enable or disable automatic injected group mode. In this mode
 * the injected sequence will start conversion after the regular
 * sequence ends.
 * @arg adc
 *      The ADC to set automatic injected group mode
 * @arg enable
 *      If non-zero, enable auto mode, otherwise disable
 */
static inline void adc_jauto (ADC_TypeDef *adc, bool enable)
{
    if (enable)
        adc->CR1 |= ADC_CR1_JAUTO;
    else
        adc->CR1 &= ~ADC_CR1_JAUTO;
}

/**
 * Enable DMA for collecting ADC sampled data.
 * @arg adc
 *      The ADC to enable or disable DMA usage
 * @arg enable
 *      If non-zero, enable DMA usage, otherwise disable
 */
static inline void adc_dma (ADC_TypeDef *adc, bool enable)
{
    if (enable)
        adc->CR2 |= ADC_CR2_DMA;
    else
        adc->CR2 &= ~ADC_CR2_DMA;
}

/**
 * Start ADC conversions from channels in the regular sequence group.
 * @arg adc
 *      The ADC to start regular sequence conversions
 */
static inline void adc_start (ADC_TypeDef *adc)
{ adc->CR2 |= ADC_CR2_SWSTART; }

/**
 * Start ADC conversions from channels in the injected sequence group.
 * @arg adc
 *      The ADC to start injected sequence conversions
 */
static inline void adc_jstart (ADC_TypeDef *adc)
{ adc->CR2 |= ADC_CR2_JSWSTART; }

/**
 * Enable or disable left alignment of the ADC results.
 * @arg adc
 *      The ADC to change alignment for
 * @arg enable
 *      If non-zero, left-align ADC conversion result (12 bits) in the
 *      output registers (16 bits), otherwise right-align
 */
static inline void adc_left_align (ADC_TypeDef *adc, bool enable)
{
    if (enable)
        adc->CR2 |= ADC_CR2_ALIGN;
    else
        adc->CR2 &= ~ADC_CR2_ALIGN;
}

/**
 * Enable or disable continuous conversion mode.
 * @arg adc
 *      The ADC to set continous conversion mode state
 * @arg enable
 *      If non-zero, enable continous mode, disable otherwise
 */
static inline void adc_continuous (ADC_TypeDef *adc, bool enable)
{
    if (enable)
        adc->CR2 |= ADC_CR2_CONT;
    else
        adc->CR2 &= ~ADC_CR2_CONT;
}

#elif defined ADC_TYPE_4

/**
 * Enable or disable Temperature Sensor & VREF on ADC1 channel 16 or 18
 * (depending on device).
 * @arg adc
 *      The ADC to set up
 * @arg enable
 *      Enable TS & VREF if non-zero, disable otherwise
 */
static inline void adc_tsvref (ADC_TypeDef *adc, bool enable)
{
    ADC_Common_TypeDef *adcc = adc_common (adc);
    if (enable)
        adcc->CCR |= ADC_CCR_TSVREFE;
    else
        adcc->CCR &= ~ADC_CCR_TSVREFE;
}

/**
 * Enable or disable measuring VBAT on ADC1.
 *  * VBAT/2 to the ADC1_IN18 input channel, on STM32F40xx and STM32F41xx devices
 *  * VBAT/4 to the ADC1_IN18 input channel, on STM32F42xx and STM32F43xx devices
 * @arg adc
 *      The ADC to set up
 * @arg enable
 *      Enable VBAT measuring if non-zero, disable otherwise
 */
static inline void adc_vbat (ADC_TypeDef *adc, bool enable)
{
    ADC_Common_TypeDef *adcc = adc_common (adc);
    if (enable)
        adcc->CCR |= ADC_CCR_VBATE;
    else
        adcc->CCR &= ~ADC_CCR_VBATE;
}

#endif // ADC_TYPE

#endif // _STM32_ADC_H
