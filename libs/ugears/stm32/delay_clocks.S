		.syntax unified
		.cpu cortex-m3
		.thumb

// extern void _delay_clocks (uint32_t clocks);
		.section .text,"ax",%progbits
		.global	_delay_clocks
		.type	_delay_clocks, %function

_delay_clocks:	movw	r1, #0xe010
		movt	r1, #0xe000
		ldr	r3, [r1, #8]		// SysTick->VAL
1:		ldr	r2, [r1, #8]
		subs	r3, r2
		bcc	3f
2:		subs	r0, r3
		mov	r3, r2
		bcs	1b
		bx	lr

3:		ldr	ip, [r1, #4]		// SysTick->LOAD
		add	r3, #1
		add	r3, ip
		b	2b
