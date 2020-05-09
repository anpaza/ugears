/*
    One-shot timers
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef __OST_H__
#define __OST_H__

/**
 * @file ost.h
 *      Simple one-shot timers.
 *
 *      Uses the global 'clock' variable which is supposed to be incremented
 *      CLOCKS_PER_SEC times per second.
 */

#include "useful.h"
#include "time.h"

/// Тип таймера, умеющего отсчитывать до 2^32 тиков
typedef uint32_t ost32_t;

/// Тип таймера, умеющего отсчитывать до 2^16 тиков
typedef uint16_t ost16_t;

/**
 * Взвести одноразовый таймер (one-shot timer).
 * Таймер "сработает" через заданное число щелчков системного таймера.
 * @arg timer
 *      Указатель на переменную-таймер
 * @arg clocks
 *      Число щелчков, через которые должен сработать таймер
 */
extern void ost32_arm (volatile ost32_t *timer, clock_t clocks);

/**
 * Аналогично ost32_arm, но работает с 16-разрядными таймерами.
 * @arg timer
 *      Указатель на переменную-таймер
 * @arg clocks
 *      Число щелчков, через которые должен сработать таймер
 */
extern void ost16_arm (volatile ost16_t *timer, clock_t clocks);

/**
 * Проверить, не истёк ли одноразовый таймер,
 * также возвращает не ноль если таймер отключён.
 * @arg timer
 *      Указатель на переменную-таймер
 * @return
 *      true если таймер истёк либо выключен
 */
extern bool ost32_expired (volatile ost32_t *timer);

/**
 * Аналогично ost32_expired, но работает с 16-разрядными таймерами.
 * @arg timer
 *      Указатель на переменную-таймер
 * @return
 *      true если таймер истёк либо выключен
 */
extern bool ost16_expired (volatile ost16_t *timer);

/**
 * Проверить, включён ли таймер
 */
#define ost_enabled(timer) \
    (timer != OST_DISABLED)

/**
 * Выключить таймер
 */
#define ost_disable(timer) \
    *(timer) = OST_DISABLED

/// Значение для инициализации статических таймеров
#define OST_DISABLED	0

/// Arm a simple timeout timer
#define stt_arm(c)              (clock + c)
/// Check if a simple timeout timer expired
#define stt_expired(t)          ((int32_t)(t - clock) < 0)

#endif // __RTC_H__
