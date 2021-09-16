/*
    This is not a header file, but a flexible piece of code.
    Prior to including this file a number of macros should/can be defined,
    after leaving the file all these macros are undefined.

    _DMA_NUM - the DMA controller number. If not defined, generates generic code
        which accepts a pointer to DMA controller as 1st parameter.
*/

#include <ugears/ugears.h>
#include <useful/useful.h>

void
#ifdef _DMA_NUM
    JOIN3 (dma, _DMA_NUM, _copy) (
#define _dma	JOIN2 (DMA, _DMA_NUM)
#else
    dma_copy (DMA_TypeDef *_dma,
#endif
        unsigned strm, uint32_t ccr, volatile void *src, volatile void *dst, unsigned count)
{
    // Unfortunately, these checks won't work in preprocessor because
    // PERIPH_BASE is not just a const like it is supposed to be.
    // So we'll use a C expression which will blank out the entire
    // function if the check fails, and optimize out completely otherwise...
    if (STM32_FLASH_ORIGIN >= PERIPH_BASE)
        // DMA library expects FLASH memory address < peripherial address
        return;
    if (STM32_RAM_ORIGIN >= PERIPH_BASE)
        // DMA library expects RAM address < peripherial address
        return;

#if defined DMA_TYPE_1
    // user counts channels from 1, we count from 0
    strm--;

    DMA_Channel_TypeDef *dmac = (DMA_Channel_TypeDef *)((uint8_t *)_dma +
        (DMA1_Channel1_BASE - DMA1_BASE) +
        (DMA1_Channel2_BASE - DMA1_Channel1_BASE) * strm);

    // -- Deduce the missing bits from ccr -- //

    // Drop bits that should not be set by user
    ccr &= ~(DMA_CCR_DIR_Msk | DMA_CCR_MINC_Msk | DMA_CCR_PINC_Msk);

    // Memory address is always incremented, no matter of transfer direction
    ccr |= DMA_CCR_MINC;

    if ((uintptr_t)src < PERIPH_BASE)
    {
        if ((uintptr_t)dst < PERIPH_BASE)
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
    while (dmac->CCR & DMA_CCR_EN)
        dmac->CCR = 0;
    // Clear DMA status for the channel
    _dma->IFCR = (DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 |
        DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1) << (strm * 4);

    // Set up number of elements to copy
    dmac->CNDTR = count;
    // Set up source & dest addresses
    dmac->CPAR = (uintptr_t)src;
    dmac->CMAR = (uintptr_t)dst;

    // Run!
    dmac->CCR = ccr | DMA_CCR_EN;

#elif defined DMA_TYPE_2

    DMA_Stream_TypeDef *dmas = (DMA_Stream_TypeDef *)((uint8_t *)_dma +
        (DMA1_Stream0_BASE - DMA1_BASE) +
        (DMA1_Stream1_BASE - DMA1_Stream0_BASE) * strm);

    // -- Deduce the missing bits from ccr -- //

    // Drop bits that should not be set by user
    ccr &= ~(DMA_SxCR_DIR_Msk | DMA_SxCR_MINC | DMA_SxCR_PINC);

    // Memory address is always incremented, no matter of transfer direction
    ccr |= DMA_SxCR_MINC;

    if ((uintptr_t)src < PERIPH_BASE)
    {
        if ((uintptr_t)dst < PERIPH_BASE)
            // memory -> memory
            ccr |= DMA_SxCR_DIR_M2M | DMA_SxCR_PINC;
        else
        {
            // memory -> device
            ccr |= DMA_SxCR_DIR_M2P;
            // src must be the peripherial
            XCHG (src, dst);
        }
    }
    else
        // device -> device impossible, so it is device -> memory
        ccr |= DMA_SxCR_DIR_P2M;

    // Disable DMA channel if it was enabled
    while (dmas->CR & DMA_SxCR_EN)
        dmas->CR = 0;
    // Clear DMA status for the channel
    *dma_ifcr (_dma, strm) =
            DMA_IFCR_IF_ (TC, strm) | DMA_IFCR_IF_ (HT, strm) |
            DMA_IFCR_IF_ (TE, strm) | DMA_IFCR_IF_ (DME, strm) |
            DMA_IFCR_IF_ (FE, strm);

    // Set up number of elements to copy
    dmas->NDTR = count;
    // Set up source & dest addresses
    dmas->PAR = (uintptr_t)src;
    dmas->M0AR = (uintptr_t)dst;

    // Run!
    dmas->CR = ccr | DMA_SxCR_EN;

#endif
}

void
#ifdef _DMA_NUM
    JOIN3 (dma, _DMA_NUM, _stop) (
#else
    dma_stop (DMA_TypeDef *_dma,
#endif
        unsigned strm)
{
#if defined DMA_TYPE_1
    // user counts channels from 1, we count from 0
    strm--;

    DMA_Channel_TypeDef *dmac = (DMA_Channel_TypeDef *)((uint8_t *)_dma +
        (DMA1_Channel1_BASE - DMA1_BASE) +
        (DMA1_Channel2_BASE - DMA1_Channel1_BASE) * strm);

    // Disable DMA channel if it was enabled
    while (dmac->CCR & DMA_CCR_EN)
        dmac->CCR = 0;

    // Clear DMA status for the channel
    _dma->IFCR = (DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 |
        DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1) << (strm * 4);

#elif defined DMA_TYPE_2

    DMA_Stream_TypeDef *dmas = (DMA_Stream_TypeDef *)((uint8_t *)_dma +
        (DMA1_Stream0_BASE - DMA1_BASE) +
        (DMA1_Stream1_BASE - DMA1_Stream0_BASE) * strm);

    // Disable DMA channel if it was enabled
    while (dmas->CR & DMA_SxCR_EN)
        dmas->CR = 0;

    // Clear DMA status for the channel
    *dma_ifcr (_dma, strm) =
            DMA_IFCR_IF_ (TC, strm) | DMA_IFCR_IF_ (HT, strm) |
            DMA_IFCR_IF_ (TE, strm) | DMA_IFCR_IF_ (DME, strm) |
            DMA_IFCR_IF_ (FE, strm);

#endif
}

#undef _dma
#undef _DMA_NUM
