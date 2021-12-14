/*
    Assembly implementation of memset and memclr for ARM/Thumb
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike-defs.h"

	.syntax unified
	.cpu cortex-m3
	.thumb

// void memclr (void *dest, unsigned len)
	.section .text,"ax",%progbits
	.global	CLIKE_P (memclr)
	.type	CLIKE_P (memclr), %function
CLIKE_P (memclr):
	mov	r2, r1
	mov	r1, #0

// void _memset (void *dest, char c, unsigned len)
	.global	CLIKE_P (memset)
	.type	CLIKE_P (memset), %function

CLIKE_P (memset):
// Align target address to word boundary first
1:	cbz	r2, 4f		// quit if len=0
	tst	r0, #3
	itttt	ne
	strbne	r1, [r0]
	addne	r0, #1
	subne	r2, #1
	bne	1b

// Fill with words
	movs	r3, r2, lsr #2	// r3 = number of words
	beq	3f

	and	r1, r1, #0xff
	orr	r1, r1, r1, lsl #8
	orr	r1, r1, r1, lsl #16

2:	stmia	r0!, {r1}
	subs	r3, #1
	bne	2b

  	and	r2, #3		// r2 = remaining bytes
3:	cbz	r2, 4f		// quit if len=0
	strb	r1, [r0]
	add	r0, #1
	subs	r2, #1
	b	3b

4:	bx	lr
