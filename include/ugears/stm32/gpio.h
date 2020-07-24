/*
    STM32 GPIO helpers library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_GPIO_H
#define _STM32_GPIO_H

/**
 * @file gpio.h
 *      Functions and macros to deal with GPIO ports.
 *
 * The following macros are expected to be defined in your HARDWARE_H
 * in order to deal with GPIO ports:
 *
 * @li HWFN_PORT defines the port name to which the feature is
 *      connected (A, B, C etc). Used by GPIO_PORT(HWFN) macro.
 * @li HWFN_PIN defines the bit number to which the feature is
 *      connected (0, 1, 2 ...). Used by GPIO_PIN(HWFN) macro.
 * @li HWFN_GPIO_CONFIG defines the parameters passed to the
 *      GPIO_SETUP macro which returns a bitmask for GPIO
 *      setup (ex: OUTPUT_2MHz,OPENDRAIN,1). The format of this
 *      string differs from MCU to MCU, to find out the correct
 *      format see the comment to GPIO_CONFIG() for your MCU series.
 *
 * Example for STM32F1 MCU series:
 * @code
 * // Example debug LED on PC13
 * #define LED_PORT		C
 * #define LED_PIN		13
 * #define LED_GPIO_CONFIG	OUTPUT_2MHz,PUSHPULL,0
 * @endcode
 */

#include <useful/useful.h>

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
/// Return the pin number (0, 1, 2 etc) given hardware feature name
#define GPIO_PIN(x)		JOIN2 (x, _PIN)
/// Return the pin mask by hardware feature name
#define GPIO_PINM(x)		BV (GPIO_PIN (x))

// Auxiliary macros (not meant to be directly used by user)
#define _GPIO_PORT_MASK		0xf000
#define _GPIO_PORT_SHIFT	12
#define _GPIO_PORT_A		0x0000
#define _GPIO_PORT_B		0x1000
#define _GPIO_PORT_C		0x2000
#define _GPIO_PORT_D		0x3000
#define _GPIO_PORT_E		0x4000
#define _GPIO_PORT_F		0x5000
#define _GPIO_PORT_G		0x6000
#define _GPIO_PORT_H		0x7000
#define _GPIO_PORT_I		0x8000
#define _GPIO_PORT_J		0x9000
#define _GPIO_PORT_K		0xA000

#define _GPIO_PIN_MASK		0x0F00
#define _GPIO_PIN_SHIFT		8

/// Preprocessor: is hw feature x port equal to port name p (A, B, ...)?
#define PORT_EQ(x, p)		(JOIN2 (_GPIO_PORT_, GPIO_PORT (x)) == JOIN2 (_GPIO_PORT_, p))
/// Preprocessor: is port and pin of hw feature x equal to port p (A, B, ...) and pin b?
#define PORTPIN_EQ(x, p, b)	(PORT_EQ (x, p) && (GPIO_PIN (x) == b))
/// Preprocessor: is port of hw feature x equal to port of hw feature y?
#define PORTS_EQ(x, y)		(JOIN2 (_GPIO_PORT_, GPIO_PORT(x)) == JOIN2 (_GPIO_PORT_, GPIO_PORT(y)))

/// Return the GPIO port (GPIOA, GPIOB etc) given feature name
#define GPIO(x)			JOIN2 (GPIO, GPIO_PORT (x))
/// Extract the port number (0-10, corresponding to A-K) from a gpio_config_t
#define GPIO_CONF_PORT(c)	((c & _GPIO_PORT_MASK) >> _GPIO_PORT_SHIFT)
/// Extract the pin number (0-15) from a gpio_config_t
#define GPIO_CONF_PIN(c)	((c & _GPIO_PIN_MASK) >> _GPIO_PIN_SHIFT)

// -------------------- // GPIO configuration bits // -------------------- //

// Port initial state (output) or pullup/pulldown (if input with pud)

// Don't care (equivalent to 0)
#define _GPIO_INIT_X		0x0000
// output a '0'
#define _GPIO_INIT_0		0x0000
// output a '1'
#define _GPIO_INIT_1		0x0080

#if defined GPIO_TYPE_1

typedef uint16_t gpio_config_t;

// GPIO mode

/// Input mode (reset state)
#define _GPIO_MODE_INPUT	0x0000
/// Output mode, max speed 10 MHz
#define _GPIO_MODE_OUTPUT_10MHz	0x0001
/// Output mode, max speed 2 MHz
#define _GPIO_MODE_OUTPUT_2MHz	0x0002
/// Output mode, max speed 50 MHz
#define _GPIO_MODE_OUTPUT_50MHz	0x0003
/// Just the bitmask for all bits
#define _GPIO_MODE_MASK		0x0003

// In input mode (MODE[1:0]=00)

/// Analog mode
#define _GPIO_CNF_ANALOG	0x0000
/// Floating input (reset state)
#define _GPIO_CNF_FLOATING	0x0004
/// Input with pull-up / pull-down (ODR sets 0=pulldown, 1=pullup)
#define _GPIO_CNF_PUD		0x0008

// enable pull-down input
#define _GPIO_INIT_PD		_GPIO_INIT_0
// enable pull-up input
#define _GPIO_INIT_PU		_GPIO_INIT_1

// In output mode (MODE[1:0] > 00)

/// General purpose output push-pull
#define _GPIO_CNF_PUSHPULL	0x0000
/// General purpose output Open-drain
#define _GPIO_CNF_OPENDRAIN	0x0004
/// Alternate function output Push-pull
#define _GPIO_CNF_AF_PUSHPULL	0x0008
/// Alternate function output Open-drain
#define _GPIO_CNF_AF_OPENDRAIN	0x000C

/// Just the bitmask for all bits
#define _GPIO_CNF_MASK		0x000C

/**
 * Define the configuration of a single GPIO port.
 * This applies to STM32F1xx MCU series.
 *
 * @arg x hw feature name
 * @arg mod GPIO mode (without the _GPIO_MODE_ prefix):
 *      INPUT, OUTPUT_2MHz, OUTPUT_10MHz, OUTPUT_50MHz
 * @arg cnf Port configuration (without _GPIO_CNF_ prefix):
 *      ANALOG, FLOATING, PUD (in INPUT mode),
 *      PUSHPULL, OPENDRAIN, AF_PUSHPULL, AF_OPENDRAIN (in OUTPUT_XXX modes)
 * @arg ini Initial pin state (without the _GPIO_INIT_ prefix):
 *      X, PU, PD in INPUT mode,
 *      0, 1 or X in OUTPUT_XXX mode
 */
#define GPIO_CONFIG(x,mod,cnf,ini) (\
	JOIN2 (_GPIO_PORT_, GPIO_PORT(x)) | \
	(GPIO_PIN (x) << _GPIO_PIN_SHIFT) | \
	JOIN2 (_GPIO_MODE_, mod) | \
	JOIN2 (_GPIO_CNF_, cnf) | \
	JOIN2 (_GPIO_INIT_, ini) \
)

#elif defined GPIO_TYPE_2 || defined GPIO_TYPE_3

typedef uint32_t gpio_config_t;

/// Input mode
#define _GPIO_MODE_INPUT	0x00000000
/// Push-Pull output mode
#define _GPIO_MODE_PUSHPULL	0x00000001
/// Open-Drain output mode
#define _GPIO_MODE_OPENDRAIN	0x00000005
/// Alternate function mode
#define _GPIO_MODE_AF		0x00000002
/// Alternate function push-pull mode
#define _GPIO_MODE_AF_PUSHPULL	0x00000002
/// Alternate function open-drain mode
#define _GPIO_MODE_AF_OPENDRAIN	0x00000006
/// Analog mode
#define _GPIO_MODE_ANALOG	0x00000003
/// The mask to select the bits for MODER
#define _GPIO_MODE_MASK		0x00000003
/// Test if user wants push-pull or open-drain
#define _GPIO_OTYPE_MASK	0x00000004

/// No pull-up or pull-down
#define _GPIO_PUD_X		0x00000000
/// Enable pull-up resistor
#define _GPIO_PUD_UP		0x00000008
/// Enable pull-down resistor
#define _GPIO_PUD_DOWN		0x00000010
/// The mask for all pull-up-down bits
#define _GPIO_PUD_MASK		0x00000018
/// Amount to shift PUD to right to get the value for PUPDR
#define _GPIO_PUD_SHIFT		3

/// Speed don't care (for inputs)
#define _GPIO_SPEED_X		0x00000000
/// Output low-speed (2MHz) mode
#define _GPIO_SPEED_LOW		0x00000000
#define _GPIO_SPEED_2MHz	0x00000000
/// Output medium-speed (10MHz) mode
#define _GPIO_SPEED_MEDIUM	0x00000020
#define _GPIO_SPEED_10MHz	0x00000020
/// Output high-speed (50MHz) mode
#define _GPIO_SPEED_HIGH	0x00000060
#define _GPIO_SPEED_50MHz	0x00000060
/// The mask for output speed bits
#define _GPIO_SPEED_MASK	0x00000060
/// Amount to shift speed bits to right to get the value for OSPEEDR
#define _GPIO_SPEED_SHIFT	5

/// Alternate Function 0 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_0		0x00000000
/// Alternate Function 1 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_1		0x00010000
/// Alternate Function 2 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_2		0x00020000
/// Alternate Function 3 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_3		0x00030000
/// Alternate Function 4 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_4		0x00040000
/// Alternate Function 5 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_5		0x00050000
/// Alternate Function 6 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_6		0x00060000
/// Alternate Function 7 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_7		0x00070000
/// Alternate Function 8 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_8		0x00080000
/// Alternate Function 9 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_9		0x00090000
/// Alternate Function 10 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_10		0x000A0000
/// Alternate Function 11 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_11		0x000B0000
/// Alternate Function 12 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_12		0x000C0000
/// Alternate Function 13 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_13		0x000D0000
/// Alternate Function 14 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_14		0x000E0000
/// Alternate Function 15 on pin (use together with GPIO_MODE_AF*)
#define _GPIO_AF_15		0x000F0000
/// Alternative function "don't care"
#define _GPIO_AF_X		_GPIO_AF_14
/// The mask to isolate all the bits making the AF value
#define _GPIO_AF_MASK		0x000F0000
/// The amount to shift the AF bits to the right
#define _GPIO_AF_SHIFT		16

/**
 * Define the configuration of a single GPIO port.
 * This applies to STM32F0, STM32F2, STM32F3, STM32F4 series.
 *
 * @arg x hw feature name
 * @arg mod GPIO mode (without the _GPIO_MODE_ prefix):
 *      INPUT, PUSHPULL, OPENDRAIN, AF, AF_PUSHPULL, AF_OPENDRAIN, ANALOG
 * @arg pud Pull up/down resistor configuration (without _GPIO_PUD_ prefix):
 *      X, UP, DOWN
 * @arg spd Port speed configuration (without _GPIO_SPEED_ prefix):
 *      X, LOW (2MHz), MEDIUM (10MHz), HIGH (50MHz)
 * @arg af Alternate function number, 0-15 or X (without _GPIO_AF_ prefix)
 * @arg ini Initial pin state (without the _GPIO_INIT_ prefix):
 *      X, PU, PD in INPUT mode,
 *      0, 1 or X in OUTPUT mode
 */
#define GPIO_CONFIG(x,mod,pud,spd,af,ini) (\
	JOIN2 (_GPIO_PORT_, GPIO_PORT(x)) | \
	(GPIO_PIN (x) << _GPIO_PIN_SHIFT) | \
	JOIN2 (_GPIO_MODE_, mod) | \
	JOIN2 (_GPIO_PUD_, pud) | \
	JOIN2 (_GPIO_SPEED_, spd) | \
	JOIN2 (_GPIO_AF_, af) | \
	JOIN2 (_GPIO_INIT_, ini) \
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

/// Similar to GPIO_CONF(), but encodes only port name & pin number
#define GPIO_CONF_PP(p)		__GPIO_CONFIG (p)

#if defined GPIO_TYPE_3

/// Atomic set of a single bit in port
#define GPIO_SET(x)		(GPIO(x)->BSRR = GPIO_PINM (x))
/// Atomic clear of a single bit in port
#define GPIO_RESET(x)		(GPIO(x)->BSRR = (GPIO_PINM (x) << 16))

#else

/// Atomic set of a single bit in port
#define GPIO_SET(x)		(GPIO(x)->BSRR = GPIO_PINM (x))
/// Atomic clear of a single bit in port
#define GPIO_RESET(x)		(GPIO(x)->BRR = GPIO_PINM (x))

#endif

/// Get the state of the bit in port
#define GPIO_GET(x)		(GPIO(x)->IDR & GPIO_PINM (x))
/// Toggle the state of an output GPIO
#define GPIO_TOGGLE(x)		(GPIO (x)->ODR ^= GPIO_PINM (x))

/// Wrapper around gpio_setup() that use hardware feature name as argument
#define GPIO_SETUP(x)		gpio_setup (GPIO_CONF (x))
/// Wrapper around gpio_get_setup() that use hardware feature name as argument
#define GPIO_GET_SETUP(x)	gpio_get_setup (GPIO_CONF_PP (x))

/**
 * Setup a GPIO port.
 * @arg conf
 *      GPIO port configuration (use GPIO_CONFIG() to create a bitmask)
 */
extern void gpio_setup (gpio_config_t conf);

/**
 * Query the current setup of a GPIO port.
 * @arg conf
 *      GPIO port name and pin index. Other bits in value are ignored.
 *      You can use GPIO_CONF_PP() to encode port & pin number.
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
 *      port & pin numbers (generated with GPIO_CONF_PP() macro) where the
 *      Cortex EVENT output is routed.
 */
INLINE_ALWAYS void gpio_eventout (int conf)
{
    uint32_t evcr = AFIO->EVCR;

    evcr &= ~(AFIO_EVCR_EVOE | AFIO_EVCR_PIN | AFIO_EVCR_PORT);
    if (conf >= 0)
        evcr |= AFIO_EVCR_EVOE |
            (((conf & _GPIO_PIN_MASK) >> _GPIO_PIN_SHIFT) << AFIO_EVCR_PIN_Pos) |
            (((conf & _GPIO_PORT_MASK) >> _GPIO_PORT_SHIFT) << AFIO_EVCR_PORT_Pos);

    AFIO->EVCR = evcr;
}

#endif // STM32F1

#endif // _STM32_GPIO_H
