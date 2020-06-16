/*
    A library of generally useful functions
    Copyright (C) 2018 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

/*
    This file implements printf and getch using ARM semihosting
    functions (e.g. st-util --semihosting).

    Note that you will be unable to run programs compiled with
    semihosting without the JTAG debugger connected.
*/

#include <stddef.h>
#include "useful/useful.h"
#include "useful/printf.h"
#undef putc

#define SYS_OPEN		0x01
#define SYS_CLOSE		0x02
#define SYS_WRITEC		0x03
#define SYS_WRITE0		0x04
#define SYS_WRITE		0x05
#define SYS_READ		0x06
#define SYS_READC		0x07
#define SYS_ISERROR		0x08
#define SYS_ISTTY		0x09
#define SYS_SEEK		0x0A
#define SYS_FLEN		0x0C
#define SYS_TMPNAM		0x0D
#define SYS_REMOVE		0x0E
#define SYS_RENAME		0x0F
#define SYS_CLOCK		0x10
#define SYS_TIME		0x11
#define SYS_SYSTEM		0x12
#define SYS_ERRNO		0x13
#define SYS_GET_CMDLINE		0x15
#define SYS_HEAPINFO		0x16
#define SYS_EXIT		0x18
#define SYS_EXIT_EXTENDED	0x20
#define SYS_ELAPSED		0x30
#define SYS_TICKFREQ		0x31

#ifdef __thumb__
#define SH_TRAP_INSN		"bkpt	0xAB"
#else
#define SH_TRAP_INSN		"swi	0x123456"
#endif

static inline int sh_trap (int op, void *arg)
{
    /* Let gcc pre-place variables in expected registers */
    register int op_reg asm ("r0") = op;
    register void *arg_reg asm ("r1") = arg;
    __asm__ __volatile__ (
        SH_TRAP_INSN
        : "+r" (op_reg) : "r" (arg_reg)
        : "r2", "r3", "ip", "lr", "memory", "cc"
    );

    /* on return from BKPT r0 contains result value */
    return op_reg;
}

void sh_putc (char c)
{
    sh_trap (SYS_WRITEC, &c);
}

void sh_puts (const char *s)
{
    sh_trap (SYS_WRITE0, (void *)s);
}

char sh_getc ()
{
    return sh_trap (SYS_READC, (void *)0);
}

static struct semihosting_backend_t
{
    printf_backend_t be;
    int top;
    char buffer [64];
} semihosting_stdout;

void sh_backend_flush (printf_backend_t *backend)
{
    struct semihosting_backend_t *self =
        CONTAINER_OF (backend, struct semihosting_backend_t, be);

    if (self->top != -1)
    {
        self->buffer [self->top] = 0;
        sh_puts (self->buffer);
        self->top = 0;
    }
}

static void sh_backend_putc (printf_backend_t *backend, char c)
{
    struct semihosting_backend_t *self =
        CONTAINER_OF (backend, struct semihosting_backend_t, be);

    if (self->top == -1)
    {
        sh_putc (c);
        return;
    }

    self->buffer [self->top++] = c;

    if ((self->top >= (int)sizeof (self->buffer) - 1) ||
        (c == '\n'))
        sh_backend_flush (backend);
}

void sh_printf (bool buffered)
{
    semihosting_stdout.be.putc = sh_backend_putc;
    semihosting_stdout.be.flush = sh_backend_flush;
    // number of used chars in buffer
    semihosting_stdout.top = buffered ? 0 : -1;

    init_printf (&semihosting_stdout.be);
}
