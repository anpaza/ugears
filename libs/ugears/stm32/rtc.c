/*
    Real-Time-Clock library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears.h"
#include "useful.h"

#ifndef RTC_INITTIME
/// If not defined, start counting time from 0
#define RTC_INITTIME 0
#endif
#ifndef RTC_CALIB
#define RTC_CALIB 64
#endif

#ifdef RTC_TYPE_1

void rtc_init ()
{
    // Включим тактирование модуля независимого питания RTC, а также самого RTC
    RCC_BEGIN;
        RCC_ENA (_PWR);
        RCC_ENA (_BKP);
    RCC_END;

    // Разрешим доступ на запись в регистры RTC
    PWR->CR |= PWR_CR_DBP;

    // Если часы не в нужном режиме работы, делаем полный сброс и конфигурируем заново
    if (((RCC->BDCR & (RCC_BDCR_LSEON | RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL)) !=
        (RCC_BDCR_LSEON | RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE)) ||
        (rtc_reload () != (RTC_FREQ / RTC_TICKS_PER_SEC) - 1))
        rtc_reset ();

    // Запускаем процесс обновления образов регистров RTC
    rtc_invalidate ();
    // Ждём синхронизации регистров RTC с их бледными копиями на APB1
    rtc_wait_sync ();
}

void rtc_reset ()
{
    // Полностью сбросим энергонезависимые часы
    RCC->BDCR = RCC_BDCR_BDRST;
    // Включим внешний кварц
    RCC->BDCR = RCC_BDCR_LSEON;
    // Ждём пока кварц заведётся (можно не ждать, но для порядка...)
    while (!(RCC->BDCR & RCC_BDCR_LSERDY)) ;
    // Запустим тактирование RTC от внешнего кварца
    RCC->BDCR = RCC_BDCR_LSEON | RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;

    // Начальная настройка RTC
    RTC_WRITE
    {
        // Reset counter to 0, ignoring overflow
        rtc_set_counter ((uint32_t)((uint64_t)(RTC_INITTIME) * RTC_TICKS_PER_SEC));
        // Reset alarm
        rtc_set_alarm (0xffffffff);
        // Set up RTC reload counter
        rtc_set_reload ((RTC_FREQ / RTC_TICKS_PER_SEC) - 1);
        // Set clock adjustment to half of the range (-64..+63)
        rtc_calibrate (RTC_CALIB);
    }
}

#elif defined RTC_TYPE_2

// Not implemented yet

#else

#error "Your MCU uses an unknown RTC type"

#endif
