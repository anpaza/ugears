/*
    Assembly implementation of memcpy for ARM/Thumb
    Copyright (C) 2014 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "useful/clike-defs.h"

	.syntax unified
	.cpu cortex-m3
	.thumb

// extern void _memcpy (void *dest, const void *src, unsigned len);
	.section .text,"ax",%progbits
	.global	CLIKE_P (memcpy)
	.type	CLIKE_P (memcpy), %function

CLIKE_P (memcpy):
	cbz	r2, 6f		// quit if len=0

// Check if src and dest can be aligned to 4-byte boundary
	eors	r3, r0, r1
	beq	6f		// quit of src==dest
	ands	r3, #3
	bne	5f		// Do unaligned, byte-by-byte, copy

// Align target address to word boundary first
1:	ands	r3, r0, #3
	beq	2f
	ldrb	r3, [r1]
	add	r1, #1
	strb	r3, [r0]
	add	r0, #1
	subs	r2, #1
	bne	1b

// Copy by words
2:	lsrs	r12, r2, #2
	beq	4f
3:	ldmia	r1!, {r3}
	stmia	r0!, {r3}
	subs	r12, #1
	bne	3b

4:	ands	r2, #3
	beq	6f

5:	ldrb	r3, [r1]
	add	r1, #1
	strb	r3, [r0]
	add	r0, #1
	subs	r2, #1
	bne	5b

6:	bx	lr
