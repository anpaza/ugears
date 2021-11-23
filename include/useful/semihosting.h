/*
    Useful function for embedded systems
    Copyright (C) 2016 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _SEMIHOSTING_H
#define _SEMIHOSTING_H

/*
    Some simple functions using semi-hosting interface.
    If you use any of those, your application won't run
    except under a debugger, so make sure you use them
    only in debug builds.
*/

#include "useful.h"

/**
 * Initialize printf() to output via JTAG into your console session.
 * For output to work, you must use software with implemented
 * semi-hosting functions (e.g. st-util --semihosting).
 * @arg buffered
 *      If true, char output will be buffered until buffer is full
 *      or an '\r' or '\n' is encountered. If you need char-by-char
 *      output, pass false here. Note this is MUCH slower.
 */
EXTERN_C void sh_printf (bool buffered);

/**
 * Output a single character via the semihosting interface.
 */
EXTERN_C void sh_putc (char c);

/**
 * Read a single character from debugging terminal.
 */
EXTERN_C char sh_getc ();

#endif // _SEMIHOSTING_H
