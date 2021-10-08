/*
 * Partial global static objects initialization support:
 * only initialization (constructors)
 * finalization (destructors) are not supported.
 */

#include <stddef.h>

extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

extern void _init () __attribute__((weak));

void __libc_init_array ()
{
    size_t count, i;

    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start [i] ();

    if (&_init)
        _init ();

    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++)
        __init_array_start [i] ();
}
