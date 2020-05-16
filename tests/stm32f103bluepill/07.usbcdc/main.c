/* Sample program that uses the USB CDC ACM library
 */

#include "hw.h"
#include "usb-cdc-acm.h"

void SysTick_Handler ()
{
    clock++;

    if ((clock & (CLOCKS_PER_SEC - 1)) == 0)
    {
        GPIO_TOGGLE (LED);
        if (uca_line_state & 1)
            puts ("\nPING");
    }
    printf ("[%u]", clock & (CLOCKS_PER_SEC - 1));
}

static const char *stuff =
        "0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ|abcdefghijklmnopqrstuvwxyz"
        "zyxwvutsrqponmlkjihgfedcba|ZYXWVUTSRQPONMLKJIHGFEDCBA@9876543210";

void uca_received (const void *data, unsigned data_size)
{
    if (data_size == 1)
        switch (*(const uint8_t *)data)
        {
            case '1': uca_transmit (stuff, 2); return;
            case '2': uca_transmit (stuff, 4); return;
            case '3': uca_transmit (stuff, 8); return;
            case '4': uca_transmit (stuff, 16); return;
            case '5': uca_transmit (stuff, 32); return;
            case '6': uca_transmit (stuff, 64); return;
            case '7': uca_transmit (stuff, 128); return;
        }

    // Loopback any received data back to user
    uca_transmit (data, data_size);
}

void uca_line_state_changed ()
{
    printf ("\nLINE STATE: %04x\n", uca_line_state);
}

// 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space */
static const char parity [] = "NOEMS";
// 0: 1 stop bit, 1: 1.5 stop bits, 2: 2 stop bits
static const char format [] = "1.2";

void uca_line_format_changed ()
{
    printf ("\nLINE FORMAT: %u %u-%c-%c\n",
            uca_line_format.dwDTERate,
            uca_line_format.bDataBits,
            parity [uca_line_format.bParityType],
            format [uca_line_format.bCharFormat]);
}

int main (void)
{
    systick_init ();
    led_init ();
    uca_init ();
    uca_printf ();

    puts ("\nUSB-CDC demo started");

    for (;;)
    {
        __WFI ();
    }
}
