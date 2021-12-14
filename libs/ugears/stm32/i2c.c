/*
    I2C controller support library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears/ugears.h"
#include <useful/fpmath.h>
#include <useful/clike.h>
#include <useful/atomic.h>

#if defined I2C_TYPE_1

#if I2C1_ENGINE
// I2C engine state for first controller
volatile i2ce_state_t i2ces1;
#endif

#if I2C2_ENGINE
// I2C engine state for second controller
volatile i2ce_state_t i2ces2;
#endif

#if I2C3_ENGINE
// I2C engine state for third controller
volatile i2ce_state_t i2ces3;
#endif

void i2c_init (I2C_TypeDef *i2c, uint32_t speed, i2c_mode_t mode, unsigned bus_freq)
{
    // If activity is going on the bus, reset the interface and hope for the best
    if (i2c->SR2 & I2C_SR2_BUSY)
    {
        i2ce_abort (i2c);

        // If bus is still busy, generate a STOP condition
        if (i2c->SR2 & I2C_SR2_BUSY)
        {
            i2c->CR1 |= I2C_CR1_STOP;
            // delay for 10 us
            delay (10E-6);
        }
    }

    // Disable I2C peripherial
    i2c->CR1 &= ~I2C_CR1_PE;

    // freq = bus_freq / 1000000
    uint32_t freq = umul_h32 (bus_freq, FxPu32 (1E-6));
    i2c->CR2 = freq;

    uint32_t ccr;
    if (speed <= 100000)
    {
        // For normal mode, high/low times are 50% of the period
        ccr = bus_freq / (speed * 2);
        if (ccr < 4)
            ccr = 4;
        // for clock <= 100 kHz rise time is 1000ns
        i2c->TRISE = freq + 1; 
    }
    else
    {
        // In fast mode, the goal is to have 9|16 high/low timings,
        // but with a relatively low clock the precision may be too low,
        // in which case we have the option to use the 1|2 (= 8|16)
        // high/low timings.
        uint32_t tmp = speed * 25;
        uint32_t ccr2 = (bus_freq + tmp / 2) / tmp;

        tmp = speed * 3;
        ccr = (bus_freq + (tmp / 2)) / tmp;

        // Goal low time is (clock/speed) * (16/25).
        // Real low time in clocks is ccr2*16 and ccr*2.
        // For better precision we won't divide by 25, instead
        // we will additionaly multiply ccr and ccr2 by 25
        tmp = (bus_freq * 16 + speed / 2) / speed;

        // Check which clock better approximates goal bus period
        if (ABS ((int32_t)(tmp - ccr2 * 16 * 25)) <= ABS ((int32_t)(tmp - ccr * 2 * 25)))
            ccr = ccr2 | I2C_CCR_FS | I2C_CCR_DUTY;
        else
            ccr |= I2C_CCR_FS;

        // for clock >= 100 kHz rise time is 300ns, e.g. multiply by 1/3
        i2c->TRISE = (((freq + 1) * 21846) >> 16) + 1;
    }

    i2c->CCR = ccr;
    i2c->CR1 = (i2c->CR1 & ~i2cmAll) | mode | I2C_CR1_PE;

#if I2C1_ENGINE
    if (i2c == I2C1)
    {
        memclr ((void *)&i2ces1, sizeof (i2ces1));
        nvic_setup (DMA_IRQ_NUM (I2C1_TX), DMA_IRQ_PRIO (I2C1_TX));
        nvic_setup (DMA_IRQ_NUM (I2C1_RX), DMA_IRQ_PRIO (I2C1_RX));
        nvic_setup (I2C1_EV_IRQn, 128);
        nvic_setup (I2C1_ER_IRQn, 128);
    }
#endif
#if I2C2_ENGINE
    if (i2c == I2C2)
    {
        memclr ((void *)&i2ces2, sizeof (i2ces2));
        nvic_setup (DMA_IRQ_NUM (I2C2_TX), DMA_IRQ_PRIO (I2C2_TX));
        nvic_setup (DMA_IRQ_NUM (I2C2_RX), DMA_IRQ_PRIO (I2C2_RX));
        nvic_setup (I2C2_EV_IRQn, 128);
        nvic_setup (I2C2_ER_IRQn, 128);
    }
#endif
}

#if I2C1_ENGINE
#  define I2C_IDX 1
#  ifndef I2C1_TX_DMA_SUFX
#    define I2C1_TX_DMA_SUFX
#  endif
#  ifndef I2C1_RX_DMA_SUFX
#    define I2C1_RX_DMA_SUFX
#  endif
#  include "i2ce.h"
#endif

#if I2C2_ENGINE
#  define I2C_IDX 2
#  ifndef I2C2_TX_DMA_SUFX
#    define I2C2_TX_DMA_SUFX
#  endif
#  ifndef I2C2_RX_DMA_SUFX
#    define I2C2_RX_DMA_SUFX
#  endif
#  include "i2ce.h"
#endif

#if I2C3_ENGINE
#  define I2C_IDX 3
#  ifndef I2C3_TX_DMA_SUFX
#    define I2C3_TX_DMA_SUFX
#  endif
#  ifndef I2C3_RX_DMA_SUFX
#    define I2C3_RX_DMA_SUFX
#  endif
#  include "i2ce.h"
#endif

#elif defined I2C_TYPE_2

// I2C support not implemented yet for STM32F0 & STM32F3

#endif // I2C_TYPE_1
