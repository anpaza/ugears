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
    if (conf & GPIO_INIT_1)
        gpio->BSRR = 1 << b;
    else
        gpio->BSRR = 16 << b;
#else
    if (conf & GPIO_INIT_1)
        gpio->BSRR = 1 << b;
    else
        gpio->BRR = 1 << b;
#endif

#if defined GPIO_TYPE_1
    uint32_t n = (conf & (GPIO_MODE_MASK | GPIO_CNF_MASK)) << ((b & 7) * 4);
    uint32_t m = ~((GPIO_MODE_MASK | GPIO_CNF_MASK) << ((b & 7) * 4));

    // Set up GPIO config & mode
    if (b < 8)
        gpio->CRL = (gpio->CRL & m) | n;
    else
        gpio->CRH = (gpio->CRH & m) | n;

#elif defined GPIO_TYPE_2 || defined GPIO_TYPE_3
    uint32_t n = (conf & GPIO_MODE_MASK) << (b * 2);
    uint32_t m = ~(GPIO_MODE_MASK << (b * 2));
    gpio->MODER = (gpio->MODER & m) | n;

    gpio->OTYPER = (gpio->OTYPER & ~(1 << b)) |
        (((conf & GPIO_OTYPE_MASK) ? 1 : 0) << b);

    n = ((conf & GPIO_PUD_MASK) >> GPIO_PUD_SHIFT) << (b * 2);
    m = ~((GPIO_PUD_MASK >> GPIO_PUD_SHIFT) << (b * 2));
    gpio->PUPDR = (gpio->PUPDR & m) | n;

    n = ((conf & GPIO_SPEED_MASK) >> GPIO_SPEED_SHIFT) << (b * 2);
    m = ~((GPIO_SPEED_MASK >> GPIO_SPEED_SHIFT) << (b * 2));
    gpio->OSPEEDR = (gpio->OSPEEDR & m) | n;

    n = conf & GPIO_AF_MASK;
    if (n != GPIO_AF_X)
    {
        n = (n >> GPIO_AF_SHIFT) << ((b & 7) * 4);
        m = ~((GPIO_AF_MASK >> GPIO_AF_SHIFT) << ((b & 7) * 4));
        gpio->AFR [b >> 3] = (gpio->AFR [b >> 3] & m) | n;
    }
#endif
}

void gpio_setups (const gpio_config_t *conf, unsigned n)
{
    while (n--)
        gpio_setup (*conf++);
}

gpio_config_t gpio_get_setup (gpio_config_t conf)
{
    // Port number (0-A, 1-B ...)
    unsigned p = GPIO_CONF_PORT (conf);
    GPIO_TypeDef *gpio = (GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * p);

    // GPIO bit number (0-15)
    unsigned b = GPIO_CONF_PIN (conf);

    conf &= GPIO_PORT_MASK | GPIO_PIN_MASK;

#if defined GPIO_TYPE_1
    uint32_t s = (b & 7) * 4;
    uint32_t m = (GPIO_MODE_MASK | GPIO_CNF_MASK) << s;
    if (b < 8)
        conf |= (gpio->CRL & m) >> s;
    else
        conf |= (gpio->CRH & m) >> s;

#elif defined GPIO_TYPE_2 || defined GPIO_TYPE_3
    conf |= (gpio->MODER >> (b * 2)) & GPIO_MODE_MASK;
    if (gpio->OTYPER & (1 << b))
        conf |= GPIO_OTYPE_MASK;
    conf |= ((gpio->PUPDR >> (b * 2)) << GPIO_PUD_SHIFT) & GPIO_PUD_MASK;
    conf |= ((gpio->OSPEEDR >> (b * 2)) << GPIO_SPEED_SHIFT) & GPIO_SPEED_MASK;
    if (conf & GPIO_MODE_AF)
        conf |= ((gpio->AFR [b >> 3] >> ((b & 7) * 4)) << GPIO_AF_SHIFT) & GPIO_AF_MASK;
#endif

    return conf;
}
