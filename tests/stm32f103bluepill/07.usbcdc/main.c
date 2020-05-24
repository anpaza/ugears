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
        if (uca_line_state & USB_CDC_LINE_STATE_DTR)
            puts ("PING");
    }
    //printf ("[%u]", clock & (CLOCKS_PER_SEC - 1));
}

static const char *stuff =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ@0123456789|abcdefghijklmnopqrstuvwxyz"
        "zyxwvutsrqponmlkjihgfedcba|9876543210@ZYXWVUTSRQPONMLKJIHGFEDCBA";

uca_status_t uca_received (const void *data, unsigned data_size)
{
    // By default, loopback any received data back to user
    const void *reply = data;

    if (data_size == 1)
    {
        unsigned n = (*(const uint8_t *)data - '1');
        if (n < 8)
        {
            reply = stuff;
            data_size = 1 << n;
        }
    }

    return uca_transmit (reply, data_size) ? UCA_ST_VALID : UCA_ST_NAK;
}

void uca_line_state_changed ()
{
    printf ("LINE STATE: %s %s\n",
            (uca_line_state & USB_CDC_LINE_STATE_DTR) ? "DTR" : "dtr",
            (uca_line_state & USB_CDC_LINE_STATE_RTS) ? "RTS" : "rts");
}

// 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space */
static const char parity [] = "NOEMS";
// 0: 1 stop bit, 1: 1.5 stop bits, 2: 2 stop bits
static const char format [] = "1.2";

void uca_line_format_changed ()
{
    printf ("LINE FORMAT: %u %u-%c-%c\n",
            uca_line_format.dwDTERate,
            uca_line_format.bDataBits,
            parity [uca_line_format.bParityType],
            format [uca_line_format.bCharFormat]);
}

void uca_line_break ()
{
    printf ("GOT A BREAK\n");
}

int main (void)
{
    systick_init ();
    led_init ();
#if 1
    uca_init ();
    uca_printf ();
#else
    serial_init ();
    usart_printf (USART (SERIAL));
    uca_init ();
#endif

    // Actually most likely you won't see this via uca_printf.
    puts ("\nUSB-CDC demo started");

    for (;;)
    {
        __WFI ();
    }
}
