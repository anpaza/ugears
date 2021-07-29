/*
    STM32 GPIO helpers library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <ugears/ugears.h>

void gpio_setup (gpio_config_t conf)
{
    // Port number (0-A, 1-B ...)
    unsigned p = GPIO_CONF_PORT (conf);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * p);

    // GPIO bit number (0-15)
    unsigned b = GPIO_CONF_PIN (conf);

    // Set up ODR
#if defined GPIO_TYPE_3
    if (conf & _GPIO_INIT_1)
        gpio->BSRR = 1 << b;
    else
        gpio->BSRR = 16 << b;
#else
    if (conf & _GPIO_INIT_1)
        gpio->BSRR = 1 << b;
    else
        gpio->BRR = 1 << b;
#endif

#if defined GPIO_TYPE_1
    uint32_t n = (conf & (_GPIO_MODE_MASK | _GPIO_CNF_MASK)) << ((b & 7) * 4);
    uint32_t m = ~((_GPIO_MODE_MASK | _GPIO_CNF_MASK) << ((b & 7) * 4));

    // Set up GPIO config & mode
    if (b < 8)
        gpio->CRL = (gpio->CRL & m) | n;
    else
        gpio->CRH = (gpio->CRH & m) | n;

#elif defined GPIO_TYPE_2 || defined GPIO_TYPE_3
    uint32_t n = (conf & _GPIO_MODE_MASK) << (b * 2);
    uint32_t m = ~(_GPIO_MODE_MASK << (b * 2));
    gpio->MODER = (gpio->MODER & m) | n;

    gpio->OTYPER = (gpio->OTYPER & ~(1 << b)) |
        (((conf & _GPIO_OTYPE_MASK) ? 1 : 0) << b);

    n = ((conf & _GPIO_PUD_MASK) >> _GPIO_PUD_SHIFT) << (b * 2);
    m = ~((_GPIO_PUD_MASK >> _GPIO_PUD_SHIFT) << (b * 2));
    gpio->PUPDR = (gpio->PUPDR & m) | n;

    n = ((conf & _GPIO_SPEED_MASK) >> _GPIO_SPEED_SHIFT) << (b * 2);
    m = ~((_GPIO_SPEED_MASK >> _GPIO_SPEED_SHIFT) << (b * 2));
    gpio->OSPEEDR = (gpio->OSPEEDR & m) | n;

    n = conf & _GPIO_AF_MASK;
    if (n != _GPIO_AF_X)
    {
        n = (n >> _GPIO_AF_SHIFT) << ((b & 7) * 4);
        m = ~((_GPIO_AF_MASK >> _GPIO_AF_SHIFT) << ((b & 7) * 4));
        gpio->AFR [b >> 3] = (gpio->AFR [b >> 3] & m) | n;
    }
#endif
}

void gpio_setups (const gpio_config_t *conf, unsigned n)
{
    while (n--)
        gpio_setup (*conf++);
}

gpio_config_t gpio_get_setup (gpio_config_t pp)
{
    // Port number (0-A, 1-B ...)
    unsigned p = GPIO_CONF_PORT (pp);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * p);

    // GPIO bit number (0-15)
    unsigned b = GPIO_CONF_PIN (pp);

    pp &= _GPIO_PORT_MASK | _GPIO_PIN_MASK;

#if defined GPIO_TYPE_1
    uint32_t s = (b & 7) * 4;
    uint32_t m = (_GPIO_MODE_MASK | _GPIO_CNF_MASK) << s;
    if (b < 8)
        conf |= (gpio->CRL & m) >> s;
    else
        conf |= (gpio->CRH & m) >> s;

#elif defined GPIO_TYPE_2 || defined GPIO_TYPE_3
    pp |= (gpio->MODER >> (b * 2)) & _GPIO_MODE_MASK;
    if (gpio->OTYPER & (1 << b))
        pp |= _GPIO_OTYPE_MASK;
    pp |= ((gpio->PUPDR >> (b * 2)) << _GPIO_PUD_SHIFT) & _GPIO_PUD_MASK;
    pp |= ((gpio->OSPEEDR >> (b * 2)) << _GPIO_SPEED_SHIFT) & _GPIO_SPEED_MASK;
    if (pp & _GPIO_MODE_AF)
        pp |= ((gpio->AFR [b >> 3] >> ((b & 7) * 4)) << _GPIO_AF_SHIFT) & _GPIO_AF_MASK;
#endif

    return pp;
}

void gpio_set (gpio_config_t pp)
{
    // Port number (0-A, 1-B ...)
    unsigned p = GPIO_CONF_PORT (pp);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * p);

    // GPIO pin mask (1 << (0-15))
    uint32_t m = 1 << GPIO_CONF_PIN (pp);

    gpio->BSRR = m;
}

void gpio_reset (gpio_config_t pp)
{
    // Port number (0-A, 1-B ...)
    unsigned p = GPIO_CONF_PORT (pp);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * p);

    // GPIO pin mask (1 << (0-15))
    uint32_t m = 1 << GPIO_CONF_PIN (pp);

#if defined GPIO_TYPE_3
    gpio->BSRR = m << 16;
#else
    gpio->BRR = m;
#endif
}

uint32_t gpio_get (gpio_config_t pp)
{
    // Port number (0-A, 1-B ...)
    unsigned p = GPIO_CONF_PORT (pp);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * p);

    // GPIO pin mask (1 << (0-15))
    uint32_t m = 1 << GPIO_CONF_PIN (pp);

    return (gpio->IDR & m);
}

void gpio_toggle (gpio_config_t pp)
{
    // Port number (0-A, 1-B ...)
    unsigned p = GPIO_CONF_PORT (pp);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * p);

    // GPIO pin mask (1 << (0-15))
    uint32_t m = 1 << GPIO_CONF_PIN (pp);

    gpio->ODR ^= m;
}
