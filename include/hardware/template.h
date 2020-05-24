/*
    Short description of your board here

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _HARDWARE_H
#define _HARDWARE_H

/**
 * @file hardware.h
 *      This is just a template for your board-specific header file.
 *      It is supposed to include the MCU-specific header files, and also
 *      declare the on-board hardware connections. It should be named by
 *      your HARDWARE (defined in local-config.mak) with the .h extension.
 *      We will refer it later as HARDWARE_H.
 *
 * A hardware definition is supposed to declare all the ports and other
 * resources used (timers, DMAs and so on). There are a lot of useful
 * macros that can be later used to conveniently set up and access every
 * hardware feature.
 *
 * A "hardware feature" is supposed to be the minimal atomic hardware
 * unit - a LED, a button, a sensor and so on. A hardware feature is
 * accessed through some MCU peripherial - GPIO pin, I2C,SPI,USB,...
 * bus and so on.
 *
 * The respective header files (gpio.h, i2c.h, spi.h, ...) contain
 * specific information on the macros that you may define in order
 * to access your hardware feature via the respective peripherial.
 *
 * For example, let's define and light up a bulb connected to port PA12.
 * For this, add to your HARDWARE_H:
 * @code
 * #define LED_PORT		A
 * #define LED_PIN		12
 * #define LED_GPIO_CONFIG	OUTPUT_2MHz,PUSHPULL,0
 * @endcode
 *
 * Now you can use those definitions in code, like this:
 * @code
 * RCC_ENA_GPIO (LED);
 * GPIO_SETUP (LED);
 * GPIO_SET (LED);
 * GPIO_RESET (LED);
 * GPIO_TOGGLE (LED);
 * @endcode
 *
 * Every macro will append a specific suffix to hardware feature you provide
 * (e.g. LED) and pick up the respective definition. For example, when you
 * invoke GPIO_SETUP (LED) the preprocessor will append "_GPIO_CONFIG" to
 * the "LED" you provided to get the LED_GPIO_CONFIG token, and then it will
 * expand it to the actual value you provided in HARDWARE_H. Then it will
 * again append "_PORT" and "_PIN" to "LED" you provided to find out which
 * GPIO peripherial registers should be modified to set up the GPIO pin in
 * the respective configuration.
 *
 * Another important thing you define here is the MCU clock configuration.
 * STM32 MCUs can have very sophisticated clock configurations, so you will
 * have to consult the respective clocks-stm32f*.h file for a short
 * description of the macros you may define to get what you need.
 *
 * The clocks always have various limitations, so you will have to consult
 * either the reference manual (the RCC section usually contains a very
 * helpful "Clock tree" diagram), or use a tool like STM32CubeMX to figure
 * out the clock values and then enter them here.
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

#endif /* _HARDWARE_H */
