/*
    STM32 GPIO helpers library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _GPIO_H
#define _GPIO_H

/**
 * @file gpio.h
 *      Functions and macros to deal with GPIO ports.
 */

#include "useful.h"

// There are at least three different types of GPIO peripherial
#if defined STM32F1
#  define GPIO_TYPE_1
#elif defined STM32F0 || defined STM32F3
#  define GPIO_TYPE_2
#elif defined STM32F2 || defined STM32F4
#  define GPIO_TYPE_3
#else
#  error "Please define the correct GPIO type for your MCU"
#endif

/// Return the port name (A, B, C etc) given hardware feature name
#define GPIO_PORT(x)		JOIN2 (x, _PORT)
/// Return the bit number (0, 1, 2 etc) given hardware feature name
#define GPIO_BIT(x)		JOIN2 (x, _BIT)
/// Return the bit value of a port bit by name
#define GPIO_BITV(x)		BV (GPIO_BIT (x))

// Auxiliary macros (not meant to be directly used by user)
#define GPIO_PORT_MASK		0xf000
#define GPIO_PORT_SHIFT		12
#define GPIO_PORT_A		0x0000
#define GPIO_PORT_B		0x1000
#define GPIO_PORT_C		0x2000
#define GPIO_PORT_D		0x3000
#define GPIO_PORT_E		0x4000
#define GPIO_PORT_F		0x5000
#define GPIO_PORT_G		0x6000
#define GPIO_PORT_H		0x7000
#define GPIO_PORT_I		0x8000
#define GPIO_PORT_J		0x9000
#define GPIO_PORT_K		0xA000

#define GPIO_BIT_MASK		0x0F00
#define GPIO_BIT_SHIFT		8

/// Preprocessor: is hw feature x port equal to port name p (A, B, ...)?
#define PORT_EQ(x, p)		(JOIN2 (GPIO_PORT_, GPIO_PORT (x)) == JOIN2 (GPIO_PORT_, p))
/// Preprocessor: is port and bit of hw feature x equal to port p (A, B, ...) and bit b?
#define PORTBIT_EQ(x, p, b)	(_PORT_EQ (x, p) && (GPIO_BIT (x) == b))
/// Preprocessor: is port of hw feature x equal to port of hw feature y?
#define PORTS_EQ(x, y)		(JOIN2 (GPIO_PORT_, GPIO_PORT(x)) == JOIN2 (GPIO_PORT_, GPIO_PORT(y)))

/// Return the GPIO port (GPIOA, GPIOB etc) given feature name
#define GPIO(x)			JOIN2 (GPIO, GPIO_PORT (x))
/// Extract the port number (0-10, corresponding to A-K) from a gpio_config_t
#define GPIO_CONF_PORT(c)	((c & GPIO_PORT_MASK) >> GPIO_PORT_SHIFT)
/// Extract the bit number (0-15) from a gpio_config_t
#define GPIO_CONF_BIT(c)	((c & GPIO_BIT_MASK) >> GPIO_BIT_SHIFT)

// -------------------- // GPIO configuration bits // -------------------- //

// Port initial state (output) or pullup/pulldown (if input with pud)

// Don't care (equivalent to 0)
#define GPIO_INIT_X		0x0000
// output a '0'
#define GPIO_INIT_0		0x0000
// output a '1'
#define GPIO_INIT_1		0x0080

#if defined GPIO_TYPE_1

typedef uint16_t gpio_config_t;

// GPIO mode

/// Input mode (reset state)
#define GPIO_MODE_INPUT		0x0000
/// Output mode, max speed 10 MHz
#define GPIO_MODE_OUTPUT_10MHz	0x0001
/// Output mode, max speed 2 MHz
#define GPIO_MODE_OUTPUT_2MHz	0x0002
/// Output mode, max speed 50 MHz
#define GPIO_MODE_OUTPUT_50MHz	0x0003
/// Just the bitmask for all bits
#define GPIO_MODE_MASK		0x0003

// In input mode (MODE[1:0]=00)

/// Analog mode
#define GPIO_CNF_ANALOG		0x0000
/// Floating input (reset state)
#define GPIO_CNF_FLOATING	0x0004
/// Input with pull-up / pull-down (ODR sets 0=pulldown, 1=pullup)
#define GPIO_CNF_PUD		0x0008

// enable pull-down input
#define GPIO_INIT_PD		GPIO_INIT_0
// enable pull-up input
#define GPIO_INIT_PU		GPIO_INIT_1

// In output mode (MODE[1:0] > 00)

/// General purpose output push-pull
#define GPIO_CNF_PUSHPULL	0x0000
/// General purpose output Open-drain
#define GPIO_CNF_OPENDRAIN	0x0004
/// Alternate function output Push-pull
#define GPIO_CNF_AF_PUSHPULL	0x0008
/// Alternate function output Open-drain
#define GPIO_CNF_AF_OPENDRAIN	0x000C

/// Just the bitmask for all bits
#define GPIO_CNF_MASK		0x000C

/**
 * Define the configuration of a single GPIO port.
 * This applies to STM32F1xx MCU series.
 *
 * @arg x hw feature name
 * @arg mod GPIO mode (without the GPIO_MODE_ prefix):
 *      INPUT, OUTPUT_2MHz, OUTPUT_10MHz, OUTPUT_50MHz
 * @arg cnf Port configuration (without GPIO_CNF_ prefix):
 *      PUSHPULL, OPENDRAIN, AF_PUSHPULL, AF_OPENDRAIN
 * @arg ini Initial output state: 0, 1 or X
 */
#define GPIO_CONFIG(x,mod,cnf,ini) (\
	JOIN2 (GPIO_PORT_, GPIO_PORT(x)) | \
	(GPIO_BIT (x) << GPIO_BIT_SHIFT) | \
	JOIN2 (GPIO_MODE_, mod) | \
	JOIN2 (GPIO_CNF_, cnf) | \
	JOIN2 (GPIO_INIT_, ini) \
)

#elif defined GPIO_TYPE_2 || defined GPIO_TYPE_3

typedef uint32_t gpio_config_t;

/// Input mode
#define GPIO_MODE_INPUT		0x00000000
/// Push-Pull output mode
#define GPIO_MODE_PUSHPULL	0x00000001
/// Open-Drain output mode
#define GPIO_MODE_OPENDRAIN	0x00000005
/// Alternate function mode
#define GPIO_MODE_AF		0x00000002
/// Alternate function push-pull mode
#define GPIO_MODE_AF_PUSHPULL	0x00000002
/// Alternate function open-drain mode
#define GPIO_MODE_AF_OPENDRAIN	0x00000006
/// Analog mode
#define GPIO_MODE_ANALOG	0x00000003
/// The mask to select the bits for MODER
#define GPIO_MODE_MASK		0x00000003
/// Test if user wants push-pull or open-drain
#define GPIO_OTYPE_MASK		0x00000004

/// No pull-up or pull-down
#define GPIO_PUD_X		0x00000000
/// Enable pull-up resistor
#define GPIO_PUD_UP		0x00000008
/// Enable pull-down resistor
#define GPIO_PUD_DOWN		0x00000010
/// The mask for all pull-up-down bits
#define GPIO_PUD_MASK		0x00000018
/// Amount to shift PUD to right to get the value for PUPDR
#define GPIO_PUD_SHIFT		3

/// Speed don't care (for inputs)
#define GPIO_SPEED_X		0x00000000
/// Output low-speed (2MHz) mode
#define GPIO_SPEED_LOW		0x00000000
#define GPIO_SPEED_2MHz		0x00000000
/// Output medium-speed (10MHz) mode
#define GPIO_SPEED_MEDIUM	0x00000020
#define GPIO_SPEED_10MHz	0x00000020
/// Output high-speed (50MHz) mode
#define GPIO_SPEED_HIGH		0x00000060
#define GPIO_SPEED_50MHz	0x00000060
/// The mask for output speed bits
#define GPIO_SPEED_MASK		0x00000060
/// Amount to shift speed bits to right to get the value for OSPEEDR
#define GPIO_SPEED_SHIFT	5

/// Alternate Function 0 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_0		0x00000000
/// Alternate Function 1 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_1		0x00010000
/// Alternate Function 2 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_2		0x00020000
/// Alternate Function 3 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_3		0x00030000
/// Alternate Function 4 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_4		0x00040000
/// Alternate Function 5 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_5		0x00050000
/// Alternate Function 6 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_6		0x00060000
/// Alternate Function 7 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_7		0x00070000
/// Alternate Function 8 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_8		0x00080000
/// Alternate Function 9 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_9		0x00090000
/// Alternate Function 10 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_10		0x000A0000
/// Alternate Function 11 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_11		0x000B0000
/// Alternate Function 12 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_12		0x000C0000
/// Alternate Function 13 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_13		0x000D0000
/// Alternate Function 14 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_14		0x000E0000
/// Alternate Function 15 on pin (use together with GPIO_MODE_AF*)
#define GPIO_AF_15		0x000F0000
/// Alternative function "don't care"
#define GPIO_AF_X		GPIO_AF_14
/// The mask to isolate all the bits making the AF value
#define GPIO_AF_MASK		0x000F0000
/// The amount to shift the AF bits to the right
#define GPIO_AF_SHIFT		16

/**
 * Define the configuration of a single GPIO port.
 * This applies to STM32F0, STM32F2, STM32F3, STM32F4 series.
 *
 * @arg x hw feature name
 * @arg mod GPIO mode (without the GPIO_MODE_ prefix):
 *      INPUT, PUSHPULL, OPENDRAIN, AF, AF_PUSHPULL, AF_OPENDRAIN, ANALOG
 * @arg pud Pull up/down resistor configuration (without GPIO_PUD_ prefix):
 *      X, UP, DOWN
 * @arg spd Port speed configuration (without GPIO_SPEED_ prefix):
 *      X, LOW (2MHz), MEDIUM (10MHz), HIGH (50MHz)
 * @arg af Alternate function number, 0-15 or X (without GPIO_AF_ prefix)
 * @arg ini Initial output state (0, 1 or X)
 */
#define GPIO_CONFIG(x,mod,pud,spd,af,ini) (\
	JOIN2 (GPIO_PORT_, GPIO_PORT(x)) | \
	(GPIO_BIT (x) << GPIO_BIT_SHIFT) | \
	JOIN2 (GPIO_MODE_, mod) | \
	JOIN2 (GPIO_PUD_, pud) | \
	JOIN2 (GPIO_SPEED_, spd) | \
	JOIN2 (GPIO_AF_, af) | \
	JOIN2 (GPIO_INIT_, ini) \
)

#endif

// Intermediate wrapper to expand multiple args passed as one arg
#define __GPIO_CONFIG(p,...)	GPIO_CONFIG(p, __VA_ARGS__)

/**
 * A shorter version of GPIO_CONFIG which relays on X_GPIO_CONFIG
 * user-defined macro (usually in hardware*.h) to define all the
 * port config details
 */
#define GPIO_CONF(p)		__GPIO_CONFIG (p, JOIN2 (p, _GPIO_CONFIG))

/// Similar to GPIO_CONF(), but encodes only port name & bit number
#define GPIO_CONF_PB(p)		__GPIO_CONFIG (p)

#if defined GPIO_TYPE_3

/// Atomic set of a single bit in port
#define GPIO_SET(x)		(GPIO(x)->BSRR = GPIO_BITV (x))
/// Atomic clear of a single bit in port
#define GPIO_RESET(x)		(GPIO(x)->BSRR = (GPIO_BITV (x) << 16))

#else

/// Atomic set of a single bit in port
#define GPIO_SET(x)		(GPIO(x)->BSRR = GPIO_BITV (x))
/// Atomic clear of a single bit in port
#define GPIO_RESET(x)		(GPIO(x)->BRR = GPIO_BITV (x))

#endif

/// Get the state of the bit in port
#define GPIO_GET(x)		(GPIO(x)->IDR & GPIO_BITV (x))
/// Toggle the state of an output GPIO
#define GPIO_TOGGLE(x)		(GPIO (x)->ODR ^= GPIO_BITV (x))

/// Wrapper around gpio_setup() that use hardware feature name as argument
#define GPIO_SETUP(x)		gpio_setup (GPIO_CONF (x))
/// Wrapper around gpio_get_setup() that use hardware feature name as argument
#define GPIO_GET_SETUP(x)	gpio_get_setup (GPIO_CONF_PB (x))

/**
 * Setup a GPIO port.
 * @arg conf
 *      GPIO port configuration (use GPIO_CONFIG() to create a bitmask)
 */
extern void gpio_setup (gpio_config_t conf);

/**
 * Query the current setup of a GPIO port.
 * @arg conf
 *      GPIO port name and bit index. Other bits in value are ignored.
 *      You can use GPIO_CONF_PB() to encode port & bit number.
 * @return
 *      The full configuration of given GPIO port.
 */
extern gpio_config_t gpio_get_setup (gpio_config_t conf);

/**
 * Setup a number of GPIO ports at once.
 * @arg conf
 *      A pointer to an array of GPIO port configurations
 *      (use GPIO_CONFIG() to create the bitmasks)
 * @arg n
 *      Number of elements in the conf array
 */
extern void gpio_setups (const gpio_config_t *conf, unsigned n);

#ifdef AFIO_EVCR_PIN

/**
 * Set EVENTOUT pin and state.
 * @arg conf
 *      If -1, EVENTOUT pin is disabled. Otherwise, it is a combination of
 *      port & pin numbers (generated with GPIO_CONF_PB() macro) where the
 *      Cortex EVENT output is routed.
 */
static inline void gpio_eventout (int conf)
{
    uint32_t evcr = AFIO->EVCR;

    evcr &= ~(AFIO_EVCR_EVOE | AFIO_EVCR_PIN | AFIO_EVCR_PORT);
    if (conf >= 0)
        evcr |= AFIO_EVCR_EVOE |
            (((conf & GPIO_BIT_MASK) >> GPIO_BIT_SHIFT) << AFIO_EVCR_PIN_Pos) |
            (((conf & GPIO_PORT_MASK) >> GPIO_PORT_SHIFT) << AFIO_EVCR_PORT_Pos);

    AFIO->EVCR = evcr;
}

#endif // STM32F1

#endif // _GPIO_H
