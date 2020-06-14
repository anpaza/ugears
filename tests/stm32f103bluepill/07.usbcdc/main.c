/* Sample program that uses the USB CDC ACM library
 */

#include "hw.h"
#include <usb/usb-cdc-acm.h>

void SysTick_Handler ()
{
    clock++;

    GPIO_SET (LED);
    if (uca_active && !uca_suspended)
    {
        if (uca_line_state & USB_CDC_LINE_STATE_DTR)
            // Heartbeat if DTR active (serial port open)
            switch (clock & (CLOCKS_PER_SEC - 1))
            {
                case 0:
                case CLOCKS_PER_SEC / 4:
                    GPIO_RESET (LED);
                    break;
            }
        else if (uca_active && !(clock & (CLOCKS_PER_SEC - 1)))
            // Flash if USB is active
            GPIO_RESET (LED);
    }
}

static const char *stuff =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ@0123456789|abcdefghijklmnopqrstuvwxyz"
        "zyxwvutsrqponmlkjihgfedcba|9876543210@ZYXWVUTSRQPONMLKJIHGFEDCBA"
        "-A-B-C-D-E-F-G-H-I-J-K-L-M-N-O-P-Q-R-S-T-U-V-W-X-Y-Z-@-0-1-2-3-4"
        "=5=6=7=8=9=|=a=b=c=d=e=f=g=h=i=j=k=l=m=n=o=p=q=r=s=t=u=v=w=x=y=z";

static uint8_t txbuff [UCA_RECEIVED_MAX];

uca_status_t uca_received (const void *data, unsigned data_size, bool last)
{
    // we don't care if it's last packet or not
    (void)last;

    if (!data_size)
        return UCA_ST_VALID;

    // By default, loopback any received data back to user
    const void *reply;

    unsigned n = (*(const uint8_t *)data - '1');
    if ((data_size == 1) && (n < 9))
    {
        reply = stuff;
        data_size = 1 << n;
    }
    else
    {
        // Make a copy of received data. We can't uca_transmit data
        // directly because if we return VALID, the RX buffer will be
        // reused for receiving new packets, and data transmission
        // may be still unfinished at that time.
        memcpy (txbuff, data, data_size);
        reply = txbuff;
    }

    return uca_transmit (reply, data_size) ? UCA_ST_VALID : UCA_ST_NAK;
}

void uca_transmitted ()
{
    // Check if more data is pending in USB CDC ACM receive buffers
    uca_check_receive ();
}

// 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space */
static const char parity [] = "NOEMS";
// 0: 1 stop bit, 1: 1.5 stop bits, 2: 2 stop bits
static const char format [] = "1.2";

void uca_event (unsigned flags)
{
    if (flags & UCA_EVENT_ACTIVE)
    {
        printf ("USB active: %d\n", uca_active);
    }
    if (flags & UCA_EVENT_SUSPEND)
    {
        printf ("USB suspended: %d\n", uca_suspended);
    }
    if (flags & UCA_EVENT_LINE_STATE)
    {
        printf ("LINE STATE: %s %s\n",
                (uca_line_state & USB_CDC_LINE_STATE_DTR) ? "DTR" : "dtr",
                (uca_line_state & USB_CDC_LINE_STATE_RTS) ? "RTS" : "rts");
    }
    if (flags & UCA_EVENT_LINE_FORMAT)
    {
        printf ("LINE FORMAT: %u %u-%c-%c\n",
                uca_line_format.dwDTERate,
                uca_line_format.bDataBits,
                parity [uca_line_format.bParityType],
                format [uca_line_format.bCharFormat]);
    }
    if (flags & UCA_EVENT_BREAK)
        printf ("GOT A BREAK\n");
}

int main (void)
{
    systick_init ();
    led_init ();
// 0 to printf() to USB, 1 to printf() to USART1
#if 0
    uca_init ();
    uca_printf ();
#else
    // Enable trace() in cdcacm.c to debug USB protocol
    serial_init ();
    usart_printf (USART (SERIAL));
    uca_init ();
#endif

    // Actually most likely you won't see this via uca_printf.
    puts ("\nUSB-CDC demo started");

    for (;;) WFI ();
}
