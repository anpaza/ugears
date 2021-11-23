/*
    STM32 flash programming library
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "ugears/ugears.h"
#include <useful/usefun.h>
#include <useful/clike.h>

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

// the header of a flash const block as it is stored in flash mem
typedef struct
{
    // const block marker
    uint16_t marker;
#define MARKER 0xB10B
    // const block checksum, size+6 bytes starting from next field
    uint16_t csum;

    // ... following fields are protected by csum ...
#define CSUM_HEADER_SIZE    (sizeof (flash_block_header_t) - OFFSETOF (flash_block_header_t, seqno))

    // sequential block number
    uint16_t seqno;
    // block id
    uint16_t id;
    // offset to prev block or FLASH_OFF_MAX if this is first block in chain
    flash_off_t prev;
    // block size
    flash_off_t size;
    // const block data follows the header
    uint8_t data [0];
} flash_block_header_t;

// start of const data initializers in flash memory
extern const void _sidata;
// start of initialized data in RAM
extern const void _sdata;
// end of initialized data in RAM
extern const void _edata;

// the address of the start of flash storage area
static uintptr_t storage_start = 0;
// next address after last used flash page
static uintptr_t storage_end;

typedef struct
{
    // offset of last block
    flash_block_header_t *last;
    // last block number in sequence
    uint16_t seqno;
} flash_storage_state_t;

static flash_storage_state_t g_fss;

// align data size up to nearest even value
#define align_size_up2(x)	(((x) + 1) & ~1)

bool flash_storage (uintptr_t start, unsigned size)
{
    storage_start = start;
    storage_end = storage_start + size;

    uintptr_t code_end = (uintptr_t)&_sidata + (uintptr_t)&_edata - (uintptr_t)&_sdata;
    if ((storage_start < code_end) && (storage_end >= code_end))
        storage_start = code_end;

    // align flash storage end down to nearest flash page boundary
    uintptr_t ps = flash_page_size (storage_end - 1);
    storage_end = ((storage_end - ps) & ~(ps - 1)) + ps;

    // align flash storage start up to nearest flash page boundary
    ps = flash_page_size (storage_start);
    storage_start = (storage_start + ps - 1) & ~(ps - 1);

    // need at least two flash memory pages to function properly
    if (storage_start + flash_page_size (storage_start) >= storage_end)
        return false;

    return true;
}

static bool flash_init_prepare ()
{
    if (storage_start == 0)
    {
        // if user did not set flash storage area, use all free flash memory pages
        if (!flash_storage (STM32_FLASH_ORIGIN, STM32_FLASH_SIZE))
            return false;
    }

    // need at least two flash memory pages to function properly
    if (storage_start + flash_page_size (storage_start) >= storage_end)
        return false;

    memclr (&g_fss, sizeof (g_fss));
    return true;
}

bool flash_format ()
{
    if (!flash_init_prepare ())
        return false;

    // erase the marker of the first header,
    // to prevent flash_init() from finding anything
    flash_begin (FLASH_CR_PSIZE_x16);
    bool rc = flash_write16 ((void *)storage_start, 0);
    flash_end ();

    return rc;
}

static bool check_header (flash_block_header_t *fbh)
{
    if (fbh->marker != MARKER)
        return false;

    if (fbh->size > storage_end - (uintptr_t)(fbh + 1))
        return false;

    if ((fbh->prev != FLASH_OFF_MAX) &&
        (fbh->prev >= storage_end - storage_start))
        return false;

    return true;
}

bool flash_init (bool format)
{
    if (!flash_init_prepare ())
        return false;

    // scan all const blocks in storage area, and find the one with the head & tail
    flash_block_header_t *fbh = (flash_block_header_t *)storage_start;
    if (check_header (fbh))
    {
        // scan forward till we find the last written block
        for (;;)
        {
            flash_block_header_t *next = (flash_block_header_t *)((uintptr_t)(fbh + 1) +
                align_size_up2 (fbh->size));
            if (!check_header (next))
                break;
            if (next->seqno != ((fbh->seqno + 1) & 0xffff))
                break;

            fbh = next;
        }

        g_fss.last = fbh;
        g_fss.seqno = fbh->seqno;
    }
    // if asked to format if not formatted, do it
    else if (format && (fbh->marker != 0))
        return flash_format ();

    return true;
}

bool flash_save (uint16_t id, const void *data, flash_off_t size)
{
    flash_block_header_t *fbh;
    uintptr_t end;

    if (!g_fss.last)
        goto fromstart;

    fbh = (flash_block_header_t *)((uintptr_t)(g_fss.last + 1) + align_size_up2 (g_fss.last->size));
    end = (uintptr_t)fbh + sizeof (flash_block_header_t) + size;

    if (end > storage_end)
    {
fromstart:
        fbh = (flash_block_header_t *)storage_start;
        end = storage_start + sizeof (flash_block_header_t) + size;
    }

    // We're making sure all writes are aligned by 2
    flash_begin (FLASH_CR_PSIZE_x16);

    // Check how many pages we're going to erase
    uintptr_t addr = (uintptr_t)fbh;
    while (addr < end)
    {
        uint32_t fps = flash_page_size (addr);

        if ((addr & (fps - 1)) == 0)
        {
            // erase the page
            if (!flash_erase ((void *)addr))
            {
                flash_end ();
                // flash is weared out?
                return false;
            }
        }

        // move to next page
        addr = (addr & ~(fps - 1)) + fps;
    }

    // now, fill the header
    static flash_block_header_t newfbh;
    newfbh.marker = MARKER;
    newfbh.prev = g_fss.last ? (uintptr_t)g_fss.last - storage_start : FLASH_OFF_MAX;
    newfbh.seqno = ++g_fss.seqno;
    newfbh.id = id;
    newfbh.size = size;
    newfbh.csum = 
        ip_crc_fin (ip_crc_block (ip_crc_block (0,
            &newfbh.seqno, CSUM_HEADER_SIZE),
            data, size));

    bool rc = false;
    if (flash_write (fbh, &newfbh, sizeof (newfbh)) &&
        flash_write (fbh + 1, data, align_size_up2 (size)))
    {
        g_fss.last = fbh;
        rc = true;
    }

    flash_end ();

    return rc;
}

const void *flash_get (uint16_t id, flash_off_t *size)
{
    flash_block_header_t *fbh = g_fss.last;
    while (fbh)
    {
        if (fbh->id == id)
        {
            if (fbh->csum == ip_crc (&fbh->seqno, CSUM_HEADER_SIZE + fbh->size))
            {
                *size = fbh->size;
                return fbh->data;
            }
        }

        // move backwards in chain till we find a valid block with requested id
        if (fbh->prev == FLASH_OFF_MAX)
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

const void *flash_enum (uint32_t idx, uint16_t *id, flash_off_t *size)
{
    flash_block_header_t *fbh = g_fss.last;
    if (!fbh)
        return 0;

    while (idx)
    {
        // move backwards in chain
        if (fbh->prev == FLASH_OFF_MAX)
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
