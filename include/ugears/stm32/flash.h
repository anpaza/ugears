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
 * If you don't care, just call systick_config(0xffffff) at the start of your program.
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
INLINE_ALWAYS uint32_t flash_page_size(uint32_t address)
{
    (void)address;
    return 2048;
}

#elif defined STM32F0
INLINE_ALWAYS uint32_t flash_page_size(uint32_t address)
{
    (void)address;
    return (STM32_RAM_SIZE <= 64*1024) ? 1024 : 2048;
}

#elif defined STM32F1
INLINE_ALWAYS uint32_t flash_page_size(uint32_t address)
{
    (void)address;
    return (STM32_RAM_SIZE <= 128*1024) ? 1024 : 2048;
}

#elif defined STM32F2
// Return the flash memory page size at certain address
INLINE_ALWAYS uint32_t flash_page_size(uint32_t address)
{
    return (address >= 0x08020000) ? 128*1024 :
           (address >= 0x08010000) ? 64*1024 :
           64*1024;
}

#elif defined STM32F4
INLINE_ALWAYS uint32_t flash_page_size(uint32_t address)
{
    (void)address;
    return 128*1024;
}

#else
#  error "Page size for your MCU is not known"
#endif

/**
 * Unlock the flash memory for writing.
 * This function will start the HSI clock, if it is not enabled.
 */
EXTERN_C void flash_begin ();

/**
 * Lock the flash memory writing.
 * This function will stop the HSI clock if it was not running
 * prior to calling flash_begin().
 */
EXTERN_C void flash_end ();

/**
 * Erase a single page.
 * @arg addr
 *      Target page address, must be a multiple of FLASH_PAGE_SIZE.
 * @return
 *      false if something went wrong, true if page was successfuly cleared.
 */
EXTERN_C bool flash_erase (void *addr);

/**
 * Copy a number of bytes from source buffer to target flash memory.
 * @arg addr
 *      Target address in flash memory
 * @arg src
 *      The address of data to write into flash
 * @arg size
 *      Source data size in bytes. This must be a multiple of 2!
 * @return
 *      false if something went wrong, true if flash memory was successfuly programmed
 */
EXTERN_C bool flash_write (void *addr, const void *src, uint32_t size);

/**
 * Write a single 16-bit data value to flash memory.
 * @arg addr
 *      Target address in flash memory
 * @arg data
 *      The data value to write
 * @return
 *      false if something went wrong, true if flash memory was successfuly programmed
 */
EXTERN_C bool flash_write16 (void *addr, uint16_t data);

/**
 * Set up flash memory storage area. By default, all unused space after
 * user code and up to the end of flash memory is used, but not exceeding
 * 64k in size, which is the maximum possible flash storage size.
 *
 * After this function is called you MUST call flash_init() to initialize
 * the internal variables related to flash storage.
 * @arg start
 *      A pointer to the start of flash memory reserved for const storage.
 *      The address will be rounded up to nearest page boundary.
 * @arg size
 *      Flash storage size minus one. The maximum flash storage size is 64k.
 */
EXTERN_C void flash_storage (const void *start, uint16_t size);

/**
 * Erase the flash storage area.
 * You don't have to call flash_init() after this function.
 *
 * This function must be called inside a flash_begin()/flash_end() context.
 * @return
 *      true if everything went fine, false if flash failed to erase.
 */
EXTERN_C bool flash_format ();

/**
 * Initialize the internal variables used to track flash storage state.
 * You must call this function before using other functions related
 * to flash storage.
 */
EXTERN_C void flash_init ();

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
 * This function must be called between flash_begin() and flash_end().
 * @arg id
 *      Constant block type identifier.
 *      The identifier of this block should be later used to find the latest
 *      version of saved block in flash memory.
 * @arg data
 *      A pointer to data. Pointer should be aligned at least to 2 bytes.
 * @arg size
 *      Block size in bytes
 * @return
 *     false if something went wrong, true if constants were successfuly saved.
 */
EXTERN_C bool flash_save (uint16_t id, const void *data, uint16_t size);

/**
 * Read the latest version of a const block with given id and read it into
 * user's buffer.
 * @arg id
 *      The identifier of the constants block to find.
 * @arg size
 *      On success function stores found block size here
 * @return
 *      On success returns a pointer to saved data, otherwise NULL.
 */
EXTERN_C const void *flash_get (uint16_t id, uint16_t *size);

/**
 * Get Nth saved block from flash memory, counting from last one.
 * This function can be used to enumerate all const blocks stored in
 * flash memory.
 * @arg idx
 *      Block number, 0 is last, 1 is the one prior to last and so on.
 * @arg id
 *      On success function stores found block ID here
 * @arg size
 *      On success function stores found block size here
 * @return
 *      On success returns a pointer to saved data, otherwise NULL.
 */
EXTERN_C const void *flash_enum (uint32_t idx, uint16_t *id, uint16_t *size);

#endif // _STM32_FLASH_H
