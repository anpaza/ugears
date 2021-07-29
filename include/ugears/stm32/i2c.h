/*
    STM32 I2C library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_I2C_H
#define _STM32_I2C_H

/**
 * @file i2c.h
 *      This library provides a easy way to use the i2c bus controllers.
 *      Besides basic functions, it provides an API to send and receive
 *      data via the I2C bus using DMA. For this to work, you have to
 *      define I2C1_ENGINE and/or I2C2_ENGINE in your hardware.h.
 *
 * The following macros are expected to be defined in your HARDWARE_H
 * in order to use I2C macros:
 *
 * @li HWFN_I2C_NUM defines the number of the I2C peripherial to which
 *      the feature is connected (1,2,...). Used by I2C(HWFN) macro.
 *
 * A simple usage example:
 * @code
 * i2c_init (I2C1, 100000, i2cmI2C);
 * ...
 * static const uint8_t cmd [] = {
 *     I2CMD_START,                     // generate START condition
 *     I2CMD_TX7 (ADDR_MYDEVICE),       // device address, transmitter mode
 *     I2CMD_SEND (1)                   // send 1 bytes to device
 *     12,                              // send register number
 *     I2CMD_START,                     // repeated START condition
 *     I2CMD_RX7 (ADDR_MYDEVICE),       // device address, receiver mode
 *     I2CMD_RECV (2)                   // read 2 bytes from device to buffer
 *     I2CMD_EOC,                       // end-of-command flag
 * };
 *
 * uint16_t reg_val;                    // storage for register value
 *
 * if (i2c_grasp (I2C1, cmd))           // lock i2c command engine
 * {
 *      i2cmd1.rxbuff = &reg_val;       // set receive buffer
 *      i2cmd1.eoc = eoc_callback;      // set end-of-command callback
 *      i2c_command (I2C1);             // go asynchronously
 * }
 * @endcode
 *
 * To set register values you can use the i2cmd1.txbuff field (similar to rxbuff),
 * together with the I2CMD_SEND_BUFF(n) command, which is similar to I2CMD_SEND
 * but will transmit next n bytes from txbuff.
 */

#include "cmsis.h"
#include <useful/useful.h>

// There are at least two different types of I2C peripherial
#if defined STM32F1 || defined STM32F2 || defined STM32F4
#  define I2C_TYPE_1
#elif defined STM32F0 || defined STM32F3
#  define I2C_TYPE_2
#else
#  error "Пожалуйста, задайте правильный тип I2C для контроллера"
#endif

/// Return the number of TIM peripherial associated with given hw feature
#define I2C_NUM(x)		JOIN2 (x, _I2C_NUM)
/// Get a pointer to TIM peripherial associated with given hw feature
#define I2C(x)			JOIN2 (I2C, I2C_NUM(x))

/**
 * Преобразовать 7-битный адрес устройства в байт адреса на шине
 * в режиме передачи данных.
 * @arg addr
 *      Адрес подчинённого устройства (0-127)
 */
#define I2C_TX7(addr)		(((addr) << 1)|0)

/**
 * Преобразовать 7-битный адрес устройства в байт адреса на шине
 * в режиме приёма данных.
 * @arg addr
 *      Адрес подчинённого устройства (0-127)
 */
#define I2C_RX7(addr)		(((addr) << 1)|1)

/**
 * Преобразовать 10-битный адрес устройства в два байта адреса на шине
 * в режиме передачи данных.
 * @arg addr
 *      Адрес подчинённого устройства (0-1023)
 */
#define I2C_TX10(addr)		(0xF0 | ((addr >> 7) & 6)), (addr & 0xff)

/**
 * Преобразовать 10-битный адрес устройства в два байта адреса на шине
 * в режиме приёма данных. Макрос генерирует только старший байт адреса,
 * младший байт при приёме не используется (см. спецификацию I2C).
 * @arg addr
 *      Адрес подчинённого устройства (0-1023)
 */
#define I2C_RX10(addr)		(0xF1 | ((addr >> 7) & 6))

/**
 * Проверить, является ли первый байт адреса первым байтом 10-битного адреса.
 */
#define I2C_ADDR_LEN(addr_hi)	((addr_hi & 0xF9) == (0xF0) ? 2 : 1)

/// I2C controller modes
typedef enum
{
    /// I2C mode, default
    i2cmI2C = 0,
    /// SMBus device
    i2cmSMBus_Device =
#if defined I2C_TYPE_1
        I2C_CR1_SMBUS,
#elif defined I2C_TYPE_2
        I2C_CR1_SMBDEN,
#endif
    /// SMBus host
    i2cmSMBus_Host =
#if defined I2C_TYPE_1
        I2C_CR1_SMBUS | I2C_CR1_SMBTYPE,
#elif defined I2C_TYPE_2
        I2C_CR1_SMBHEN,
#endif
    /// Do not use
    i2cmAll = i2cmSMBus_Device | i2cmSMBus_Host,
} i2c_mode_t;

/**
 * Initialize I2C interface according to parameters defined in hardware.h.
 * If you need the DMA-based i2c API, you must define the following macros in hardware.h:
 *  - I2C{1,2}_{TX,RX}_DMA_NUM - defines the DMA number for I2C TX/RX channels
 *  - I2C{1,2}_{TX,RX}_DMA_CHAN - defines the DMA channel for I2C TX/RX channels
 *  - I2C{1,2}_{TX,RX}_DMA_IRQ_PRIO - defines the DMA IRQ priority for I2C TX/RX channels
 * @arg i2c
 *      A pointer to I2C controller
 * @arg speed
 *      I2C clock speed in Hz
 * @arg mode
 *      I2C controller mode
 */
EXTERN_C void i2c_init (I2C_TypeDef *i2c, uint32_t speed, i2c_mode_t mode);

/**
 * I2C addressing mode
 */
typedef enum
{
    /// 7-bit own address
    i2c_Addr_7bit = 1,
    /// 10-bit own address
    i2c_Addr_10bit
} i2c_addr_mode_t;

/**
 * Set I2C controller address
 * @arg i2c
 *      A pointer to I2C controller
 * @arg addr
 *      The 7-bit or 10-bit address
 * @arg mode
 *      Address mode
 */
INLINE_ALWAYS void i2c_address_set (I2C_TypeDef *i2c, uint32_t addr, i2c_addr_mode_t mode)
{
    i2c->OAR1 = ((mode == i2c_Addr_7bit) ? (addr << 1) : addr) |
#if defined I2C_TYPE_1
        ((mode == i2c_Addr_7bit) ? 0 : I2C_OAR1_ADDMODE)
#elif defined I2C_TYPE_2
        ((mode == i2c_Addr_7bit) ? I2C_OAR1_OA1EN : (I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE))
#endif
        ;
}

/**
 * Set second I2C controller address
 * @arg i2c
 *      A pointer to I2C controller
 * @arg addr
 *      The second 7-bit device address. If larger than 127, second address is disabled
 */
INLINE_ALWAYS void i2c_address2_set (I2C_TypeDef *i2c, uint32_t addr)
{
    i2c->OAR2 = (addr > 127) ? 0 :
#if defined I2C_TYPE_1
        ((addr << 1) | I2C_OAR2_ENDUAL)
#elif defined I2C_TYPE_2
        ((addr << 1) | I2C_OAR2_OA2EN)
#endif
        ;
}

// Define DMA controller and channel numbers for I2C controllers
#ifdef I2C1_ENGINE
#  define I2C1_TX_DMA_IRQ_PRIO	128
#  define I2C1_RX_DMA_IRQ_PRIO	128
#  if defined STM32F1
#    ifndef I2C1_TX_DMA_NUM
#      define I2C1_TX_DMA_NUM	1
#      define I2C1_TX_DMA_CHAN	6
#    endif
#    ifndef I2C1_RX_DMA_NUM
#      define I2C1_RX_DMA_NUM	1
#      define I2C1_RX_DMA_CHAN	7
#    endif
#  else
#    error "Unknown I2C1 DMA channel assignments for your platform!"
#  endif
#endif

#ifdef I2C2_ENGINE
#  define I2C2_TX_DMA_IRQ_PRIO	128
#  define I2C2_RX_DMA_IRQ_PRIO	128
#  if defined STM32F1
#    ifndef I2C2_TX_DMA_NUM
#      define I2C2_TX_DMA_NUM	1
#      define I2C2_TX_DMA_CHAN	4
#    endif
#    ifndef I2C2_RX_DMA_NUM
#      define I2C2_RX_DMA_NUM	1
#      define I2C2_RX_DMA_CHAN	5
#    endif
#  else
#    error "Unknown I2C DMA channel assignments for your platform!"
#  endif
#endif

// If user wants the DMA engine for I2C1 and/or I2C2
#if defined I2C1_ENGINE || defined I2C2_ENGINE

/// The bitmask to separate opcode from command byte
#define I2CMD_OPCODE_MASK	0xE0
/// End-of-command opcode
#define I2CMD_OPCODE_EOC	0x00
/// 'START' condition opcode
#define I2CMD_OPCODE_START	0x20
/// Send bytes to bus, (number of bytes - 1) in bits 0-4 (1..32), bytes follows the command
#define I2CMD_OPCODE_SEND	0x40
/// Receive bytes from bus, (number of bytes - 1) in bits 0-4 (1..32)
#define I2CMD_OPCODE_RECV	0x60
/// Send bytes from txbuff to bus, (number of bytes - 1) in bits 0-4 and next byte (up to 2^13 bytes)
#define I2CMD_OPCODE_SEND_BUF	0x80
/// Receive bytes from bus to txbuff, (number of bytes - 1) in bits 0-4 and next byte
#define I2CMD_OPCODE_RECV_BUF	0xA0
/// Send 7-bit address in next byte
#define I2CMD_OPCODE_ADDR7	0xC0
/**
 * Send 10-bit adress, first byte in bits 0-4 (top bits are '1'
 * which incidentally equals to opcode :)), then second byte if bit 0
 * is not 1 (in receive mode second byte of address is not sent).
 */
#define I2CMD_OPCODE_ADDR10	0xE0

/// Opcode to set START condition on bus
#define I2CMD_START		(I2CMD_OPCODE_START)
/// Send n bytes (1..32) to bus, bytes to send follows the command
#define I2CMD_SEND(n)		(I2CMD_OPCODE_SEND | ((n) - 1) | ((((n) - 1) & (0xffffff00 | I2CMD_OPCODE_MASK)) << 3))
/// Receive n bytes (1..32) from bus to rxbuff
#define I2CMD_RECV(n)		(I2CMD_OPCODE_RECV | ((n) - 1) | ((((n) - 1) & (0xffffff00 | I2CMD_OPCODE_MASK)) << 3))
/// Send n bytes (1..8192) from txbuff to bus
#define I2CMD_SEND_BUF(n)	(I2CMD_OPCODE_SEND_BUF | (((n) - 1) >> 8)), ((((n) - 1) & 0xff) | (((n) - 1) & (0xffff0000 | (I2CMD_OPCODE_MASK << 8))))
/// Receive n bytes (1..8192) from bus to rxbuff
#define I2CMD_RECV_BUF(n)	(I2CMD_OPCODE_RECV_BUF | (((n) - 1) >> 8)), ((((n) - 1) & 0xff) | (((n) - 1) & (0xffff0000 | (I2CMD_OPCODE_MASK << 8))))
/// Send 7-bit destination address in transmit mode
#define I2CMD_TX7(addr)		(I2CMD_OPCODE_ADDR7), I2C_TX7 (addr)
/// Send 7-bit destination address in receive mode
#define I2CMD_RX7(addr)		(I2CMD_OPCODE_ADDR7), I2C_RX7 (addr)
/// Send 10-bit destination address in transmit mode
#define I2CMD_TX10(addr)	I2CMD_OPCODE_ADDR10 | I2C_TX10 (addr)
/// Send 10-bit destination address in receive mode
#define I2CMD_RX10(addr)	I2CMD_OPCODE_ADDR10 | I2C_RX10 (addr)
/// Finish command sequence
#define I2CMD_EOC		I2CMD_OPCODE_EOC

/// Host to bus endianess conversion for 16-bit ints
#define I2C_H2B16(x)		bswap16(x)
/// Host to bus endianess conversion for 32-bit ints
#define I2C_H2B32(x)		bswap32(x)
/// Fill a TX buffer with a 16-bit constant to send to device
#define I2CTX_16(x)		((x) >> 8), ((x) & 255)
/// Fill a TX buffer with a 32-bit constant to send to device
#define I2CTX_32(x)		((x) >> 24), ((x) >> 16), ((x) >> 8), ((x) & 255)

/// I2C command engine states
typedef enum
{
    /// Engine is idle
    i2cesIdle,
    /// Engine waits while the command structure is filled
    i2cesWarmup,
    /// Sending the START condition
    i2cesStart,
    /// Sending last byte of address
    i2cesAddr7,
    /// Sending first byte of a 10-bit address
    i2cesAddr10,
    /// Sending data with DMA
    i2cesSendDMA,
    /// Receiving data with DMA
    i2cesRecvDMA,
    /// Receiving data via IRQ
    i2cesRecvIRQ,
} i2cmd_state_t;

/**
 * Текущее состояние исполнителя команд i2c.
 * Имеется по одной статической переменной такого типа
 * для каждого используемого контроллера i2c.
 *
 * Поле state должно быть типа i2cmd_state_t,
 * но размер должен быть гарантировано 1 байт)
 */
typedef struct
{
    /// Текущее состояние алгоритма (i2cmd_state_t)
    uint8_t state;
    /// Текущий индекс внутри буффера cmd
    uint8_t cmd_idx;
    /// Текущий индекс внутри буффера txbuff
    uint8_t txbuff_idx;
    /// Текущее смещение внутри буффера rxbuff
    uint8_t rxbuff_idx;
    // Тайм-аут текущей операции (поддерживается клиентским кодом)
    uint16_t timeout;
    /// Последовательность команд на выполнение
    const uint8_t *cmd;
    /// A pointer to transmit buffer
    const void *txbuff;
    /// A pointer to receive buffer
    void *rxbuff;
    /**
     * A pointer to a user function called at end of command execution.
     * The function may start another I2C operation, or decide to stop the engine.
     * @arg error
     *     true if the request was aborted due to a error, false if it was
     *     successfuly completed.
     * @return
     *     return true to restart the request again, false to finish.
     *     This works independently of error argument.
     */
    void (*eoc) (bool error);
} i2c_state_t;

/**
 * Получить указатель на структуру i2c_state_t для указанного интерфейса
 * @arg i2c
 *      Указатель на контроллер i2c
 */
INLINE_ALWAYS volatile i2c_state_t *i2c_state (I2C_TypeDef *i2c)
{
    (void)i2c;

    if (0)
        return NULL;
#ifdef I2C1_ENGINE
    else if (i2c == I2C1)
        return &i2cmd1;
#endif
#ifdef I2C2_ENGINE
    else if (i2c == I2C2)
        return &i2cmd2;
#endif
    return NULL;
}

#endif // I2C1_ENGINE || I2C2_ENGINE

#ifdef I2C1_ENGINE
/// Буффер команд для первого контроллера I2C
EXTERN_C volatile i2c_state_t i2cmd1;
#endif

#ifdef I2C2_ENGINE
/// Буффер команд для второго контроллера I2C
EXTERN_C volatile i2c_state_t i2cmd2;
#endif

#define i2cmd(x)		JOIN2 (i2cmd, I2C_NUM(x))

#ifdef I2C1_ENGINE
/// @see i2c_grasp
EXTERN_C bool i2ce1_grasp (const void *cmd);
/// @see i2c_command
EXTERN_C void i2ce1_command ();
/// @see i2c_abort
EXTERN_C void i2ce1_abort ();
#endif

#ifdef I2C2_ENGINE
/// @see i2c_grasp
EXTERN_C bool i2ce2_grasp (const void *cmd);
/// @see i2c_command
EXTERN_C void i2ce2_command ();
/// @see i2c_abort
EXTERN_C void i2ce2_abort ();
#endif

/**
 * Заблокировать буффер i2cmd1 или i2cmd2. В дальнейшем вызывающий
 * должен заполнить структуру i2cmd и вызвать i2c_command().
 * В случае успешного выполнения, содержимое структуры i2cmd
 * почти полностью инициализируется нулями.
 * @arg i2c
 *      Указатель на контроллер i2c
 * @arg cmd
 *      Последовательность команд, которые выполняются асинхронно
 * @return
 *      true если буффер был свободен, и он заблокирован по требованию
 *      вызывающего кода. false, если буффер был занят.
 */
INLINE_ALWAYS bool i2c_grasp (I2C_TypeDef *i2c, const void *cmd)
{
    (void)i2c; (void)cmd;

    if (0)
        return false;
#ifdef I2C1_ENGINE
    else if (i2c == I2C1)
        return i2ce1_grasp (cmd);
#endif
#ifdef I2C2_ENGINE
    else if (i2c == I2C2)
        return i2ce2_grasp (cmd);
#endif
    return false;
}

/**
 * Выполнить команду из буффера i2cmd1 или i2cmd2. Если функция
 * вернула true, функция eoc() из структуры i2c_state_t будет
 * вызвана по окончании операции независимо от её результата.
 * @arg i2c
 *      Указатель на контроллер i2c
 */
INLINE_ALWAYS void i2c_command (I2C_TypeDef *i2c)
{
    (void)i2c;

    if (0)
        return;
#ifdef I2C1_ENGINE
    else if (i2c == I2C1)
        i2ce1_command ();
#endif
#ifdef I2C2_ENGINE
    else if (i2c == I2C2)
        i2ce2_command ();
#endif
}

/**
 * Мгновенно прервать выполнение команды.
 * @arg i2c
 *      Указатель на контроллер i2c
 */
INLINE_ALWAYS void i2c_abort (I2C_TypeDef *i2c)
{
    (void)i2c;

    if (0)
        return;
#ifdef I2C1_ENGINE
    else if (i2c == I2C1)
        i2ce1_abort ();
#endif
#ifdef I2C2_ENGINE
    else if (i2c == I2C2)
        i2ce2_abort ();
#endif
}

#endif // _STM32_I2C_H
