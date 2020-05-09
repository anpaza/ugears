/*
    A library of generally useful functions
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "usefun.h"

/*
 * Xorshift RNG by George Marsaglia.
 * https://en.wikipedia.org/wiki/Xorshift
 */

void xs_init (xs_rng_t xsr, uint32_t seed)
{
    memset (xsr, 0, sizeof (xs_rng_t));
    xs_seed (xsr, seed);
}

void xs_seed (xs_rng_t xsr, uint32_t seed)
{
    for (int i = 0; i < 3; i++)
    {
        xsr [4] += seed;
        xsr [3] ^= xs_rand (xsr);
    }
}

uint32_t xs_rand (xs_rng_t xsr)
{
    uint32_t s, t;

    /* Algorithm "xorwow" from p. 5 of Marsaglia, "Xorshift RNGs" */

    // additional xor-constant added so that it doesn't lock at (0,0,0,0)
    t = xsr [3] ^ 0xdeadface;
    t ^= t >> 2;
    t ^= t << 1;
    xsr [3] = xsr [2];
    xsr [2] = xsr [1];
    s = xsr [1] = xsr [0];
    t ^= s;
    t ^= s << 4;
    xsr [0] = t;

    return t + (xsr [4] += 362437);
}

static xs_rng_t rng;

void srand (unsigned seed)
{
    xs_init (rng, seed);
}

unsigned rand ()
{
    return xs_rand (rng);
}
