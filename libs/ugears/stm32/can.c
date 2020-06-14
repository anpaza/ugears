/*
    STM32 low-level CAN library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include <ugears/ugears.h>

#ifdef HAS_CAN

#define INAK_TIMEOUT		0x0000ffff

bool can_init (CAN_TypeDef *can, uint32_t mode)
{
    uint32_t timeout;

    // Totally reset the bxCAN peripherial
    can->MCR = CAN_MCR_RESET;
    while (can->MCR & CAN_MCR_RESET)
        ;

    // Leave sleep mode
    can->MCR &= (~(uint32_t)CAN_MCR_SLEEP);

    // Enter initialization mode
    can->MCR |= CAN_MCR_INRQ;

    for (timeout = INAK_TIMEOUT; timeout != 0; timeout--)
        if (can->MSR & CAN_MSR_INAK)
            break;

    if (timeout == 0)
        return false;

    uint32_t mcr = can->MCR & ~(CAN_MCR_TTCM | CAN_MCR_ABOM | CAN_MCR_AWUM |
        CAN_MCR_NART | CAN_MCR_RFLM | CAN_MCR_TXFP);

    if (mode & CAN_TIMESTAMPS)
        mcr |= CAN_MCR_TTCM;

    if (mode & CAN_BUSOFF_RECOVERY)
        mcr |= CAN_MCR_ABOM;

    if (mode & CAN_WAKEUP_ON_RECEIVE)
        mcr |= CAN_MCR_AWUM;

    if (mode & CAN_NO_RETRANSMIT)
        mcr |= CAN_MCR_NART;

    if (mode & CAN_RXFIFO_LOCKED)
        mcr |= CAN_MCR_RFLM;

    if (mode & CAN_TX_SEQUENTIALLY)
        mcr |= CAN_MCR_TXFP;

    can->MCR = mcr;

    uint32_t speed = CAN_BITRATE (mode);

    // Вычислим самый точный tq (в тактах APB1), исходя из
    // желаемой длины Nominal Bit Time примерно в (8..20)*tq
    int tq_delta = INT32_MAX;
    uint32_t bs1 = 0;
    uint32_t prescaler = 0;
    for (uint32_t nbt_tq = 8; nbt_tq < 21; nbt_tq++)
    {
        uint32_t nbt_tq_speed = nbt_tq * speed;
        uint32_t tq = (PCLK1_FREQ + nbt_tq_speed / 2) / nbt_tq_speed;
        int delta = ABS ((int)(PCLK1_FREQ - tq * nbt_tq * speed));
        if (delta < tq_delta)
        {
            bs1 = nbt_tq;
            prescaler = tq;
            if ((tq_delta = delta) == 0)
                break;
        }
    }

    // Resynchronization jump width примерно в 1/8 от общей длины NBT
    uint32_t sjw = (bs1 / 8) - 1;
    // Интервал BS2 устанавливаем равным 25% от количества tq в NBT
    uint32_t bs2 = ((bs1 + 2) / 4) - 1;
    // Интервал BS1 равен полной длине NBT минус 1 такт синхронизации минус такты BS2
    bs1 -= bs2 + 3;

    // Запишем вычисленные тайминги в регистр
    can->BTR = (mode & (CAN_BTR_LBKM | CAN_BTR_SILM)) |
               (sjw << 24) | (bs2 << 20) | (bs1 << 16) |
               (prescaler - 1);

    // Leave initialization mode
    can->MCR &= ~CAN_MCR_INRQ;

    return true;
}

bool can_wait_bus (CAN_TypeDef *can)
{
    uint32_t timeout;

    for (timeout = INAK_TIMEOUT; timeout != 0; timeout--)
        if ((can->MSR & CAN_MSR_INAK) == 0)
            break;

    if (timeout == 0)
        return false;

    return true;
}

void can_filter_split (uint32_t fbn)
{
    if (fbn > 28)
        return;

    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FMR = (CAN1->FMR & ~0x3F00) | (fbn << 8);
    CAN1->FMR &= ~CAN_FMR_FINIT;
}

void can_filter_reset ()
{
    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FM1R = 0;
    CAN1->FS1R = 0;
    CAN1->FFA1R = 0;
    CAN1->FA1R = 0;
    CAN1->FMR &= ~CAN_FMR_FINIT;
}

void can_filter_disable (uint32_t n)
{
    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FA1R &= ~BV (n);
    CAN1->FMR &= ~CAN_FMR_FINIT;
}

void can_filter (uint32_t n, uint32_t fr1, uint32_t fr2, uint32_t flags)
{
    uint32_t nb = BV (n);
    CAN_FilterRegister_TypeDef *filter = &CAN1->sFilterRegister[n];

    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FA1R &= ~nb;

    filter->FR1 = fr1;
    filter->FR2 = fr2;

    if (flags & CAN_FILTER_32BIT)
        CAN1->FS1R |= nb;
    else
        CAN1->FS1R &= ~nb;

    if (flags & CAN_FILTER_MASK)
        CAN1->FM1R &= ~nb;
    else
        CAN1->FM1R |= nb;

    if (flags & CAN_FILTER_FIFO1)
        CAN1->FFA1R |= nb;
    else
        CAN1->FFA1R &= ~nb;

    CAN1->FA1R |= nb;
    CAN1->FMR &= ~CAN_FMR_FINIT;
}

#endif
