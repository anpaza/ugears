/*
File: printf.h

Copyright (C) 2004  Kustaa Nyholm

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
@file printf.h

This library is realy just two files: 'printf.h' and 'printf.c'.

They provide a simple and small (+200 loc) printf functionality to 
be used in embedded systems.

I've found them so usefull in debugging that I do not bother with a 
debugger at all.

They are distributed in source form, so to use them, just compile them 
into your project. 

Two printf variants are provided: printf and sprintf. 

The formats supported by this implementation are: 'd' 'u' 'c' 's' 'x' 'X'.

Zero padding and field width are also supported.

If the library is compiled with 'PRINTF_SUPPORT_LONG' defined then the 
long specifier is also
supported. Note that this will pull in some long math routines (pun intended!)
and thus make your executable noticably longer.

The memory foot print of course depends on the target cpu, compiler and 
compiler options, but a rough guestimate (based on a H8S target) is about 
1.4 kB for code and some twenty 'int's and 'char's, say 60 bytes of stack space. 
Not too bad. Your milage may vary. By hacking the source code you can 
get rid of some hunred bytes, I'm sure, but personally I feel the balance of 
functionality and flexibility versus  code size is close to optimal for
many embedded systems.

To use the printf you need to supply your own character output function, 
something like :

@code
void putc ( char c, void* p )
	{
	while (!SERIAL_PORT_EMPTY) ;
	SERIAL_PORT_TX_REGISTER = c;
	}
@endcode

Before you can call printf you need to initialize it to use your 
character output function with something like:

@code
init_printf(NULL,putc);
@endcode

Notice the 'NULL' in 'init_printf' and the parameter 'void* p' in 'putc', 
the NULL (or any pointer) you pass into the 'init_printf' will eventually be 
passed to your 'putc' routine. This allows you to pass some storage space (or 
anything realy) to the character output function, if necessary. 
This is not often needed but it was implemented like that because it made 
implementing the sprintf function so neat (look at the source code).

The code is re-entrant, except for the 'init_printf' function, so it 
is safe to call it from interupts too, although this may result in mixed output. 
If you rely on re-entrancy, take care that your 'putc' function is re-entrant!

The printf and sprintf functions are actually macros that translate to 
'tfp_printf' and 'tfp_sprintf'. This makes it possible
to use them along with 'stdio.h' printf's in a single source file. 
You just need to undef the names before you include the 'stdio.h'.
Note that these are not function like macros, so if you have variables
or struct members with these names, things will explode in your face.
Without variadic macros this is the best we can do to wrap these
fucnction. If it is a problem just give up the macros and use the
functions directly or rename them.

For further details see source code.

regs Kusti, 23.10.2004
*/


#ifndef __TFP_PRINTF__
#define __TFP_PRINTF__

#include <stdarg.h>

// Uncomment for long format support (%l)
//#define PRINTF_LONG_SUPPORT
// Uncomment for fixed-point support (%[width].[fracdigits].[fracbits](f|F))
#define PRINTF_FP_SUPPORT

/**
 * This defines the backend functions that do actual low-level output.
 */
typedef struct _printf_backend_t
{
    /**
     * The function that outputs a single character.
     * @arg self A pointer to this printf_backend_t structure
     * @arg c The character to echo
     */
    void (*putc) (struct _printf_backend_t *self, char c);

    /**
     * Flush the accumulation buffer, if supported.
     * This can be NULL (if the backend does not support buffering),
     * so you must check before using it.
     */
    void (*flush) (struct _printf_backend_t *self);

    /**
     * Additional user data attached to this printing backend.
     */
    void *data;
} printf_backend_t;

extern printf_backend_t *stdout_backend;

/**
 * Define the low-level backend for printf() & company.
 */
static inline void init_printf (printf_backend_t *stdout)
{ stdout_backend = stdout; }

/**
 * The usual printf(), works via the stdout backend.
 * @arg fmt
 *      The C-style format string with some modifications.
 */
extern void tfp_printf (const char *fmt, ...);

/**
 * Output a single character via the stdout backend.
 */
extern void tfp_putc (char c);

/**
 * Output a string followed by \n.
 * @arg s
 *      The string.
 */
extern void tfp_puts (const char *s);

/**
 * This is your usual sprintf(), using same format strings as printf().
 * Note this function does not check for buffer bounds, so if you don't
 * take care, you can write past the end of your buffer.
 * @arg s
 *      The output buffer
 * @arg fmt
 *      The C-style format string.
 */
extern void tfp_sprintf (char *s, const char *fmt, ...);

/**
 * This is a lower-level printf using a custom backend.
 */
extern void tfp_format (printf_backend_t *backend, const char *fmt, va_list va);

/**
 * Flush the stdout buffer, if any
 */
extern void tfp_fflush ();

#define printf		tfp_printf
#define sprintf		tfp_sprintf
#define putc		tfp_putc
#define puts		tfp_puts
#define fflush		tfp_fflush

#endif
