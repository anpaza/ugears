/*
    i2c engine
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

/* On entry:
 * I2C_IDX - Numeric I2C controller index
 * I2C1_TX_DMA_NUM,
 * I2C2_TX_DMA_NUM      - The DMA controller number bound to respective I2C controller
 *                        for transmission.
 * I2C1_TX_DMA_STRM,
 * I2C2_TX_DMA_STRM     - The DMA controller stream bound to respective I2C controller
 *                        for transmission.
 * I2C1_TX_DMA_CHAN,
 * I2C2_TX_DMA_CHAN     - (DMA_TYPE_2 only) The channel of the stream used to connect
 *                        to the respective I2C controller's TX.
 * I2C1_RX_DMA_NUM,
 * I2C2_RX_DMA_NUM      - The DMA controller number bound to respective I2C controller
 *                        for receiving.
 * I2C1_RX_DMA_STRM,
 * I2C2_RX_DMA_STRM     - The DMA controller stream bound to respective I2C controller
 *                        for receiving.
 * I2C1_RX_DMA_CHAN,
 * I2C2_RX_DMA_CHAN     - (DMA_TYPE_2 only) The channel of the stream used to connect
 *                        to the respective I2C controller's RX.
 * I2C1_TX_DMA_SUFX,
 * I2C2_TX_DMA_SUFX     - the DMA IRQ handler suffix (define if you need to share the IRQ)
 */

#ifndef I2C_IDX
// Calm down advanced IDEs which would mark most code below as invalid
#  include "useful/useful.h"
#  include "useful/atomic.h"
#  include "ugears/ugears.h"
#  define I2C_IDX 1
#  define I2C1_TX_DMA_SUFX
#  define I2C1_RX_DMA_SUFX
#endif

#undef I2C
#undef i2cmd
#define I2C_TX			JOIN3(I2C, I2C_IDX, _TX)
#define I2C_RX			JOIN3(I2C, I2C_IDX, _RX)
#define I2C_TX_DMA_SUFX		JOIN2(I2C_TX, _DMA_SUFX)
#define I2C_RX_DMA_SUFX		JOIN2(I2C_RX, _DMA_SUFX)
#define I2C_			JOIN2(I2C, I2C_IDX)
#define i2ces			JOIN2(i2ces, I2C_IDX)
#define dma_copy		JOIN3(dma, DMA_NUM (I2C_TX), _copy)
#define dma_stop		JOIN3(dma, DMA_NUM (I2C_TX), _stop)

#if defined DMA_TYPE_1
#  define I2C_DMA_COPY_CCR(x) \
    (DMA_CCR_TCIE | DMA_CCR_TEIE | DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8)
#elif defined DMA_TYPE_2
#  define I2C_DMA_COPY_CCR(x) \
    (DMA_SxCR_CHSEL (x) | \
     DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_PSIZE_8 | DMA_SxCR_MSIZE_8)
#endif

static void JOIN3 (i2ce, I2C_IDX, _finish) (bool error)
{
    // we don't acknowledge anymore, we don't START nor STOP
    I2C_->CR1 &= ~(I2C_CR1_ACK | I2C_CR1_STOP | I2C_CR1_START);
    // disable DMA and buffer interrupts
    I2C_->CR2 &= ~(I2C_CR2_LAST | I2C_CR2_DMAEN | I2C_CR2_ITBUFEN);

    // Finished - invoke user callback
    i2ces.state = i2cesIdle;
    if (i2ces.eoc)
        i2ces.eoc (&i2ces, error);

    // If user callback did not invoke another operation, stop the bus
    if ((i2ces.state == i2cesIdle) && (I2C_->SR2 & I2C_SR2_MSL))
    {
        // disable i2c interrupts
        I2C_->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
        // emit the STOP condition
        I2C_->CR1 |= I2C_CR1_STOP;
    }
}

void JOIN3 (i2ce, I2C_IDX, _run) ()
{
    uint32_t len;
    uint32_t cmd_off = i2ces.cmd_off;
    uint32_t opcode = i2ces.cmd [cmd_off++];

    switch (opcode & I2CMD_OPCODE_MASK)
    {
        case I2CMD_OPCODE_EOC:
            JOIN3 (i2ce, I2C_IDX, _finish) (false);
            break;

        case I2CMD_OPCODE_START:
        {
            i2ce_fsm_state_t st = (i2ce_fsm_state_t)i2ces.state;
            i2ces.state = i2cesStart;
            i2ces.cmd_off = cmd_off;

            // Don't START again if START has been already set in previous I2CMD_OPCODE_RECV
            if (st != i2cesRecvIRQ)
                I2C_->CR1 |= I2C_CR1_START;
            break;
        }

        case I2CMD_OPCODE_SEND:
            len = (opcode & ~I2CMD_OPCODE_MASK) + 1;
            i2ces.state = i2cesSendDMA;
            i2ces.cmd_off = cmd_off + len;

            // Transmit bytes to the bus
            if (len == 1)
            {
                I2C_->CR2 &= ~(I2C_CR2_DMAEN | I2C_CR2_LAST);
                I2C_->DR = i2ces.cmd [cmd_off];
            }
            else
            {
                I2C_->CR2 = (I2C_->CR2 & ~(I2C_CR2_LAST | I2C_CR2_ITBUFEN)) | I2C_CR2_DMAEN;
                dma_copy (DMA_STRM (I2C_TX), I2C_DMA_COPY_CCR (I2C_TX),
                    (void *)(i2ces.cmd + cmd_off), &I2C_->DR, len);
            }
            break;

        case I2CMD_OPCODE_RECV:
            len = (opcode & ~I2CMD_OPCODE_MASK) + 1;
recv:
            // Receive bytes from the bus
            i2ces.cmd_off = cmd_off;

            // "When using DMA, master reception of a single byte is not supported"
            if (len == 1)
            {
                i2ces.state = i2cesRecvIRQ;
                i2ces.rxbuff_off++;
                uint32_t cr1 = I2C_->CR1 & ~I2C_CR1_ACK;
                // if we receive more, set ACK, otherwise send NACK
                // If next command is START or STOP, set the respective flags
                switch (i2ces.cmd [cmd_off] & I2CMD_OPCODE_MASK)
                {
                    case I2CMD_OPCODE_EOC:   cr1 |= I2C_CR1_STOP;  break;
                    case I2CMD_OPCODE_START: cr1 |= I2C_CR1_START; break;
                    case I2CMD_OPCODE_RECV:  cr1 |= I2C_CR1_ACK;   break;
                }
                I2C_->CR1 = cr1;
                I2C_->CR2 = (I2C_->CR2 & ~(I2C_CR2_DMAEN | I2C_CR2_LAST)) | I2C_CR2_ITBUFEN;
            }
            else
            {
                uint32_t rxbuff_off = i2ces.rxbuff_off;
                i2ces.rxbuff_off = rxbuff_off + len;
                i2ces.state = i2cesRecvDMA;
                // acknowledge bytes until DMA sends EOT
                I2C_->CR1 |= I2C_CR1_ACK;
                I2C_->CR2 = (I2C_->CR2 & ~I2C_CR2_ITBUFEN) | I2C_CR2_DMAEN | I2C_CR2_LAST;
                dma_copy (DMA_STRM (I2C_RX), I2C_DMA_COPY_CCR (I2C_RX),
                    &I2C_->DR, (uint8_t *)i2ces.rxbuff + rxbuff_off, len);
            }
            break;

        case I2CMD_OPCODE_SEND_BUF:
        {
            len = (((opcode & ~I2CMD_OPCODE_MASK) << 8) | (i2ces.cmd [cmd_off++])) + 1;
            I2C_->CR2 = (I2C_->CR2 & ~(I2C_CR2_LAST | I2C_CR2_ITBUFEN)) | I2C_CR2_DMAEN;
            i2ces.state = i2cesSendDMA;
            i2ces.cmd_off = cmd_off;
            uint32_t txbuff_off = i2ces.txbuff_off;
            i2ces.txbuff_off = txbuff_off + len;

            // Transmit bytes to the bus
            dma_copy (DMA_STRM (I2C_TX), I2C_DMA_COPY_CCR (I2C_TX),
                (uint8_t *)i2ces.txbuff + txbuff_off, &I2C_->DR, len);
            break;
        }

        case I2CMD_OPCODE_RECV_BUF:
            len = (((opcode & ~I2CMD_OPCODE_MASK) << 8) | (i2ces.cmd [cmd_off++])) + 1;
            goto recv;

        case I2CMD_OPCODE_ADDR7:
        {
            i2ces.state = i2cesAddr7;
            i2ces.cmd_off = cmd_off + 1;
            I2C_->DR = i2ces.cmd [cmd_off];
            break;
        }

        case I2CMD_OPCODE_ADDR10:
            // If address is sent in receiver mode, there are no lower 8 bits following
            i2ces.state = ((opcode & 0xF1) == 0xF1) ? i2cesAddr7 : i2cesAddr10;
            i2ces.cmd_off = cmd_off;
            // Send the first byte of address
            I2C_->DR = opcode;
            break;
    }
}

void JOIN3 (i2ce, I2C_IDX, _abort) ()
{
    dma_stop (DMA_STRM (I2C_TX));
    dma_stop (DMA_STRM (I2C_RX));

    if (I2C_->SR2 & I2C_SR2_BUSY)
        // if line is busy, reset line state
        I2C_->CR1 |= I2C_CR1_SWRST;
    else
        // disable the peripherial
        I2C_->CR1 &= ~I2C_CR1_PE;

    // abort all actions
    I2C_->CR1 &= ~(I2C_CR1_ACK | I2C_CR1_STOP | I2C_CR1_START);
    // just in case, disable DMA
    I2C_->CR2 &= ~(I2C_CR2_LAST | I2C_CR2_DMAEN | I2C_CR2_ITBUFEN);
    // clear status registers
    I2C_->SR1 = 0;
    // enable the I2C controller back, take out from reset
    I2C_->CR1 = (I2C_->CR1 & ~I2C_CR1_SWRST) | I2C_CR1_PE;

    if (i2ces.state != i2cesIdle)
        JOIN3 (i2ce, I2C_IDX, _finish) (true);
}

bool JOIN3 (i2ce, I2C_IDX, _lock) (const void *cmd)
{
    uint8_t expected = i2cesIdle;
    if (!cmpxchg_u8 ((uint8_t *)&i2ces/*.state*/, &expected, i2cesWarmup))
        return false;

    // initialize the i2c command engine
    i2ces.cmd = (const uint8_t *)cmd;
    i2ces.cmd_off = i2ces.txbuff_off = i2ces.rxbuff_off = 0;
    i2ces.txbuff = i2ces.rxbuff = NULL;
    i2ces.eoc = NULL;

    // enable i2c interrupts
    I2C_->CR2 |= I2C_CR2_ITEVTEN | I2C_CR2_ITERREN;

    return true;
}

void JOIN2 (DMA_IRQ_HANDLER (I2C_TX), I2C_TX_DMA_SUFX) ()
{
    uint32_t isr = DMA_ISR (I2C_TX);

    // Transfer error?
    if (isr & DMA_ISR_IF (TE, I2C_TX))
    {
        // Acknowledge the interrupt
        DMA_IFCR (I2C_TX) = DMA_IFCR_IF (TE, I2C_TX);

        // Notify user callback of error
        JOIN3 (i2ce, I2C_IDX, _abort) ();
    }
    else if (isr & DMA_ISR_IF (TC, I2C_TX))
    {
        // Acknowledge the interrupt
        DMA_IFCR (I2C_TX) = DMA_IFCR_IF (TC, I2C_TX);
        // Next command will be proceeded in EV_IRQHandler
    }
}

void JOIN2 (DMA_IRQ_HANDLER (I2C_RX), I2C_RX_DMA_SUFX) ()
{
    uint32_t isr = DMA_ISR (I2C_RX);

    // Transfer error?
    if (isr & DMA_ISR_IF (TE, I2C_RX))
    {
        // Acknowledge the interrupt
        DMA_IFCR (I2C_RX) = DMA_IFCR_IF (TE, I2C_RX);

        // Notify user callback of error
        JOIN3 (i2ce, I2C_IDX, _abort) ();
    }
    else if (isr & DMA_ISR_IF (TC, I2C_RX))
    {
        // Acknowledge the interrupt
        DMA_IFCR (I2C_RX) = DMA_IFCR_IF (TC, I2C_RX);

        if (i2ces.state == i2cesRecvDMA)
            // Go on with the next command
            JOIN3 (i2ce, I2C_IDX, _run) ();
    }
}

void JOIN3 (I2C, I2C_IDX, _EV_IRQHandler) ()
{
    // read all status bits into one register
    uint32_t sr = I2C_->SR1;
    sr |= (I2C_->SR2 << 16);

    if (sr & I2C_SR1_STOPF)
    {
        // The last move of the mumbo-jumbo dance
        // "It is recommended to perform the complete clearing sequence
        // (READ SR1 then WRITE CR1) after the STOPF is set"
        // is performed in the first line of _finish()
        JOIN3 (i2ce, I2C_IDX, _finish) (false);
        return;
    }

    switch (i2ces.state)
    {
        case i2cesStart:
            if (sr & I2C_SR1_SB)
                JOIN3 (i2ce, I2C_IDX, _run) ();
            break;

        case i2cesAddr7:
            // If address has been sent, advance to next command
            if (sr & I2C_SR1_ADDR)
                JOIN3 (i2ce, I2C_IDX, _run) ();
            break;

        case i2cesAddr10:
            if (sr & I2C_SR1_ADD10)
            {
                // Send lower 8 bits of address
                i2ces.state = i2cesAddr7;
                I2C_->DR = i2ces.cmd [i2ces.cmd_off++];
            }
            break;

        case i2cesRecvIRQ:
            // Receive one byte without DMA
            if ((sr & I2C_SR1_RXNE) == 0)
                break;

            I2C_->CR2 &= ~I2C_CR2_ITBUFEN;
            ((uint8_t *)i2ces.rxbuff) [i2ces.rxbuff_off - 1] = I2C_->DR;

            // fall-through

        case i2cesSendDMA:
            JOIN3 (i2ce, I2C_IDX, _run) ();
            break;

        case i2cesWarmup:
        case i2cesIdle:
        case i2cesRecvDMA:
            // Never reach here in these states
            break;
    }
}

void JOIN3 (I2C, I2C_IDX, _ER_IRQHandler) ()
{
    bool finish = false;
    uint32_t sr1 = I2C_->SR1;
    if (sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR | I2C_SR1_TIMEOUT))
    {
        // bus error, arbitration lost, overrun or timeout
        // we can't do anything except maybe restart
        sr1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR | I2C_SR1_TIMEOUT);
        finish = true;
    }
    if (sr1 & I2C_SR1_AF)
    {
        // Acknowledge failure
        sr1 &= ~I2C_SR1_AF;
        finish = true;
    }
    I2C_->SR1 = sr1;

    if (finish)
        JOIN3 (i2ce, I2C_IDX, _finish) (true);
}

#undef i2ces
#undef I2C_
#undef I2C_IDX
#undef I2C_RX_DMA_SUFX
#undef I2C_TX_DMA_SUFX
#undef I2C_RX
#undef I2C_TX
#undef I2C_DMA_COPY_CCR
