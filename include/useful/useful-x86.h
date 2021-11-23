/*
    Useful small inline functions coded in x86 assembler
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _USEFUL_X86_H
#define _USEFUL_X86_H

#define _USEFUL_ROL32
INLINE_ALWAYS uint32_t rol32 (uint32_t s, uint8_t count)
{ uint32_t ret; asm ("roll %%cl,%1" : "=r" (ret) : "0" (s), "c" (count)); return ret; }

#define _USEFUL_ROR32
INLINE_ALWAYS uint32_t ror32 (uint32_t s, uint8_t count)
{ uint32_t ret; asm ("rorl %%cl,%1" : "=r" (ret) : "0" (s), "c" (count)); return ret; }

#define _USEFUL_ROL8
INLINE_ALWAYS uint8_t rol8 (uint8_t s, uint8_t count)
{ uint8_t ret; asm ("rolb %%cl,%1" : "=r" (ret) : "0" (s), "c" (count)); return ret; }

#define _USEFUL_ROR8
INLINE_ALWAYS uint8_t ror8 (uint8_t s, uint8_t count)
{ uint8_t ret; asm ("rorb %%cl,%1" : "=r" (ret) : "0" (s), "c" (count)); return ret; }

#define _USEFUL_BSWAP16
INLINE_ALWAYS uint16_t bswap16 (uint16_t x)
{ uint16_t ret; asm ("xchgb %%al,%%ah" : "=a" (ret) : "0" (x)); return ret; }

#define _USEFUL_BSWAP32
INLINE_ALWAYS uint32_t bswap32 (uint32_t x)
{ uint32_t ret; asm ("bswap %1" : "=r" (ret) : "0" (x)); return ret; }

#endif // _USEFUL_X86_H
