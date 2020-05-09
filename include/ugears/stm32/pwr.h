/*
    STM32 Backup domain library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _PWR_H
#define _PWR_H

/**
 * @file pwr.h
 *      A set of functions for easier management of power unit.
 */

/**
 * Enable/disable write access to RTC & backup registers.
 * @arg enable
 *      If true, allow writes to RTC & backup registers, disallow otherwise.
 */
static inline void pwr_rtc_backup (bool enable)
{
    if (enable)
        PWR->CR |= PWR_CR_DBP;
    else
        PWR->CR &= ~PWR_CR_DBP;
}

// Should test for PWR_PVD_SUPPORT, but it's not present in all files
#ifdef PWR_CR_PLS

/**
 * Set Power Voltage Detector level and enable it.
 * @arg pvd
 *      Voltage threshold (see PWR_CR_PLS_XXX).
 */
static inline void pwr_pvd (uint32_t pvd)
{
    PWR->CR = (PWR->CR & ~PWR_CR_PLS) | (pvd & PWR_CR_PLS) | PWR_CR_PVDE;
}

/**
 * Check Power Voltage Detector status.
 * @return
 *      Non-zero if VDD is lower than PVD threshold.
 */
static inline uint32_t pwr_pvd_get ()
{
    return (PWR->CSR & PWR_CSR_PVDO);
}

#endif // PWR_PVD_SUPPORT

/**
 * Enable or disable the usage of WKUP pins. Some controllers support only one pin,
 * some may support up to 8 wakeup pins, see the datasheet for your controller
 * for details.
 * @arg mask
 *      If bit #n is set, a rising edge on the WKUP#(n+1) pin will wake up
 *      the processor from the Standby mode. The pin is in input pull-down
 *      configuration. If 0, the WKUP pin can be used as a regular IO.
 */
static inline void pwr_wkup (unsigned mask)
{
#if defined PWR_CSR_EWUP8
    // We have 8 wakeup pins
    PWR->CSR = (PWR->CSR & ~(PWR_CSR_EWUP1 | PWR_CSR_EWUP2 | PWR_CSR_EWUP3 | PWR_CSR_EWUP4 |
        PWR_CSR_EWUP5 | PWR_CSR_EWUP6 | PWR_CSR_EWUP7 | PWR_CSR_EWUP8)) | ((mask & 0xFF) << 8);
#elif defined PWR_CSR_EWUP7
    // We have 7 wakeup pins
    PWR->CSR = (PWR->CSR & ~(PWR_CSR_EWUP1 | PWR_CSR_EWUP2 | PWR_CSR_EWUP3 | PWR_CSR_EWUP4 |
        PWR_CSR_EWUP5 | PWR_CSR_EWUP6 | PWR_CSR_EWUP7)) | ((mask & 0x7F) << 8);
#elif defined PWR_CSR_EWUP3
    // Only three wakeup pins
    if (PWR_CSR_EWUP3_Pos < PWR_CSR_EWUP1_Pos) // on some MCUs bits have reverse direction
        mask = ((mask & 4) >> 2) | (mask & 2) | ((mask & 1) << 2);
    PWR->CSR = (PWR->CSR & ~(PWR_CSR_EWUP1 | PWR_CSR_EWUP2 | PWR_CSR_EWUP3)) |
        ((mask & 0x07) << 8);
#elif defined PWR_CSR_EWUP2
    // Only two
    if (PWR_CSR_EWUP2_Pos < PWR_CSR_EWUP1_Pos) // on some MCUs bits have reverse direction
        mask = ((mask & 2) >> 1) | ((mask & 1) << 1);
    PWR->CSR = (PWR->CSR & ~(PWR_CSR_EWUP1 | PWR_CSR_EWUP2)) |
        ((mask & 0x03) << 8);
#elif defined PWR_CSR_EWUP
    // Only one
    PWR->CSR = (PWR->CSR & ~PWR_CSR_EWUP) | ((mask & 1) << 8);
#else
#  error "No support for wakeup pin on this controller"
#endif
}

/**
 * Check if device has been in Standby mode.
 * This flag can be cleared with the pwr_clear_standby () function.
 * @return
 *      Non-zero if device was in Standby mode, zero if not.
 */
static inline uint32_t pwr_standby_get ()
{
    return PWR->CSR & PWR_CSR_SBF;
}

/// Clear Standby flag
static inline void pwr_standby_clear ()
{
    PWR->CR |= PWR_CR_CSBF;
}

/**
 * Check if device woke up through the WKUP pin or RTC Alarm.
 * This flag can be cleared with the pwr_clear_wakeup () function.
 * @return
 *      Non-zero if device woke up through WKUP or RTC Alarm, zero if not.
 */
static inline uint32_t pwr_wakeup_get ()
{
    return PWR->CSR & PWR_CSR_WUF;
}

/// Clear Wakeup flag
static inline void pwr_wakeup_clear ()
{
    PWR->CR |= PWR_CR_CWUF;
}

typedef enum
{
    /**
     * CPU clock off, all peripherals including Cortex-M3 core peripherals like
     * NVIC, SysTick, etc. are kept running.
     */
    psmSleep = 0,

    /**
     * All clocks in the 1.8 V domain are stopped, the PLL, the HSI and the HSE RC
     * oscillators are disabled. SRAM and register contents are preserved.
     */
    psmStop,

    /**
     * Sleep mode + the internal voltage regulator in low-power mode.
     * An additional startup delay is incurred when waking up from Stop mode.
     */
    psmStopLP,

    /**
     * Lowest power consumption. The 1.8 V domain is powered off. The PLL, the HSI
     * oscillator and the HSE oscillator are also switched off. SRAM and register
     * contents are lost except for registers in the Backup domain and Standby circuitry.
     */
    psmStandby,
} pwr_sleep_mode_t;

/// Sleep until a interrupt occurs
#define SLEEP_WFI		0x00000000
/// Sleep until a event occurs
#define SLEEP_WFE		0x00000001
/// Start sleeping when the lowest-priority ISR exits
#define SLEEP_ONEXIT		0x00000002
/// Interpret any pending IRQ, disabled in NVIC, as a event
#define SLEEP_EVONIRQ		0x00000004

/**
 * Set up power unit, then enter SLEEPDEEP CPU mode.
 * @arg pwr_mode
 *      One of the psmXXX constants
 * @arg flags
 *      A combination of SLEEP_XXX bit flags
 */
static inline void pwr_sleep (pwr_sleep_mode_t pwr_mode, uint32_t flags)
{
    // Clear all EXTI pending interrupts/events
    exti_clear_all ();

    uint32_t pwr_flags =
        (pwr_mode == psmSleep) ? 0 :
        (pwr_mode == psmStop) ? 0 :
        (pwr_mode == psmStopLP) ? PWR_CR_LPDS :
        /*(pwr_mode == psmSuspend) ?*/ PWR_CR_PDDS;

    // Set sleep mode, clear standby & wakeup flags
    PWR->CR = (PWR->CR & ~(PWR_CR_PDDS | PWR_CR_LPDS)) | pwr_flags |
        (PWR_CR_CWUF | PWR_CR_CSBF);

    uint32_t scr = SCB->SCR &
        ~(SCB_SCR_SLEEPONEXIT_Msk | SCB_SCR_SEVONPEND_Msk | SCB_SCR_SLEEPDEEP_Msk);

    if (flags & SLEEP_ONEXIT)
        scr |= SCB_SCR_SLEEPONEXIT_Msk;
    if (flags & SLEEP_EVONIRQ)
        scr |= SCB_SCR_SEVONPEND_Msk;
    if (pwr_mode != psmSleep)
        scr |= SCB_SCR_SLEEPDEEP_Msk;

    SCB->SCR = scr;

    if (flags & SLEEP_WFE)
        __WFE ();
    else
        __WFI ();
}

#endif // _PWR_H
