/*
    i2c command execution
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

/* On entry:
 * I2C_IDX - Numeric I2C controller index
 * I2C1_TX_DMA_NUM,
 * I2C2_TX_DMA_NUM      - The DMA controller number bound to respective I2C controller
 *                        for transmission.
 * I2C1_TX_DMA_CHAN,
 * I2C2_TX_DMA_CHAN     - The DMA controller channel bound to respective I2C controller
 *                        for transmission.
 * I2C1_RX_DMA_NUM,
 * I2C2_RX_DMA_NUM      - The DMA controller number bound to respective I2C controller
 *                        for receiving.
 * I2C1_RX_DMA_CHAN,
 * I2C2_RX_DMA_CHAN     - The DMA controller number bound to respective I2C controller
 *                        for receiving.
 * I2C1_TX_DMA_SUFX,
 * I2C2_TX_DMA_SUFX     - the DMA IRQ handler suffix (define if you need to share the IRQ)
 */

#undef I2C
#undef i2cmd
#define I2C_TX			JOIN3(I2C, I2C_IDX, _TX)
#define I2C_RX			JOIN3(I2C, I2C_IDX, _RX)
#define I2C_TX_DMA_NUM		JOIN2(I2C_TX, _DMA_NUM)
#define I2C_TX_DMA_CHAN		JOIN2(I2C_TX, _DMA_CHAN)
#define I2C_TX_DMA_SUFX		JOIN2(I2C_TX, _DMA_SUFX)
#define I2C_RX_DMA_NUM		JOIN2(I2C_RX, _DMA_NUM)
#define I2C_RX_DMA_CHAN		JOIN2(I2C_RX, _DMA_CHAN)
#define I2C_RX_DMA_SUFX		JOIN2(I2C_RX, _DMA_SUFX)
#define I2C_			JOIN2(I2C, I2C_IDX)
#define i2cmd			JOIN2(i2cmd, I2C_IDX)
#define dma_copy		JOIN3(dma, DMA_NUM (I2C_TX), _copy)
#define dma_stop		JOIN3(dma, DMA_NUM (I2C_TX), _stop)

static void JOIN3 (i2ce, I2C_IDX, _finish) (bool error)
{
    // we don't acknowledge anymore, we don't START nor STOP
    I2C_->CR1 &= ~(I2C_CR1_ACK | I2C_CR1_STOP | I2C_CR1_START);
    // disable DMA and buffer interrupts
    I2C_->CR2 &= ~(I2C_CR2_LAST | I2C_CR2_DMAEN | I2C_CR2_ITBUFEN);

    // Finished - invoke user callback
    i2cmd.state = i2cesIdle;
    if (i2cmd.eoc)
        i2cmd.eoc (error);

    // If user callback did not invoke another operation, stop the bus
    if ((i2cmd.state == i2cesIdle) && (I2C_->SR2 & I2C_SR2_MSL))
    {
        // disable i2c interrupts
        I2C_->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
        // emit the STOP condition
        I2C_->CR1 |= I2C_CR1_STOP;
    }
}

void JOIN3 (i2ce, I2C_IDX, _command) ()
{
    uint32_t len;
    uint32_t cmd_idx = i2cmd.cmd_idx;
    uint32_t opcode = i2cmd.cmd [cmd_idx++];

    switch (opcode & I2CMD_OPCODE_MASK)
    {
        case I2CMD_OPCODE_EOC:
            JOIN3 (i2ce, I2C_IDX, _finish) (false);
            break;

        case I2CMD_OPCODE_START:
        {
            i2cmd_state_t st = i2cmd.state;
            i2cmd.state = i2cesStart;
            i2cmd.cmd_idx = cmd_idx;

            // Don't START again if START has been already set in previous I2CMD_OPCODE_RECV
            if (st != i2cesRecvIRQ)
                I2C_->CR1 |= I2C_CR1_START;
            break;
        }

        case I2CMD_OPCODE_SEND:
            len = (opcode & ~I2CMD_OPCODE_MASK) + 1;
            i2cmd.state = i2cesSendDMA;
            i2cmd.cmd_idx = cmd_idx + len;

            // Transmit bytes to the bus
            if (len == 1)
            {
                I2C_->CR2 &= ~(I2C_CR2_DMAEN | I2C_CR2_LAST);
                I2C_->DR = i2cmd.cmd [cmd_idx];
            }
            else
            {
                I2C_->CR2 = (I2C_->CR2 & ~(I2C_CR2_LAST | I2C_CR2_ITBUFEN)) | I2C_CR2_DMAEN;
                dma_copy (DMA_CHAN (I2C_TX),
                    DMA_CCR_TCIE | DMA_CCR_TEIE | DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8,
                    (void *)(i2cmd.cmd + cmd_idx), &I2C_->DR, len);
            }
            break;

        case I2CMD_OPCODE_RECV:
            len = (opcode & ~I2CMD_OPCODE_MASK) + 1;
recv:
            // Receive bytes from the bus
            i2cmd.cmd_idx = cmd_idx;

            // "When using DMA, master reception of a single byte is not supported"
            if (len == 1)
            {
                i2cmd.state = i2cesRecvIRQ;
                i2cmd.rxbuff_idx++;
                uint32_t cr1 = I2C_->CR1 & ~I2C_CR1_ACK;
                // If next command is START or STOP, set the respective flags
                switch (i2cmd.cmd [cmd_idx] & I2CMD_OPCODE_MASK)
                {
                    case I2CMD_OPCODE_EOC:   cr1 |= I2C_CR1_STOP;  break;
                    case I2CMD_OPCODE_START: cr1 |= I2C_CR1_START; break;
                }
                I2C_->CR1 = cr1;
                I2C_->CR2 = (I2C_->CR2 & ~(I2C_CR2_DMAEN | I2C_CR2_LAST)) | I2C_CR2_ITBUFEN;
            }
            else
            {
                uint32_t rxbuff_idx = i2cmd.rxbuff_idx;
                i2cmd.rxbuff_idx = rxbuff_idx + len;
                i2cmd.state = i2cesRecvDMA;
                // acknowledge bytes until DMA sends EOT
                I2C_->CR1 |= I2C_CR1_ACK;
                I2C_->CR2 = (I2C_->CR2 & ~I2C_CR2_ITBUFEN) | I2C_CR2_DMAEN | I2C_CR2_LAST;
                dma_copy (DMA_CHAN (I2C_RX),
                    DMA_CCR_TCIE | DMA_CCR_TEIE | DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8,
                    &I2C_->DR, i2cmd.rxbuff + rxbuff_idx, len);
            }
            break;

        case I2CMD_OPCODE_SEND_BUF:
        {
            len = (((opcode & ~I2CMD_OPCODE_MASK) << 8) | (i2cmd.cmd [cmd_idx++])) + 1;
            I2C_->CR2 = (I2C_->CR2 & ~(I2C_CR2_LAST | I2C_CR2_ITBUFEN)) | I2C_CR2_DMAEN;
            i2cmd.state = i2cesSendDMA;
            i2cmd.cmd_idx = cmd_idx;
            uint32_t txbuff_idx = i2cmd.txbuff_idx;
            i2cmd.txbuff_idx = txbuff_idx + len;

            // Transmit bytes to the bus
            dma_copy (DMA_CHAN (I2C_TX),
                DMA_CCR_TCIE | DMA_CCR_TEIE | DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8,
                (void *)(i2cmd.txbuff + txbuff_idx), &I2C_->DR, len);
            break;
        }

        case I2CMD_OPCODE_RECV_BUF:
            len = (((opcode & ~I2CMD_OPCODE_MASK) << 8) | (i2cmd.cmd [cmd_idx++])) + 1;
            goto recv;

        case I2CMD_OPCODE_ADDR7:
        {
            i2cmd.state = i2cesAddr7;
            i2cmd.cmd_idx = cmd_idx + 1;
            I2C_->DR = i2cmd.cmd [cmd_idx];
            break;
        }

        case I2CMD_OPCODE_ADDR10:
            // If address is sent in receiver mode, there are no lower 8 bits following
            i2cmd.state = ((opcode & 0xF1) == 0xF1) ? i2cesAddr7 : i2cesAddr10;
            i2cmd.cmd_idx = cmd_idx;
            // Send the first byte of address
            I2C_->DR = opcode;
            break;
    }
}

void JOIN3 (i2ce, I2C_IDX, _abort) ()
{
    dma_stop (DMA_CHAN (I2C_TX));
    dma_stop (DMA_CHAN (I2C_RX));

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

    if (i2cmd.state != i2cesIdle)
        JOIN3 (i2ce, I2C_IDX, _finish) (true);
}

bool JOIN3 (i2ce, I2C_IDX, _grasp) (const void *cmd)
{
    bool rc = false;

    ATOMIC_BLOCK (FORCEON)
    {
        if (i2cmd.state == i2cesIdle)
        {
            i2cmd.state = i2cesWarmup;
            rc = true;
        }
    }

    if (rc)
    {
        // initialize the i2c command engine
        i2cmd.cmd = cmd;
        i2cmd.cmd_idx = i2cmd.txbuff_idx = i2cmd.rxbuff_idx = 0;
        i2cmd.txbuff = i2cmd.rxbuff = NULL;
        i2cmd.eoc = NULL;

        // enable i2c interrupts
        I2C_->CR2 |= I2C_CR2_ITEVTEN | I2C_CR2_ITERREN;
    }

    return rc;
}

void DMA_IRQ_HANDLER (I2C_TX, I2C_TX_DMA_SUFX) ()
{
    uint32_t isr = DMA (I2C_TX)->ISR;

    // Transfer error?
    if (isr & DMA_ISR (I2C_TX, TEIF))
    {
        // Acknowledge the interrupt
        DMA (I2C_TX)->IFCR = DMA_IFCR (I2C_TX, CTEIF);

        // Notify user callback of error
        JOIN3 (i2ce, I2C_IDX, _abort) ();
    }
    else if (isr & DMA_ISR (I2C_TX, GIF))
    {
        // Acknowledge the interrupt
        DMA (I2C_TX)->IFCR = DMA_IFCR (I2C_TX, CGIF);
        // Next command will be proceeded in EV_IRQHandler
    }
}

void DMA_IRQ_HANDLER (I2C_RX, I2C_RX_DMA_SUFX) ()
{
    uint32_t isr = DMA (I2C_RX)->ISR;

    // Transfer error?
    if (isr & DMA_ISR (I2C_RX, TEIF))
    {
        // Acknowledge the interrupt
        DMA (I2C_RX)->IFCR = DMA_IFCR (I2C_RX, CTEIF);

        // Notify user callback of error
        JOIN3 (i2ce, I2C_IDX, _abort) ();
    }
    else if (isr & DMA_ISR (I2C_RX, GIF))
    {
        // Acknowledge the interrupt
        DMA (I2C_RX)->IFCR = DMA_IFCR (I2C_RX, CGIF);
        if (i2cmd.state == i2cesRecvDMA)
            // Go on with the next command
            JOIN3 (i2ce, I2C_IDX, _command) ();
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

    switch (i2cmd.state)
    {
        case i2cesStart:
            if (sr & I2C_SR1_SB)
                JOIN3 (i2ce, I2C_IDX, _command) ();
            break;

        case i2cesAddr7:
            // If address has been sent, advance to next command
            if (sr & I2C_SR1_ADDR)
                JOIN3 (i2ce, I2C_IDX, _command) ();
            break;

        case i2cesAddr10:
            if (sr & I2C_SR1_ADD10)
            {
                // Send lower 8 bits of address
                i2cmd.state = i2cesAddr7;
                I2C_->DR = i2cmd.cmd [i2cmd.cmd_idx++];
            }
            break;

        case i2cesRecvIRQ:
            // Receive one byte without DMA
            if ((sr & I2C_SR1_RXNE) == 0)
                break;

            I2C_->CR2 &= ~I2C_CR2_ITBUFEN;
            ((uint8_t *)i2cmd.rxbuff) [i2cmd.rxbuff_idx - 1] = I2C_->DR;
            // fallback to _command()

        case i2cesSendDMA:
            JOIN3 (i2ce, I2C_IDX, _command) ();
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

#undef i2cmd
#undef I2C_
#undef I2C_IDX
#undef I2C_RX_DMA_SUFX
#undef I2C_RX_DMA_CHAN
#undef I2C_RX_DMA_NUM
#undef I2C_TX_DMA_SUFX
#undef I2C_TX_DMA_CHAN
#undef I2C_TX_DMA_NUM
#undef I2C_RX
#undef I2C_TX
