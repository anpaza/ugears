/*
 * We're not using libc at all on stm32, so we'll just provide this stub to make
 * startup_xxx.s files happy.
 *
 * Keep in mind that static C++ objects will not be constructed at startup time.
 */

extern void _init () __attribute__((weak));

void __libc_init_array ()
{
    if (&_init)
        _init ();
}
