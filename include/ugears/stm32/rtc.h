/*
    STM32 Real-Time-Clock library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _RTC_H
#define _RTC_H

#include "useful.h"

/**
 * @file rtc.h
 *      A small simple library that makes life easy when working
 *      with STM32's Real-Time-Clock. Also this file provides
 *      a set of functions to work with the BKP peripherial
 *      (on STM32F1xx) which is integrated with RTC on other
 *      microcontrollers.
 */

/**
 * How many RTC ticks per second we want (1 = 1 tick per second).
 * Define in your HARDWARE_H if default is not suitable.
 */
#ifndef RTC_TICKS_PER_SEC
#define RTC_TICKS_PER_SEC	16U
#endif

// The F1 series uses a different RTC from others
#if defined STM32F1
# define RTC_TYPE_1
#elif defined STM32F0 || defined STM32F2 || defined STM32F3 || defined STM32F4
# define RTC_TYPE_2
#endif

#if defined RTC_TYPE_1

/// Alternative RTC_TypeDef which uses 32-bit access, this generates better code
typedef struct
{
  __IO uint32_t CRH;
  __IO uint32_t CRL;
  __IO uint32_t PRLH;
  __IO uint32_t PRLL;
  __IO uint32_t DIVH;
  __IO uint32_t DIVL;
  __IO uint32_t CNTH;
  __IO uint32_t CNTL;
  __IO uint32_t ALRH;
  __IO uint32_t ALRL;
} RTC32_TypeDef;

#define RTC32		((RTC32_TypeDef *) RTC_BASE)

/**
 * Call after APB1 reset or APB1 clock stop (including reset)
 */
static inline void rtc_invalidate ()
{
    RTC32->CRL &= ~RTC_CRL_RSF;
}

/**
 * Wait RTC register to syncrhonize with the backup domain.
 * Call after APB1 reset or APB1 clock stop (including reset)
 * to ensure RTC regs valid.
 */
static inline void rtc_wait_sync ()
{
    while ((RTC32->CRL & RTC_CRL_RSF) == 0)
        ;
}

/**
 * Wait until a write to RTC registers is possible, and enable writing.
 * Use rtc_wait_write()/rtc_done_write() to mark the start and the end
 * of a RTC registers write block.
 */
static inline int rtc_wait_write ()
{
    while (!(RTC32->CRL & RTC_CRL_RTOFF))
        ;
    RTC32->CRL |= RTC_CRL_CNF;

    return 1;
}

/// Disable writing to RTC registers
static inline int rtc_done_write ()
{
    RTC32->CRL &= ~RTC_CRL_CNF;

    // Wait until the write completes
    while (!(RTC32->CRL & RTC_CRL_RTOFF))
        ;

    return 0;
}

/**
 * A nice wrapper around @a rtc_wait_write() / @a rtc_done_write ().
 * Usage example:
 * @code
 *  RTC_WRITE
 *  {
 *      rtc_set_counter (0);
 *      rtc_set_alarm (60);
 *  }
 * @endcode
 * The above code is equivalent to:
 * @code
 *  rtc_wait_write ();
 *  rtc_set_counter (0);
 *  rtc_set_alarm (60);
 *  rtc_done_write ();
 * @endcode
 * Please don't 'break' from inside the code block, as you'll leave
 * without notifying RTC of the changed registers. Use 'continue' to
 * jump right to the code after the block.
 */
#define RTC_WRITE \
    for (int __tmp = rtc_wait_write (); __tmp; __tmp = rtc_done_write ())

/// Get the current RTC seconds counter
static inline uint32_t rtc_counter ()
{
    return  (RTC32->CNTL & 0xffff) | (RTC32->CNTH << 16);
}

/// Set the RTC seconds counter
static inline void rtc_set_counter (uint32_t x)
{
    RTC32->CNTL = x;
    RTC32->CNTH = x >> 16;
}

/// Get the current RTC alarm counter
static inline uint32_t rtc_alarm ()
{
    return (RTC32->ALRL & 0xffff) | (RTC32->ALRH << 16);
}

/// Set the RTC alarm counter
static inline void rtc_set_alarm (uint32_t x)
{
    RTC32->ALRL = x;
    RTC32->ALRH = x >> 16;
}

/// Get the current RTC reload counter
static inline uint32_t rtc_reload ()
{
    return (RTC32->PRLL & 0xffff) | (RTC32->PRLH << 16);
}

/// Set RTC reload counter
static inline void rtc_set_reload (uint32_t prl)
{
    RTC32->PRLL = prl;
    RTC32->PRLH = prl >> 16;
}

/// Get current RTC prescaler divider (can be used to get sub-clock accuracy)
static inline uint32_t rtc_divider ()
{
    return (RTC32->DIVL & 0xffff) | ((RTC32->DIVH & 15) << 16);
}

/**
 * Enable RTC interrupts
 * @arg ie
 *      Interrupts to enable (combination of RTC_CRH_XXX bit flags)
 */
static inline void rtc_irqs (uint32_t ie)
{
    RTC32->CRH =
        (RTC32->CRH & ~(RTC_CRH_OWIE | RTC_CRH_ALRIE | RTC_CRH_SECIE)) |
        (ie & (RTC_CRH_OWIE | RTC_CRH_ALRIE | RTC_CRH_SECIE));
}

/// Working modes of the TAMPER pin
typedef enum
{
    /// TAMPER pin is unused by RTC, can be used as GPIO
    rtpNone = 0,
    /// "Second" pulse output for 1 RTC clock (where "second" means "prescaler" ticks)
    rtpSecond = BKP_RTCCR_ASOS | BKP_RTCCR_ASOE,
    /// "Alarm" pulse output for 1 RTC clock
    rtpAlarm = BKP_RTCCR_ASOE,
    /// Output RTC clock divided by 64, used for RTC calibration
    rtpClockDiv64 = BKP_RTCCR_CCO,
    /// TAMPER function, high level on this pin resets all backup registers
    rtpTamperHigh = BKP_CR_TPE,
    /// TAMPER function, low level on this pin resets all backup registers
    rtpTamperLow = BKP_CR_TPE | BKP_CR_TPAL,
} rtc_tamper_pin_t;

/**
 * Set TAMPER pin function.
 * @arg mode
 *      The mode to run TAMPER pin.
 */
static inline void rtc_tamper_pin (rtc_tamper_pin_t mode)
{
    if (mode & BKP_CR_TPE)
    {
        // Datasheet recommends first to disable TPE to avoid spurious TAMPER events
        uint32_t cr = BKP->CR & ~BKP_CR_TPE;
        BKP->CR = cr;
        BKP->CR = (cr & ~BKP_CR_TPAL) | mode;
    }
    else
        BKP->RTCCR = (BKP->RTCCR & ~(BKP_RTCCR_ASOS | BKP_RTCCR_ASOE |
            BKP_RTCCR_CCO)) | mode;
}

/**
 * Enable IRQ when TAMPER event occurs.
 * @arg enable
 *      If true, a IRQ will be generated when TAMPER event occurs.
 */
static inline void rtc_tamper_irq (bool enable)
{
    if (enable)
        BKP->CSR |= BKP_CSR_TPIE;
    else
        BKP->CSR &= ~BKP_CSR_TPIE;
}

/// Query TAMPER IRQ status (non-zero if IRQ was asserted)
static inline uint32_t rtc_tamper_get_irq ()
{
    return BKP->CSR & BKP_CSR_TIF;
}

/// Query TAMPER event status (non-zero if tamper event occured, backup in reset state)
static inline uint32_t rtc_tamper_get_event ()
{
    return BKP->CSR & BKP_CSR_TEF;
}

/// Clear TAMPER IRQ flag
static inline void rtc_tamper_clear_irq ()
{
    BKP->CSR |= BKP_CSR_CTI;
}

/// Clear TAMPER event flag
static inline void rtc_tamper_clear_event ()
{
    BKP->CSR |= BKP_CSR_CTE;
}

/**
 * Calibrate RTC clock. This sets amount of ignored RTC clocks
 * after every 2^20 clock pulses. The maximal amount (127) is equivalent
 * to skipping 10.5 sec per day.
 * @arg calib
 *      Number of clocks to ignore after every 2^20 clock pulses, range 0-127.
 */
static inline void rtc_calibrate (uint32_t calib)
{
    BKP->RTCCR = (BKP->RTCCR & ~BKP_RTCCR_CAL) | (calib & BKP_RTCCR_CAL);
}

// Backup memory size
#if defined STM32F10X_HD || defined STM32F10X_XL || defined STM32F10X_CL
# define BKP_SIZE		(42*2)
#else
# define BKP_SIZE		(10*2)
#endif

/**
 * Get the value of a data register from backup domain.
 * @arg idx
 *      Data index (0-41)
 * @return
 *      The 16-bit value of the corresponding data register
 */
static inline uint16_t bkp_data (uint32_t idx)
{
    return (idx < 10) ? ((uint16_t *)&BKP->DR1) [idx * 2] :
#if BKP_SIZE > (10*2)
        (idx < 42) ? ((uint16_t *)&BKP->DR11) [(idx - 10) * 2] :
#endif
        0;
}

/**
 * Set the value of a data register from backup domain.
 * @arg idx
 *      Data index (0-41)
 * @arg val
 *      The new 16-bit value of the corresponding data register
 */
static inline void bkp_set_data (uint32_t idx, uint16_t val)
{
    if (idx < 10)
        ((uint16_t *)&BKP->DR1) [idx * 2] = val;
#if BKP_SIZE > (10*2)
    else if (idx < 42)
        ((uint16_t *)&BKP->DR11) [(idx - 10) * 2] = val;
#endif
}

/**
 * Save a continuous region of memory into backup registers.
 * This is easier than manually splitting data into 16-bit pieces.
 * @arg idx
 *      Starting register index
 * @arg data
 *      A pointer to saved data
 * @arg size
 *      Data size in bytes
 */
extern void bkp_save (uint32_t idx, const void *data, uint32_t size);

/**
 * Load a continuous region of memory from backup registers.
 * @arg idx
 *      Starting register index
 * @arg data
 *      A pointer to memory region where data should be placed
 * @arg size
 *      Data size in bytes
 */
extern void bkp_load (uint32_t idx, void *data, uint32_t size);

#elif defined RTC_TYPE_2

//#warning "RTC support for this MCU series not implemented yet"

#endif

/**
 * Initialize the RTC.
 * Checks the RTC and if it has not been initialized yet,
 * sets it to emit RTC_TICKS_PER_SEC ticks per second.
 */
extern void rtc_init ();

/// Reset the RTC completely
extern void rtc_reset ();

#endif // _RTC_H
