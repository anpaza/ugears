/**
 * Semi-hosting is a doubtfully useful feature of ARM/JTAG that allows
 * the program running on MCU to invoke host functions, for example
 * writing and reading to console, opening, reading and even removing
 * and renaming files on the debugging host etc.
 *
 * Currently uGears supports only console i/o functions.
 *
 * To test them, flash this application to your MCU and then run your
 * SWD/JTAG agent with semihosting option enabled
 * (e.g. st-util --semihosting). Then connect to application with
 * debuger as usual and run the application.
 */

#include "useful/semihosting.h"
#include "useful/printf.h"

int main ()
{
    sh_printf (false);
    printf ("Unbuffered hello from test app!\n");

    sh_printf (true);
    printf ("Buffered hello from test app!\n");

    printf ("one"); fflush ();
    printf ("two"); fflush ();
    printf ("three"); fflush ();
    puts ("\ndone");

    for (;;)
        printf ("sh_getc = %d\n", sh_getc ());

    return 0;
}
