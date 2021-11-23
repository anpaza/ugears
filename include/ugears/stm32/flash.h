/*
    STM32 GPIO helpers library
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _STM32_FLASH_H
#define _STM32_FLASH_H

/**
 * @file flash.h
 *      Flash memory programming helper routines.
 *
 * WARNING! If you use the FLASH library, you must initialize the SysTick timer.
 * If you don't care, just call systick_config(0xffffff) at the start of
 * your program.
 *
 * For F2 and F4 families you can define a macro named FLASH_PSIZE in your
 * HARDWARE_H file (see section "Program/erase parallelism" in datasheet)
 * to one of FLASH_CR_PSIZE_xxx constants defined below. If not defined,
 * a safe guess of FLASH_CR_PSIZE_x16 is used. This is the minimal size
 * of a data unit that can be written into flash memory, e.g. if you use
 * PSIZE_x16 you can write 16-bit, 32-bit, 64-bit data but not 8-bit.
 * The flash_write() routine will choose the appropiate data size for
 * transfer.
 *
 * Also library provides a handy layer on top of the low-level flash code
 * which can be used to easily store program constants in flash memory with
 * the following features:
 *
 * * Flash wearing minimized by using all unused flash memory pages in a
 *   round-robin sequence.
 * * Flash wearing minimized by using multiple writes to same page after
 *   erasing. If your constants are small enough, they will be stored
 *   multiple times into the same page (with increasing addresses),
 *   and erase will be executed only once at the beginning.
 * * The library will allow you to easily find the latest copy of stored
 *   constants.
 * * You can store different sets of constants, each set identified by a
 *   unique id value.
 * * Constants set integrity is automatically ensured and checked. If the
 *   latest version of a constants set is broken, library will try to find
 *   a earlier version of same constants set.
 */

#include <useful/useful.h>
#include "cmsis.h"
#include <limits.h>

#if defined STM32F0 || defined STM32F1 || defined STM32F3
#  define FLASH_TYPE_1
#elif defined STM32F2 || defined STM32F4
#  define FLASH_TYPE_2
#endif

#ifndef FLASH_FKEY1
#define FLASH_FKEY1			((uint32_t)0x45670123)        /*!< Flash program erase key1 */
#endif
#ifndef FLASH_FKEY2
#define FLASH_FKEY2			((uint32_t)0xCDEF89AB)        /*!< Flash program erase key2 */
#endif
#ifndef FLASH_RDPRT
#define FLASH_RDPRT			((uint32_t)0x00A5)
#endif

#if defined STM32F070xB || defined STM32F030xC || \
    defined STM32F105xC || defined STM32F107xC || \
    defined SRM32F3
INLINE_ALWAYS uint32_t flash_page_size (uint32_t address)
{
    (void)address;
    return 2048;
}

#elif defined STM32F0
INLINE_ALWAYS uint32_t flash_page_size (uint32_t address)
{
    (void)address;
    return (STM32_RAM_SIZE <= 64*1024) ? 1024 : 2048;
}

#elif defined STM32F1
INLINE_ALWAYS uint32_t flash_page_size (uint32_t address)
{
    (void)address;
    return (STM32_RAM_SIZE <= 128*1024) ? 1024 : 2048;
}

#elif defined STM32F2 || defined STM32F4
INLINE_ALWAYS uint32_t flash_page_size (uint32_t address)
{
#ifdef FLASH_OPTCR_DB1M
    // dual flash banks on STM32F42x & STM32F43x use identical layout
    if (address >= 0x08100000)
        address -= 0x00100000;
    // in dual bank mode identical layout on pages 0-7 and 12-19
    else if ((FLASH->OPTCR & FLASH_OPTCR_DB1M) &&
        (address >= 0x08080000))
        address -= 0x00080000;
#endif
    return (address >= 0x08020000) ? 128*1024 :
           (address >= 0x08010000) ? 64*1024 :
           16*1024;
}

#else
#  error "Page size for your MCU is not known"
#endif

#if STM32_FLASH_SIZE > 64 * 1024
/// Offset relative to start of flash memory
typedef uint32_t flash_off_t;
#define FLASH_OFF_MAX			UINT32_MAX
#elif defined STM32_FLASH_SIZE
/// Offset relative to start of flash memory
typedef uint16_t flash_off_t;
#define FLASH_OFF_MAX			UINT16_MAX
#else
#  error "Flash memory size not defined!"
#endif

#if defined FLASH_TYPE_1

// Constants for compatibility with FLASH_TYPE_2
#define FLASH_CR_PSIZE_x64		(0)
#define FLASH_CR_PSIZE_x32		(0)
#define FLASH_CR_PSIZE_x16		(0)
#define FLASH_CR_PSIZE_x8		(0)

/**
 * Unlock the flash memory for writing.
 * This function will start the HSI clock, if it is not enabled.
 *
 * The argument is ignored for compatibility with FLASH_TYPE_2.
 */
EXTERN_C bool flash_begin (uint32_t);

#elif defined FLASH_TYPE_2

// define FLASH_PSIZE in your HARDWARE_H file to one of the constants below

// 11: 64 bits, Voltage range 2.7 - 3.6 V with External V PP
#define FLASH_CR_PSIZE_x64		(FLASH_CR_PSIZE_0 | FLASH_CR_PSIZE_1)
// 10: 32 bits, Voltage range 2.7 - 3.6 V
#define FLASH_CR_PSIZE_x32		(FLASH_CR_PSIZE_1)
// 01: 16 bits, Voltage range 2.4 - 2.7 V, 2.1 - 2.4 V
#define FLASH_CR_PSIZE_x16		(FLASH_CR_PSIZE_0)
// 00: 8 bits, Voltage range 1.8 V - 2.1 V
#define FLASH_CR_PSIZE_x8		(0)

/**
 * Unlock the flash memory for writing.
 * You must specify the "parallelism size", which limits the size of acesses
 * to flash memory during programming. The flash_write* routines will
 * try to adapt to the specified parallelism (e.g you may write a 32-bit word
 * as two 16-bit words, but you cannot write a 8-bit value if you specified
 * 16-bit access).
 *
 * @arg psize One of FLASH_CR_PSIZE_xxx constants.
 * @return false if psize exceeds FLASH_PSIZE defined in HARDWARE_H.
 */
EXTERN_C bool flash_begin (uint32_t psize);

#endif

/**
 * Lock the flash memory writing.
 * On F0, F1, F3 series this function will stop the HSI clock
 * if it was not running prior to calling flash_begin().
 */
EXTERN_C void flash_end ();

/**
 * Erase a single page.
 * @arg addr Target page address, must be on page boundary.
 * @return false if something went wrong, true if page successfuly cleared.
 */
EXTERN_C bool flash_erase (void *addr);

/**
 * Copy data from source buffer to target flash memory.
 * Transfer is automatically done in appropiate units (16 bits a time on
 * F0,F1,F3 devices, 8 to 64 bits depending on psize on F2,F4).
 *
 * Function will fail if it cannot break data transfer into respective units
 * (e.g. size cannot be split into transfer-unit-aligned quantities).
 *
 * Also make sure source and destination addresses are aligned to transfer
 * unit size.
 *
 * @arg addr Target address in flash memory
 * @arg src The address of data to write into flash
 * @arg size Source data size in bytes. This must be a multiple of 2!
 * @return false if something went wrong, true if flash memory was successfuly programmed
 */
EXTERN_C bool flash_write (void *addr, const void *src, uint32_t size);

/**
 * Write a single 16-bit data value to flash memory.
 * @arg addr Target address in flash memory
 * @arg data The data value to write
 * @return false if something went wrong, true if flash memory was successfuly programmed
 */
EXTERN_C bool flash_write16 (void *addr, uint16_t data);


/**
 * @addtogroup flash_storage Recyclable flash memory storage
 * The following group of functions provide a way to store blocks of data
 * into flash memory. The pages are incrementally cyclically used, thus
 * providing even wear-leveling.
 * @{
 */

/**
 * Set up flash memory storage area. By default, all unused space after
 * user code and up to the end of flash memory is used.
 *
 * After this function is called you MUST call flash_init() to initialize
 * the internal variables related to flash storage.
 * @arg start Address of the start of flash memory reserved for const storage.
 *      The address will be rounded up to nearest page boundary.
 * @arg size Flash storage size.
 * @return true if flash area meets minimum library requirements
 */
EXTERN_C bool flash_storage (uintptr_t start, unsigned size);

/**
 * Erase the flash storage area.
 * You don't have to call flash_init() after this function.
 *
 * You don't have to call flash_begin() before and flash_end() after
 * this function.
 * @return true if everything went fine, false if flash failed to erase.
 */
EXTERN_C bool flash_format ();

/**
 * Initialize the internal variables used to track flash storage state.
 * You must call this function before using other functions related
 * to flash storage.
 *
 * You don't have to call flash_begin() before and flash_end() after
 * this function.
 * @param format Format flash storage if storage is uninitialized
 * @return false if there's not enough space in flash memory
 */
EXTERN_C bool flash_init (bool format);

/**
 * Save a block of constants into the flash memory, using unused flash memory
 * pages in a round-robin sequence to reduce flash memory wearing.
 *
 * This will overwrite a number of oldest available constant blocks, so user
 * must take care not to lose useful data when saving a new block if you're
 * using blocks with different ids. For example, if you save a block with
 * id=0x1234 and then saving a block with id=0x5678 many times, at some point
 * the newly saved block will overwrite the old block with id=0x1234.
 *
 * There are several approaches to avoid losing data:
 * * Save all blocks every time, or
 * * After saving a block, check that other blocks are still readable
 *   (by using flash_get()), if they are not, save them again from RAM.
 *
 * You don't have to call flash_begin() before and flash_end() after
 * this function.
 * @arg id Constant block type identifier.
 *      The identifier of this block should be later used to find the latest
 *      version of saved block in flash memory.
 * @arg data A pointer to data. Pointer should be aligned at least to 2 bytes.
 * @arg size Block size in bytes
 * @return false if something went wrong, true if constants were successfuly
 *      saved.
 */
EXTERN_C bool flash_save (uint16_t id, const void *data, flash_off_t size);

/**
 * Read the latest version of a const block with given id and read it into
 * user's buffer.
 * @arg id The identifier of the constants block to find.
 * @arg size On success function stores found block size here
 * @return On success returns a pointer to saved data, otherwise NULL.
 */
EXTERN_C const void *flash_get (uint16_t id, flash_off_t *size);

/**
 * Get Nth saved block from flash memory, counting from last one.
 * This function can be used to enumerate all const blocks stored in
 * flash memory.
 * @arg idx Block number, 0 is last, 1 is the one prior to last and so on.
 * @arg id On success function stores found block ID here
 * @arg size On success function stores found block size here
 * @return On success returns a pointer to saved data, otherwise NULL.
 */
EXTERN_C const void *flash_enum (uint32_t idx, uint16_t *id, flash_off_t *size);

/**
 * @}
 */

#endif // _STM32_FLASH_H
