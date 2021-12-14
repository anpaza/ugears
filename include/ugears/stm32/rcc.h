/*
    STM32 Reset and Clock Control
    Copyright (C) 2015 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_RCC_H
#define _STM32_RCC_H

/**
 * @file rcc.h
 *      Some handy macros to deal with the reset and clock control
 *      circuit of the STM32 microcontroller.
 *
 * This is an attempt to wrap around the Reset and Enable bits into
 * something more platform-independent.
 */

#include "cmsis.h"

/// On some MCUs ADC1RST is named ADCRST
#if defined RCC_APB2RSTR_ADCRST && !defined RCC_APB2RSTR_ADC1RST
#  define RCC_APB2RSTR_ADC1RST		RCC_APB2RSTR_ADCRST
#endif

#if defined STM32F0

// Peripherial -> RCC register name mapping

#define RCC_REG_DMA1			AHB
#define RCC_REG_SRAM			AHB
#define RCC_REG_FLITF			AHB
#define RCC_REG_CRC			AHB
#define RCC_REG_GPIOA			AHB
#define RCC_REG_GPIOB			AHB
#define RCC_REG_GPIOC			AHB
#define RCC_REG_GPIOD			AHB
#define RCC_REG_GPIOE			AHB
#define RCC_REG_GPIOF			AHB
#define RCC_REG_TSC			AHB

#define RCC_REG_TIM2			APB1
#define RCC_REG_TIM3			APB1
#define RCC_REG_TIM6			APB1
#define RCC_REG_TIM7			APB1
#define RCC_REG_TIM14			APB1
#define RCC_REG_WWDG			APB1
#define RCC_REG_SPI2			APB1
#define RCC_REG_USART2			APB1
#define RCC_REG_USART3			APB1
#define RCC_REG_USART4			APB1
#define RCC_REG_USART5			APB1
#define RCC_REG_I2C1			APB1
#define RCC_REG_I2C2			APB1
#define RCC_REG_USB			APB1
#define RCC_REG_PWR			APB1
#define RCC_REG_DAC			APB1
#define RCC_REG_CEC			APB1

#define RCC_REG_SYSCFG			APB2
#define RCC_REG_USART6			APB2
#define RCC_REG_ADC1			APB2
#define RCC_REG_TIM1			APB2
#define RCC_REG_SPI1			APB2
#define RCC_REG_USART1			APB2
#define RCC_REG_TIM15			APB2
#define RCC_REG_TIM16			APB2
#define RCC_REG_TIM17			APB2
#define RCC_REG_DBGMCU			APB2

// Group reset/enable/disable helper macros
#define _RCC_REGS \
    AHBENR  = 0, APB1ENR  = 0, APB2ENR  = 0, \
    AHBDISR = 0, APB1DISR = 0, APB2DISR = 0, \
    AHBRSTR = 0, APB1RSTR = 0, APB2RSTR = 0
#define _RCC_APPLY \
    if (AHBENR) { RCC->AHBENR |= AHBENR; } \
    if (APB1ENR) { RCC->APB1ENR |= APB1ENR; } \
    if (APB2ENR) { RCC->APB2ENR |= APB2ENR; } \
    if (AHBDISR) { RCC->AHBENR &= ~AHBDISR; } \
    if (APB1DISR) { RCC->APB1ENR &= ~APB1DISR; } \
    if (APB2DISR) { RCC->APB2ENR &= ~APB2DISR; } \
    if (AHBRSTR) { RCC->AHBRSTR |= AHBRSTR; RCC->AHBRSTR &= ~AHBRSTR; } \
    if (APB1RSTR) { RCC->APB1RSTR |= APB1RSTR; RCC->APB1RSTR &= ~APB1RSTR; } \
    if (APB2RSTR) { RCC->APB2RSTR |= APB2RSTR; RCC->APB2RSTR &= ~APB2RSTR; }

#elif defined STM32F1

#ifdef RCC_AHBRSTR_OTGFSRST
#define RCC_REG_OTGFS			AHB
#define RCC_REG_ETHMAC			AHB
#endif

#define RCC_REG_DMA1			AHB
#define RCC_REG_SRAM			AHB
#define RCC_REG_FLITF			AHB
#define RCC_REG_CRC			AHB

#ifdef RCC_AHBENR_DMA2EN
#define RCC_REG_DMA2			AHB
#endif

#ifdef RCC_AHBENR_FSMCEN
#define RCC_REG_FSMC			AHB
#define RCC_REG_SDIO			AHB
#endif

#ifdef RCC_AHBENR_OTGFSEN
#define RCC_REG_OTGFS			AHB
#define RCC_REG_ETHMAC			AHB
#define RCC_REG_ETHMACTX		AHB
#define RCC_REG_ETHMACRX		AHB
#endif

// STM32F1 series is the only that uses "IOP" instead of "GPIO" in RCC bit names.
// Define the GPIO aliases for compatibility with other series
#define RCC_APB2ENR_GPIOAEN		RCC_APB2ENR_IOPAEN
#define RCC_APB2ENR_GPIOBEN		RCC_APB2ENR_IOPBEN
#define RCC_APB2ENR_GPIOCEN		RCC_APB2ENR_IOPCEN
#define RCC_APB2ENR_GPIODEN		RCC_APB2ENR_IOPDEN
#define RCC_APB2ENR_GPIOEEN		RCC_APB2ENR_IOPEEN
#define RCC_APB2ENR_GPIOFEN		RCC_APB2ENR_IOPFEN
#define RCC_APB2ENR_GPIOGEN		RCC_APB2ENR_IOPGEN

#define RCC_APB2RSTR_GPIOARST		RCC_APB2RSTR_IOPARST
#define RCC_APB2RSTR_GPIOBRST		RCC_APB2RSTR_IOPBRST
#define RCC_APB2RSTR_GPIOCRST		RCC_APB2RSTR_IOPCRST
#define RCC_APB2RSTR_GPIODRST		RCC_APB2RSTR_IOPDRST
#define RCC_APB2RSTR_GPIOERST		RCC_APB2RSTR_IOPERST
#define RCC_APB2RSTR_GPIOFRST		RCC_APB2RSTR_IOPFRST
#define RCC_APB2RSTR_GPIOGRST		RCC_APB2RSTR_IOPGRST

#define RCC_REG_AFIO			APB2
#define RCC_REG_GPIOA			APB2
#define RCC_REG_GPIOB			APB2
#define RCC_REG_GPIOC			APB2
#define RCC_REG_GPIOD			APB2
#define RCC_REG_ADC1			APB2

#ifdef RCC_APB2RSTR_ADC2RST
#define RCC_REG_ADC2			APB2
#endif

#define RCC_REG_TIM1			APB2
#define RCC_REG_SPI1			APB2
#define RCC_REG_USART1			APB2

#ifdef RCC_APB2RSTR_TIM15RST
#define RCC_REG_TIM15			APB2
#define RCC_REG_TIM16			APB2
#define RCC_REG_TIM17			APB2
#endif

#ifdef RCC_APB2RSTR_GPIOERST
#define RCC_REG_GPIOE			APB2
#endif

#ifdef RCC_APB2RSTR_GPIOFRST
#define RCC_REG_GPIOF			APB2
#define RCC_REG_GPIOG			APB2
#define RCC_REG_TIM8			APB2
#define RCC_REG_ADC3			APB2
#endif

#ifdef RCC_APB2RSTR_TIM9RST
#define RCC_REG_TIM9			APB2
#define RCC_REG_TIM10			APB2
#define RCC_REG_TIM11			APB2
#endif

#define RCC_REG_TIM2			APB1
#define RCC_REG_TIM3			APB1
#define RCC_REG_WWDG			APB1
#define RCC_REG_USART2			APB1
#define RCC_REG_I2C1			APB1

#ifdef RCC_APB1RSTR_CAN1RST
#define RCC_REG_CAN1			APB1
#endif

#define RCC_REG_BKP			APB1
#define RCC_REG_PWR			APB1

#ifdef RCC_APB1RSTR_TIM4RST
#define RCC_REG_TIM4			APB1
#define RCC_REG_SPI2			APB1
#define RCC_REG_USART3			APB1
#define RCC_REG_I2C2			APB1
#endif

#ifdef RCC_APB1RSTR_USBRST
#define RCC_REG_USB			APB1
#endif

#ifdef RCC_APB1RSTR_TIM5RST
#define RCC_REG_TIM5			APB1
#define RCC_REG_TIM6			APB1
#define RCC_REG_TIM7			APB1
#define RCC_REG_SPI3			APB1
#define RCC_REG_UART4			APB1
#define RCC_REG_UART5			APB1
#define RCC_REG_DAC			APB1
#endif

#ifdef RCC_APB1RSTR_TIM6RST
#define RCC_REG_TIM6			APB1
#define RCC_REG_TIM7			APB1
#define RCC_REG_DAC			APB1
#define RCC_REG_CEC			APB1
#endif

#ifdef RCC_APB1RSTR_CAN2RST
#define RCC_REG_CAN2			APB1
#endif

#ifdef RCC_APB1RSTR_TIM12RST
#define RCC_REG_TIM12			APB1
#define RCC_REG_TIM13			APB1
#define RCC_REG_TIM14			APB1
#endif

// Group reset/enable/disable helper macros
#define _RCC_REGS \
    AHBENR  = 0, APB1ENR  = 0, APB2ENR  = 0, \
    AHBDISR = 0, APB1DISR = 0, APB2DISR = 0, \
                 APB1RSTR = 0, APB2RSTR = 0
#define _RCC_APPLY \
    if (AHBENR) { RCC->AHBENR |= AHBENR; } \
    if (APB1ENR) { RCC->APB1ENR |= APB1ENR; } \
    if (APB2ENR) { RCC->APB2ENR |= APB2ENR; } \
    if (AHBDISR) { RCC->AHBENR &= ~AHBDISR; } \
    if (APB1DISR) { RCC->APB1ENR &= ~APB1DISR; } \
    if (APB2DISR) { RCC->APB2ENR &= ~APB2DISR; } \
    if (APB1RSTR) { RCC->APB1RSTR |= APB1RSTR; RCC->APB1RSTR &= ~APB1RSTR; } \
    if (APB2RSTR) { RCC->APB2RSTR |= APB2RSTR; RCC->APB2RSTR &= ~APB2RSTR; }

#elif defined STM32F2

#error "STM32F2 support not implemented yet"

#elif defined STM32F3

#error "STM32F3 support not implemented yet"

#elif defined STM32F4

#define RCC_REG_GPIOA			AHB1
#define RCC_REG_GPIOB			AHB1
#define RCC_REG_GPIOC			AHB1
#define RCC_REG_GPIOD			AHB1
#define RCC_REG_GPIOE			AHB1
#define RCC_REG_GPIOF			AHB1
#define RCC_REG_GPIOG			AHB1
#define RCC_REG_GPIOH			AHB1
#define RCC_REG_GPIOI			AHB1
#define RCC_REG_GPIOJ			AHB1
#define RCC_REG_GPIOK			AHB1
#define RCC_REG_CRC  			AHB1
#define RCC_REG_DMA1                    AHB1
#define RCC_REG_DMA2                    AHB1
#define RCC_REG_DMA2D                   AHB1
#define RCC_REG_ETHMAC                  AHB1
#define RCC_REG_OTGH                    AHB1

#define RCC_REG_DCMI                    AHB2
#define RCC_REG_CRYP                    AHB2
#define RCC_REG_HASH                    AHB2
#define RCC_REG_RNG                     AHB2
#define RCC_REG_OTGFS                   AHB2

#ifdef RCC_AHB3RSTR_FSMCRST
#define RCC_REG_FSMC                    AHB3
#endif
#ifdef RCC_AHB3RSTR_FMCRST
#define RCC_REG_FMC                     AHB3
#endif

#define RCC_REG_TIM2                    APB1
#define RCC_REG_TIM3                    APB1
#define RCC_REG_TIM4                    APB1
#define RCC_REG_TIM5                    APB1
#define RCC_REG_TIM6                    APB1
#define RCC_REG_TIM7                    APB1
#define RCC_REG_TIM12                   APB1
#define RCC_REG_TIM13                   APB1
#define RCC_REG_TIM14                   APB1
#define RCC_REG_WWDG                    APB1
#define RCC_REG_SPI2                    APB1
#define RCC_REG_SPI3                    APB1
#define RCC_REG_USART2                  APB1
#define RCC_REG_USART3                  APB1
#define RCC_REG_UART4                   APB1
#define RCC_REG_UART5                   APB1
#define RCC_REG_I2C1                    APB1
#define RCC_REG_I2C2                    APB1
#define RCC_REG_I2C3                    APB1
#define RCC_REG_CAN1                    APB1
#define RCC_REG_CAN2                    APB1
#define RCC_REG_PWR                     APB1
#define RCC_REG_DAC                     APB1
#define RCC_REG_UART7                   APB1
#define RCC_REG_UART8                   APB1

#define RCC_REG_TIM1                    APB2
#define RCC_REG_TIM8                    APB2
#define RCC_REG_USART1                  APB2
#define RCC_REG_USART6                  APB2
#define RCC_REG_ADC1                    APB2
#define RCC_REG_ADC2                    APB2
#define RCC_REG_ADC3                    APB2
#define RCC_REG_SDIO                    APB2
#define RCC_REG_SPI1                    APB2
#define RCC_REG_SPI4                    APB2
#define RCC_REG_SYSCFG                  APB2
#define RCC_REG_TIM9                    APB2
#define RCC_REG_TIM10                   APB2
#define RCC_REG_TIM11                   APB2
#define RCC_REG_SPI5                    APB2
#define RCC_REG_SPI6                    APB2
#define RCC_REG_SAI1                    APB2
#define RCC_REG_LTDC                    APB2

#define RCC_REG_GPIOALP                 AHB1LP
#define RCC_REG_GPIOBLP                 AHB1LP
#define RCC_REG_GPIOCLP                 AHB1LP
#define RCC_REG_GPIODLP                 AHB1LP
#define RCC_REG_GPIOELP                 AHB1LP
#define RCC_REG_GPIOFLP                 AHB1LP
#define RCC_REG_GPIOGLP                 AHB1LP
#define RCC_REG_GPIOHLP                 AHB1LP
#define RCC_REG_GPIOILP                 AHB1LP
#define RCC_REG_GPIOJLP                 AHB1LP
#define RCC_REG_GPIOKLP                 AHB1LP
#define RCC_REG_CRCLP                   AHB1LP
#define RCC_REG_FLITFLP                 AHB1LP
#define RCC_REG_SRAM1LP                 AHB1LP
#define RCC_REG_SRAM2LP                 AHB1LP
#define RCC_REG_BKPSRAMLP               AHB1LP
#define RCC_REG_SRAM3LP                 AHB1LP
#define RCC_REG_DMA1LP                  AHB1LP
#define RCC_REG_DMA2LP                  AHB1LP
#define RCC_REG_DMA2DLP                 AHB1LP
#define RCC_REG_ETHMACLP                AHB1LP
#define RCC_REG_ETHMACTXLP              AHB1LP
#define RCC_REG_ETHMACRXLP              AHB1LP
#define RCC_REG_ETHMACPTPLP             AHB1LP
#define RCC_REG_OTGHSLP                 AHB1LP
#define RCC_REG_OTGHSULPILP             AHB1LP

#define RCC_REG_DCMILP                  AHB2LP
#define RCC_REG_CRYPLP                  AHB2LP
#define RCC_REG_HASHLP                  AHB2LP
#define RCC_REG_RNGLP                   AHB2LP
#define RCC_REG_OTGFSLP                 AHB2LP

#ifdef RCC_AHB3LPENR_FSMCLPEN
#define RCC_REG_FSMCLP                  AHB3LP
#endif
#ifdef RCC_AHB3LPENR_FMCLPEN
#define RCC_REG_FMCLP                   AHB3LP
#endif

#define RCC_REG_TIM2LP                  APB1LP
#define RCC_REG_TIM3LP                  APB1LP
#define RCC_REG_TIM4LP                  APB1LP
#define RCC_REG_TIM5LP                  APB1LP
#define RCC_REG_TIM6LP                  APB1LP
#define RCC_REG_TIM7LP                  APB1LP
#define RCC_REG_TIM12LP                 APB1LP
#define RCC_REG_TIM13LP                 APB1LP
#define RCC_REG_TIM14LP                 APB1LP
#define RCC_REG_WWDGLP                  APB1LP
#define RCC_REG_SPI2LP                  APB1LP
#define RCC_REG_SPI3LP                  APB1LP
#define RCC_REG_USART2LP                APB1LP
#define RCC_REG_USART3LP                APB1LP
#define RCC_REG_UART4LP                 APB1LP
#define RCC_REG_UART5LP                 APB1LP
#define RCC_REG_I2C1LP                  APB1LP
#define RCC_REG_I2C2LP                  APB1LP
#define RCC_REG_I2C3LP                  APB1LP
#define RCC_REG_CAN1LP                  APB1LP
#define RCC_REG_CAN2LP                  APB1LP
#define RCC_REG_PWRLP                   APB1LP
#define RCC_REG_DACLP                   APB1LP
#define RCC_REG_UART7LP                 APB1LP
#define RCC_REG_UART8LP                 APB1LP

#define RCC_REG_TIM1LP                  APB2LP
#define RCC_REG_TIM8LP                  APB2LP
#define RCC_REG_USART1LP                APB2LP
#define RCC_REG_USART6LP                APB2LP
#define RCC_REG_ADC1LP                  APB2LP
#define RCC_REG_ADC2P                   APB2LP
#define RCC_REG_ADC3LP                  APB2LP
#define RCC_REG_SDIOLP                  APB2LP
#define RCC_REG_SPI1LP                  APB2LP
#define RCC_REG_SPI4LP                  APB2LP
#define RCC_REG_SYSCFGLP                APB2LP
#define RCC_REG_TIM9LP                  APB2LP
#define RCC_REG_TIM10LP                 APB2LP
#define RCC_REG_TIM11LP                 APB2LP
#define RCC_REG_SPI5LP                  APB2LP
#define RCC_REG_SPI6LP                  APB2LP
#define RCC_REG_SAI1LP                  APB2LP
#define RCC_REG_LTDCLP                  APB2LP

// Group reset/enable/disable helper macros
#define _RCC_REGS \
    AHB1ENR = 0, AHB2ENR = 0, AHB3ENR = 0, APB1ENR = 0, APB2ENR = 0, \
    AHB1LPENR = 0, AHB2LPENR = 0, AHB3LPENR = 0, APB1LPENR = 0, APB2LPENR = 0, \
    AHB1DISR = 0, AHB2DISR = 0, AHB3DISR = 0, APB1DISR = 0, APB2DISR = 0, \
    AHB1LPDISR = 0, AHB2LPDISR = 0, AHB3LPDISR = 0, APB1LPDISR = 0, APB2LPDISR = 0, \
    AHB1RSTR = 0, AHB2RSTR = 0, AHB3RSTR = 0, APB1RSTR = 0, APB2RSTR = 0
#define _RCC_APPLY \
    if (AHB1ENR) { RCC->AHB1ENR |= AHB1ENR; } \
    if (AHB2ENR) { RCC->AHB2ENR |= AHB2ENR; } \
    if (AHB3ENR) { RCC->AHB3ENR |= AHB3ENR; } \
    if (APB1ENR) { RCC->APB1ENR |= APB1ENR; } \
    if (APB2ENR) { RCC->APB2ENR |= APB2ENR; } \
    if (AHB1LPENR) { RCC->AHB1LPENR |= AHB1LPENR; } \
    if (AHB2LPENR) { RCC->AHB2LPENR |= AHB2LPENR; } \
    if (AHB3LPENR) { RCC->AHB3LPENR |= AHB3LPENR; } \
    if (APB1LPENR) { RCC->APB1LPENR |= APB1LPENR; } \
    if (APB2LPENR) { RCC->APB2LPENR |= APB2LPENR; } \
    if (AHB1DISR) { RCC->AHB1ENR &= ~AHB1DISR; } \
    if (AHB2DISR) { RCC->AHB2ENR &= ~AHB2DISR; } \
    if (AHB3DISR) { RCC->AHB3ENR &= ~AHB3DISR; } \
    if (APB1DISR) { RCC->APB1ENR &= ~APB1DISR; } \
    if (APB2DISR) { RCC->APB2ENR &= ~APB2DISR; } \
    if (AHB1LPDISR) { RCC->AHB1LPENR &= ~AHB1LPDISR; } \
    if (AHB2LPDISR) { RCC->AHB2LPENR &= ~AHB2LPDISR; } \
    if (AHB3LPDISR) { RCC->AHB3LPENR &= ~AHB3LPDISR; } \
    if (APB1LPDISR) { RCC->APB1LPENR &= ~APB1LPDISR; } \
    if (APB2LPDISR) { RCC->APB2LPENR &= ~APB2LPDISR; } \
    if (AHB1RSTR) { RCC->AHB1RSTR |= AHB1RSTR; RCC->AHB1RSTR &= ~AHB1RSTR; } \
    if (AHB2RSTR) { RCC->AHB2RSTR |= AHB2RSTR; RCC->AHB2RSTR &= ~AHB2RSTR; } \
    if (AHB3RSTR) { RCC->AHB3RSTR |= AHB3RSTR; RCC->AHB3RSTR &= ~AHB3RSTR; } \
    if (APB1RSTR) { RCC->APB1RSTR |= APB1RSTR; RCC->APB1RSTR &= ~APB1RSTR; } \
    if (APB2RSTR) { RCC->APB2RSTR |= APB2RSTR; RCC->APB2RSTR &= ~APB2RSTR; }

#endif

/**
 * Get reset register name for given peripherial:
 * ((RCC_REG + _ADC1) = APB2) + RSTR = APB2RSTR
 * @arg p
 *   Peripherial name with underscore prepended (e.g. _USART1 etc).
 *   The underscore prefix is needed because e.g. USART1 is a macro
 *   defined in CMSIS header files.
 */
#define RCC_RSTR(p)			JOIN2 (JOIN2 (RCC_REG, p), RSTR)
/**
 * Get enable register name for given peripherial:
 * ((RCC_REG + _ADC1) = APB2) + ENR = APB2ENR
 * @arg p
 *   Peripherial name with underscore prepended (e.g. _USART1 etc).
 *   The underscore prefix is needed because e.g. USART1 is a macro
 *   defined in CMSIS header files.
 */
#define RCC_ENR(p)			JOIN2 (JOIN2 (RCC_REG, p), ENR)

/// RCC GPIO peripherial name by hw feature: RCC_GPIO (LED) -> _GPIOA
#define RCC_GPIO(x)			JOIN2 (_GPIO, GPIO_PORT (x))
/// RCC USART peripherial name by hw feature: SERIAL -> _USART1
#define RCC_USART(x)			JOIN2 (_USART, USART_NUM (x))
/// RCC UART peripherial name by hw feature: SERIAL -> _UART1
#define RCC_UART(x)			JOIN2 (_UART, USART_NUM (x))
/// RCC ADC peripherial name by hw feature: TEMP_INT -> _ADC1
#define RCC_ADC(x)			JOIN2 (_ADC, ADC_NUM (x))
/// RCC DMA peripherial name by hw feature: ADC -> _DMA1
#define RCC_DMA(x)			JOIN2 (_DMA, DMA_NUM (x))
/// RCC I2C peripherial name by hw feature: INA220 -> _I2C1
#define RCC_I2C(x)			JOIN2 (_I2C, I2C_NUM (x))
/// RCC SPI peripherial name by hw feature: DISPLAY -> _SPI1
#define RCC_SPI(x)			JOIN2 (_SPI, SPI_NUM (x))
/// RCC timer peripherial name by hw feature: PWM -> _TIM2
#define RCC_TIM(x)			JOIN2 (_TIM, TIM_NUM (x))
/// RCC CAN peripherial name by hw feature: BUS -> _CAN1
#define RCC_CAN(x)			JOIN2 (_CAN, CAN_NUM (x))

/**
 * Reset a specific MCU peripherial
 * @arg p
 *   Peripherial name with underscore prepended (e.g. _USART1 etc).
 *   The underscore prefix is needed because e.g. USART1 is a macro
 *   defined in CMSIS header files.
 */
#define RCC_RESET(p) \
    { \
        RCC->RCC_RSTR(p) |= JOIN4 (RCC_,RCC_RSTR(p),p,RST); \
        RCC->RCC_RSTR(p) &= ~JOIN4 (RCC_,RCC_RSTR(p),p,RST); \
    }

/**
 * Enable a specific MCU peripherial
 * @arg p
 *   Peripherial name with underscore prepended (e.g. _USART1 etc).
 *   The underscore prefix is needed because e.g. USART1 is a macro
 *   defined in CMSIS header files.
 */
#define RCC_ENABLE(p) \
    RCC->RCC_ENR(p) |= JOIN4 (RCC_,RCC_ENR(p),p,EN)

/**
 * Disable a specific MCU peripherial
 * @arg p
 *   Peripherial name with underscore prepended (e.g. _USART1 etc).
 *   The underscore prefix is needed because e.g. USART1 is a macro
 *   defined in CMSIS header files.
 */
#define RCC_DISABLE(p) \
    RCC->RCC_ENR(p) &= ~JOIN4 (RCC_,RCC_ENR(p),p,EN)

/**
 * Check if a peripherial is currently enabled
 * @arg p
 *   Peripherial name with underscore prepended (e.g. _USART1 etc).
 *   The underscore prefix is needed because e.g. USART1 is a macro
 *   defined in CMSIS header files.
 */
#define RCC_ENABLED(p) \
    (RCC->RCC_ENR(p) & JOIN4 (RCC_,RCC_ENR(p),p,EN))

/**
 * Start a group of reset/enable/disable directives. This has the advantage
 * of doing it all in one operation, rather than modifying one bit at
 * a time. Also it, obviously, generates shorter code - compiler takes
 * care of optimizing out unused code.
 *
 * Inside the RCC_BEGIN ... RCC_END block you may invoke any number of
 * RCC_ENA, RCC_DIS, RCC_RES and their peripherial-specific versions
 * (using hardware feature name as argument).
 *
 * Usage example:
 * @code
 * RCC_BEGIN;
 *      RCC_ENA (_GPIOA);
 *      RCC_ENA (_AFIO);
 *      RCC_ENA (_USART1);
 *      RCC_DIS (_SPI1);
 *      RCC_RES (_USB1);
 *      RCC_RES (_CEC);
 *      RCC_ENA_GPIO (LED);
 *      RCC_DIS_USART (CONSOLE);
 * RCC_END;
 * @endcode
 */
#define RCC_BEGIN \
    { \
        uint32_t _RCC_REGS

/**
 * Same as RCC_RESET(), but used inside a RCC_BEGIN/END block.
 */
#define RCC_RES(p) \
        RCC_RSTR(p) |= JOIN4 (RCC_,RCC_RSTR(p),p,RST)

/**
 * Same as RCC_ENABLE(), but used inside a RCC_BEGIN/END block.
 */
#define RCC_ENA(p) \
        RCC_ENR(p) |= JOIN4 (RCC_,RCC_ENR(p),p,EN)

/**
 * Same as RCC_DISABLE(), but used inside a RCC_BEGIN/END block.
 */
#define RCC_DIS(p) \
        JOIN2 (JOIN2 (RCC_REG, p), DISR) |= JOIN4 (RCC_,RCC_ENR(p),p,EN)

/**
 * End a group of reset/enable/disable directives.
 */
#define RCC_END \
        _RCC_APPLY; \
    }

// -------- // Shortcuts for RCC_XXX (RCC_GPIO|USART|DMA|... (x)) // -------- //

/// Same as RCC_RESET (RCC_GPIO (x))
#define RCC_RESET_GPIO(x)		RCC_RESET (RCC_GPIO (x))
/// Same as RCC_RESET (RCC_USART (x))
#define RCC_RESET_USART(x)		RCC_RESET (RCC_USART (x))
/// Same as RCC_RESET (RCC_ADC (x))
#define RCC_RESET_ADC(x)		RCC_RESET (RCC_ADC (x))
/// Same as RCC_RESET (RCC_DMA (x))
#define RCC_RESET_DMA(x)		RCC_RESET (RCC_DMA (x))
/// Same as RCC_RESET (RCC_I2C (x))
#define RCC_RESET_I2C(x)		RCC_RESET (RCC_I2C (x))
/// Same as RCC_RESET (RCC_SPI (x))
#define RCC_RESET_SPI(x)		RCC_RESET (RCC_SPI (x))
/// Same as RCC_RESET (RCC_TIM (x))
#define RCC_RESET_TIM(x)		RCC_RESET (RCC_TIM (x))
/// Same as RCC_RESET (RCC_CAN (x))
#define RCC_RESET_CAN(x)		RCC_RESET (RCC_CAN (x))

/// Same as RCC_ENABLE (RCC_GPIO (x))
#define RCC_ENABLE_GPIO(x)		RCC_ENABLE (RCC_GPIO (x))
/// Same as RCC_ENABLE (RCC_USART (x))
#define RCC_ENABLE_USART(x)		RCC_ENABLE (RCC_USART (x))
/// Same as RCC_ENABLE (RCC_ADC (x))
#define RCC_ENABLE_ADC(x)		RCC_ENABLE (RCC_ADC (x))
/// Same as RCC_ENABLE (RCC_DMA (x))
#define RCC_ENABLE_DMA(x)		RCC_ENABLE (RCC_DMA (x))
/// Same as RCC_ENABLE (RCC_I2C (x))
#define RCC_ENABLE_I2C(x)		RCC_ENABLE (RCC_I2C (x))
/// Same as RCC_ENABLE (RCC_SPI (x))
#define RCC_ENABLE_SPI(x)		RCC_ENABLE (RCC_SPI (x))
/// Same as RCC_ENABLE (RCC_TIM (x))
#define RCC_ENABLE_TIM(x)		RCC_ENABLE (RCC_TIM (x))
/// Same as RCC_ENABLE (RCC_CAN (x))
#define RCC_ENABLE_CAN(x)		RCC_ENABLE (RCC_CAN (x))

/// Same as RCC_DISABLE (RCC_GPIO (x))
#define RCC_DISABLE_GPIO(x)		RCC_DISABLE (RCC_GPIO (x))
/// Same as RCC_DISABLE (RCC_USART (x))
#define RCC_DISABLE_USART(x)		RCC_DISABLE (RCC_USART (x))
/// Same as RCC_DISABLE (RCC_ADC (x))
#define RCC_DISABLE_ADC(x)		RCC_DISABLE (RCC_ADC (x))
/// Same as RCC_DISABLE (RCC_DMA (x))
#define RCC_DISABLE_DMA(x)		RCC_DISABLE (RCC_DMA (x))
/// Same as RCC_DISABLE (RCC_I2C (x))
#define RCC_DISABLE_I2C(x)		RCC_DISABLE (RCC_I2C (x))
/// Same as RCC_DISABLE (RCC_SPI (x))
#define RCC_DISABLE_SPI(x)		RCC_DISABLE (RCC_SPI (x))
/// Same as RCC_DISABLE (RCC_TIM (x))
#define RCC_DISABLE_TIM(x)		RCC_DISABLE (RCC_TIM (x))
/// Same as RCC_DISABLE (RCC_CAN (x))
#define RCC_DISABLE_CAN(x)		RCC_DISABLE (RCC_CAN (x))

/// Same as RCC_ENABLED (RCC_GPIO (x))
#define RCC_ENABLED_GPIO(x)		RCC_ENABLED (RCC_GPIO (x))
/// Same as RCC_ENABLED (RCC_USART (x))
#define RCC_ENABLED_USART(x)		RCC_ENABLED (RCC_USART (x))
/// Same as RCC_ENABLED (RCC_ADC (x))
#define RCC_ENABLED_ADC(x)		RCC_ENABLED (RCC_ADC (x))
/// Same as RCC_ENABLED (RCC_DMA (x))
#define RCC_ENABLED_DMA(x)		RCC_ENABLED (RCC_DMA (x))
/// Same as RCC_ENABLED (RCC_I2C (x))
#define RCC_ENABLED_I2C(x)		RCC_ENABLED (RCC_I2C (x))
/// Same as RCC_ENABLED (RCC_SPI (x))
#define RCC_ENABLED_SPI(x)		RCC_ENABLED (RCC_SPI (x))
/// Same as RCC_ENABLED (RCC_TIM (x))
#define RCC_ENABLED_TIM(x)		RCC_ENABLED (RCC_TIM (x))
/// Same as RCC_ENABLED (RCC_CAN (x))
#define RCC_ENABLED_CAN(x)		RCC_ENABLED (RCC_CAN (x))

/// Same as RCC_RES (RCC_GPIO (x))
#define RCC_RES_GPIO(x)			RCC_RES (RCC_GPIO (x))
/// Same as RCC_RES (RCC_USART (x))
#define RCC_RES_USART(x)		RCC_RES (RCC_USART (x))
/// Same as RCC_RES (RCC_ADC (x))
#define RCC_RES_ADC(x)			RCC_RES (RCC_ADC (x))
/// Same as RCC_RES (RCC_DMA (x))
#define RCC_RES_DMA(x)			RCC_RES (RCC_DMA (x))
/// Same as RCC_RES (RCC_I2C (x))
#define RCC_RES_I2C(x)			RCC_RES (RCC_I2C (x))
/// Same as RCC_RES (RCC_SPI (x))
#define RCC_RES_SPI(x)			RCC_RES (RCC_SPI (x))
/// Same as RCC_RES (RCC_TIM (x))
#define RCC_RES_TIM(x)			RCC_RES (RCC_TIM (x))
/// Same as RCC_RES (RCC_CAN (x))
#define RCC_RES_CAN(x)			RCC_RES (RCC_CAN (x))

/// Same as RCC_ENA (RCC_GPIO (x))
#define RCC_ENA_GPIO(x)			RCC_ENA (RCC_GPIO (x))
/// Same as RCC_ENA (RCC_USART (x))
#define RCC_ENA_USART(x)		RCC_ENA (RCC_USART (x))
/// Same as RCC_ENA (RCC_UART (x))
#define RCC_ENA_UART(x)			RCC_ENA (RCC_UART (x))
/// Same as RCC_ENA (RCC_ADC (x))
#define RCC_ENA_ADC(x)			RCC_ENA (RCC_ADC (x))
/// Same as RCC_ENA (RCC_DMA (x))
#define RCC_ENA_DMA(x)			RCC_ENA (RCC_DMA (x))
/// Same as RCC_ENA (RCC_I2C (x))
#define RCC_ENA_I2C(x)			RCC_ENA (RCC_I2C (x))
/// Same as RCC_ENA (RCC_SPI (x))
#define RCC_ENA_SPI(x)			RCC_ENA (RCC_SPI (x))
/// Same as RCC_ENA (RCC_TIM (x))
#define RCC_ENA_TIM(x)			RCC_ENA (RCC_TIM (x))
/// Same as RCC_ENA (RCC_CAN (x))
#define RCC_ENA_CAN(x)			RCC_ENA (RCC_CAN (x))

/// Same as RCC_DIS (RCC_GPIO (x))
#define RCC_DIS_GPIO(x)			RCC_DIS (RCC_GPIO (x))
/// Same as RCC_DIS (RCC_USART (x))
#define RCC_DIS_USART(x)		RCC_DIS (RCC_USART (x))
/// Same as RCC_DIS (RCC_ADC (x))
#define RCC_DIS_ADC(x)			RCC_DIS (RCC_ADC (x))
/// Same as RCC_DIS (RCC_DMA (x))
#define RCC_DIS_DMA(x)			RCC_DIS (RCC_DMA (x))
/// Same as RCC_DIS (RCC_I2C (x))
#define RCC_DIS_I2C(x)			RCC_DIS (RCC_I2C (x))
/// Same as RCC_DIS (RCC_SPI (x))
#define RCC_DIS_SPI(x)			RCC_DIS (RCC_SPI (x))
/// Same as RCC_DIS (RCC_TIM (x))
#define RCC_DIS_TIM(x)			RCC_DIS (RCC_TIM (x))
/// Same as RCC_DIS (RCC_CAN (x))
#define RCC_DIS_CAN(x)			RCC_DIS (RCC_CAN (x))

// -------------------------- // RCC functions // --------------------------- //

typedef enum
{
    /// Unknown cause
    rstUnknown = 0,
    /// Low-power reset
    rstLowPower,
    /// Window watchdog reset
    rstWindowWatchog,
#ifdef RCC_CSR_IWDGRSTF
    /// Independent watchdog reset
    rstIndependentWatchdog,
#endif
    /// Software reset
    rstSoftware,
    /// POR/PDR reset
    rstPower,
#ifdef RCC_CSR_PINRSTF
    /// RST PIN reset
    rstPin,
#endif
#ifdef RCC_CSR_BORRSTF
    /// Brownout reset
    rstBrownout,
#endif
#ifdef RCC_CSR_OBLRSTF
    /// Option Byte Loader reset
    rstOptionByteLoader,
#endif

    // Reset causes not defined for platform are defined to -1

#ifndef RCC_CSR_IWDGRSTF
    rstIndependentWatchdog = -1,
#endif
#ifndef RCC_CSR_PINRSTF
    rstPin = -1,
#endif
#ifndef RCC_CSR_BORRSTF
    rstBrownout = -1,
#endif
#ifndef RCC_CSR_OBLRSTF
    rstOptionByteLoader = -1,
#endif
} rst_reset_cause_t;

/**
 * Clear reset flag.
 */
INLINE_ALWAYS void rcc_reset_cause_clear ()
{
    RCC->CSR |= RCC_CSR_RMVF;
}

/**
 * Return the cause of the last RESET.
 * @return
 *      One of the rstXXX values.
 */
INLINE_ALWAYS rst_reset_cause_t rcc_reset_cause ()
{
    uint32_t csr = RCC->CSR;
    rcc_reset_cause_clear ();

    return
        (csr & RCC_CSR_LPWRRSTF) ? rstLowPower :
        (csr & RCC_CSR_WWDGRSTF) ? rstWindowWatchog :
#ifdef RCC_CSR_IWDGRSTF
        (csr & RCC_CSR_IWDGRSTF) ? rstIndependentWatchdog :
#endif
        (csr & RCC_CSR_SFTRSTF) ? rstSoftware :
        (csr & RCC_CSR_PORRSTF) ? rstPower :
#ifdef RCC_CSR_PINRSTF
        (csr & RCC_CSR_PINRSTF) ? rstPin :
#endif
#ifdef RCC_CSR_BORRSTF
        (csr & RCC_CSR_BORRSTF) ? rstBrownout :
#endif
#ifdef RCC_CSR_OBLRSTF
        (csr & RCC_CSR_OBLRSTF) ? rstOptionByteLoader :
#endif
        rstUnknown;
}

#endif // _STM32_RCC_H
