/*
    STM32 flash programming library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears.h"
#include "usefun.h"

/**
    @file flash-storage.c

    Flash memory constant block storage library.

    Const blocks are placed sequentially one after another in flash memory,
    starting from first free address in flash memory (after user code, aligned
    up to nearest page boundary). When next block does not fit in remaining
    free space, it starts writing again from the lowest address. Every block
    contains a pointer to previous block, this way we can go backwards in chain
    up to the point when we find that previous block header is damaged.
*/

// flash support for stm32f2 and stm32f4 not implemented yet
#if !defined FLASH_TYPE_2

// the header of a flash const block as it is stored in flash mem
typedef struct
{
    // const block marker
    uint16_t marker;
#define MARKER 0xB10B
    // offset to prev block or 0xffff if this is first block in chain
    uint16_t prev;
    // const block checksum, size+6 bytes starting from next field
    uint16_t csum;
    // sequential block number
    uint16_t seqno;
    // block id
    uint16_t id;
    // block size
    uint16_t size;
    // const block data follows the header
    uint8_t data [0];
} flash_block_header_t;

// the last used address in flash memory
extern const void __fini_array_end;
// the address of the start of flash storage area
static uint32_t storage_start = (uint32_t)&__fini_array_end;
// the address of the end of flash storage area
static uint32_t storage_end = STM32_FLASH_ORIGIN + STM32_FLASH_SIZE;

typedef struct
{
    // offset of last block
    flash_block_header_t *last;
    // last block number in sequence
    uint16_t seqno;
} flash_storage_state_t;

static flash_storage_state_t fss;

// align data size to nearest even value
#define aligned_size(x)		(((x) + 1) & ~1)

void flash_storage (const void *start, uint16_t size)
{
    storage_start = (uint32_t)start;
    storage_end = storage_start + 1 + size;
}

static void flash_init_prepare ()
{
    // round storage start up to nearest page boundary
    unsigned fps = flash_page_size (storage_start);
    storage_start = (storage_start + (fps - 1)) & ~(fps - 1);
    // round storage end down to nearest page boundary plus page size
    storage_end = ((storage_end - fps) & ~(fps - 1)) + fps;
    // limit storage size to 64k
    if (storage_end - storage_start > 0x10000)
        storage_end = storage_end - 0x10000;

    memclr (&fss, sizeof (fss));
}

bool flash_format ()
{
    flash_init_prepare ();

    // erase the marker of the first header,
    // to prevent flash_init() from finding anything
    flash_write16 ((void *)storage_start, 0);

    return true;
}

static bool check_header (flash_block_header_t *fbh)
{
    if (fbh->marker != MARKER)
        return false;

    if (fbh->size > storage_end - (uint32_t)(fbh + 1))
        return false;

    if ((fbh->prev != 0xffff) &&
        (fbh->prev >= storage_end - storage_start))
        return false;

    return true;
}

void flash_init ()
{
    flash_init_prepare ();

    // scan all const blocks in storage area, and find the one with the head & tail
    flash_block_header_t *fbh = (flash_block_header_t *)storage_start;
    if (check_header (fbh))
    {
        // scan forward till we find the last written block
        for (;;)
        {
            flash_block_header_t *next = (flash_block_header_t *)((uint32_t)(fbh + 1) +
                aligned_size (fbh->size));
            if (!check_header (next))
                break;
            if (next->seqno != ((fbh->seqno + 1) & 0xffff))
                break;

            fbh = next;
        }

        fss.last = fbh;
        fss.seqno = fbh->seqno;
    }
}

bool flash_save (uint16_t id, const void *data, uint16_t size)
{
    flash_block_header_t *fbh;

    if (!fss.last)
        goto fromstart;

    fbh = (flash_block_header_t *)((uint32_t)(fss.last + 1) + aligned_size (fss.last->size));

    uint32_t end = (uint32_t)fbh + sizeof (flash_block_header_t) + size;
    if (end > storage_end)
    {
fromstart:
        end = storage_start + sizeof (flash_block_header_t) + size;
        fbh = (flash_block_header_t *)storage_start;
    }

    // Check how many pages we're going to erase
    uint32_t addr = (uint32_t)fbh;
    while (addr < end)
    {
        if ((addr & (flash_page_size (addr) - 1)) == 0)
        {
            // erase the page
            if (!flash_erase ((void *)addr))
                // flash is weared out?
                return false;
        }

        // move to next page
        addr = (addr & ~(flash_page_size (addr) - 1)) + flash_page_size (addr);
    }

    // now, fill the header
    static flash_block_header_t newfbh;
    newfbh.marker = MARKER;
    newfbh.prev = fss.last ? (uint32_t)fss.last - storage_start : 0xffff;
    newfbh.seqno = ++fss.seqno;
    newfbh.id = id;
    newfbh.size = size;
    newfbh.csum = 
        ip_crc_fin (ip_crc_block (ip_crc_block (0,
            &newfbh.seqno, sizeof (uint16_t) * 3),
            data, size));

    if (!flash_write (fbh, &newfbh, sizeof (newfbh)) ||
        !flash_write (fbh + 1, data, aligned_size (size)))
        return false;

    fss.last = fbh;

    return true;
}

const void *flash_get (uint16_t id, uint16_t *size)
{
    flash_block_header_t *fbh = fss.last;
    while (fbh)
    {
        if (fbh->id == id)
        {
            if (fbh->csum == ip_crc (&fbh->seqno, fbh->size + sizeof (uint16_t) * 3))
            {
                *size = fbh->size;
                return fbh->data;
            }
        }

        // move backwards in chain till we find a valid block with requested id
        if (fbh->prev == 0xffff)
            break;
        flash_block_header_t *prev = (flash_block_header_t *)(storage_start + fbh->prev);
        if (!check_header (prev))
            break;
        if (prev->seqno != ((fbh->seqno - 1) & 0xffff))
            break;

        fbh = prev;
    }

    return 0;
}

const void *flash_enum (uint32_t idx, uint16_t *id, uint16_t *size)
{
    flash_block_header_t *fbh = fss.last;
    if (!fbh)
        return 0;

    while (idx)
    {
        // move backwards in chain
        if (fbh->prev == 0xffff)
            return 0;
        flash_block_header_t *prev = (flash_block_header_t *)(storage_start + fbh->prev);
        if (!check_header (prev))
            return 0;
        if (prev->seqno != ((fbh->seqno - 1) & 0xffff))
            return 0;

        fbh = prev;
        idx--;
    }

    if (fbh->csum != ip_crc (&fbh->seqno, fbh->size + sizeof (uint16_t) * 3))
        return 0;

    *id = fbh->id;
    *size = fbh->size;
    return fbh->data;
}

#endif // FLASH_TYPE_2
