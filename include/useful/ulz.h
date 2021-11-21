/*
    uLZ compression library
    Copyright (C) 2021 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _ULZ_H
#define _ULZ_H

#include <stdint.h>
#include "useful.h"

/**
 * @file ulz.h
 *      These de/compression routines are designed to be fast, small footprint
 *      and effective. They won't give you best compression, but they're
 *      designed with a balanced approach. Also, due to simplicity, they
 *      will give you pretty fast decompression speeds.
 */

/**
 * Compress a block of data.
 *
 * @param idata A pointer to input data
 * @param isize The size of input data.
 * @param odata A pointer to output buffer (uninitialized)
 * @param osize A pointer to a variable that gets the size of output
 *      (compressed) data. On entry it contains the allocated size
 *      of the output buffer.
 * @return true if compressed data does not fit into output buffer.
 */
extern bool ulz_compress (const void *idata, unsigned isize,
                          void *odata, unsigned *osize);

#endif // _ULZ_H