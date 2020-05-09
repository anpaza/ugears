/*
    This is not a header file, but a flexible piece of code.
    Prior to including this file a number of macros should/can be defined,
    after leaving the file all these macros are undefined.

    _DMA_NUM - the DMA controller number. If not defined, generates generic code
        which accepts a pointer to DMA controller as 1st parameter.
*/

void
#ifdef _DMA_NUM
    JOIN3 (dma, _DMA_NUM, _copy) (
#define _dma	JOIN2 (DMA, _DMA_NUM)
#else
    dma_copy (DMA_TypeDef *_dma,
#endif
        unsigned chan, uint32_t ccr, volatile void *src, volatile void *dst, unsigned count)
{
    // Unfortunately, these checks won't work in preprocessor because
    // PERIPH_BASE is not just a const like it is supposed to be.
    // So we'll use a C expression which will blank out the entire
    // function if the check fails, and optimize out completely otherwise...
    if (STM32_FLASH_ORIGIN >= PERIPH_BASE)
        // DMA library expects FLASH memory address < peripherial address
        return;
    if (STM32_RAM_ORIGIN >= PERIPH_BASE)
        // "DMA library expects RAM address < peripherial address"
        return;

    // user counts channels from 1, we count from 0
    chan--;

    DMA_Channel_TypeDef *dmac = (DMA_Channel_TypeDef *)((uint8_t *)_dma +
        (DMA1_Channel1_BASE - DMA1_BASE) +
        (DMA1_Channel2_BASE - DMA1_Channel1_BASE) * chan);

    // -- Deduce the missing bits from ccr -- //

    // Memory address is always incremented, no matter of transfer direction
    ccr |= DMA_CCR_MINC;

    if ((uint32_t)src < PERIPH_BASE)
    {
        if ((uint32_t)dst < PERIPH_BASE)
            // memory -> memory
            ccr |= DMA_CCR_MEM2MEM | DMA_CCR_PINC;
        else
        {
            // memory -> device
            ccr |= DMA_CCR_DIR;
            // src must be the peripherial
            XCHG (src, dst);
        }
    }
    /*else
        // device -> device impossible, so it is device -> memory
        ccr &= ~DMA_CCR_DIR*/

    // Disable DMA channel if it was enabled
    dmac->CCR = 0;
    // Clear DMA status for the channel
    _dma->IFCR = (DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 |
        DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1) << (chan * 4);

    // Set up number of elements to copy
    dmac->CNDTR = count;
    // Set up source & dest addresses
    dmac->CPAR = (uint32_t)src;
    dmac->CMAR = (uint32_t)dst;

    // Run!
    dmac->CCR = ccr | DMA_CCR_EN;
}

void
#ifdef _DMA_NUM
    JOIN3 (dma, _DMA_NUM, _stop) (
#else
    dma_stop (DMA_TypeDef *_dma,
#endif
        unsigned chan)
{
    // user counts channels from 1, we count from 0
    chan--;

    DMA_Channel_TypeDef *dmac = (DMA_Channel_TypeDef *)((uint8_t *)_dma +
        (DMA1_Channel1_BASE - DMA1_BASE) +
        (DMA1_Channel2_BASE - DMA1_Channel1_BASE) * chan);

    // Disable DMA channel if it was enabled
    dmac->CCR = 0;
    // Quirk: We must do it TWICE, otherwise DMA won't reset
    dmac->CCR = 0;

    // Clear DMA status for the channel
    _dma->IFCR = (DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 |
        DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1) << (chan * 4);
}

#undef _dma
#undef _DMA_NUM
