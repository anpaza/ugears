/*
    Analog-to-Digital Convertor library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears/ugears.h"

#ifdef ADC_TYPE_1

void adc_calibrate (ADC_TypeDef *adc)
{
    adc_enable (adc, 0);

    // Wait at least two ADC cycles so that calibration works
    delay_clocks (2 * CLOCK_ADC_DIV * PCLK2_DIV);

    adc_enable (adc, 1);

    /* Reset ADC calibartion registers */
    adc->CR2 |= ADC_CR2_RSTCAL;
    while (adc->CR2 & ADC_CR2_RSTCAL)
        ;

    /* Start calibration */
    adc->CR2 |= ADC_CR2_CAL;
    while (adc->CR2 & ADC_CR2_CAL)
        ;
}

void adc_regseq_chan (ADC_TypeDef *adc, uint32_t idx, uint32_t chan)
{
    uint32_t shift;
    if (idx < 6)
    {
        shift = idx * 5;
        idx = 2;
    }
    else if (idx < 12)
    {
        shift = (idx - 6) * 5;
        idx = 1;
    }
    else if (idx < 16)
    {
        shift = (idx - 12) * 5;
        idx = 0;
    }
    else
        return;

    volatile uint32_t *sqr = &adc->SQR1;
    sqr [idx] = (sqr [idx] & ~(31 << shift)) | (chan << shift);
}

#endif // ADC_TYPE_1
