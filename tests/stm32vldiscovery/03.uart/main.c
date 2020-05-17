/*
 * Test UART functions
 */

#include "hw.h"

static void do_test_xsend ()
{
    for (;;)
    {
        if (usart_rx_ready (USART (SERIAL)))
            switch (usart_getc (USART (SERIAL)))
            {
                case 'q':
                    return;

                default:
                    puts ("\r\nq: Quit test");
                    break;
            }

        puts ("\r\nSending 1000 visible characters:");
        for (unsigned i = 0; i < 1000; i++)
            putc ('.');

        puts ("\r\nNow sending 1000 visible characters with TX off:");

        // Wait until data leaves output FIFO and DR.
        while (!usart_tx_complete (USART (SERIAL)))
            ;

        // Reconfugure TX to mute mode
        GPIO_SETUP (SERIAL_TX_MUTE);

        // You should see none of these
        for (unsigned i = 0; i < 1000; i++)
            putc ('@');

        // Wait until data leaves output FIFO and DR.
        while (!usart_tx_complete (USART (SERIAL)))
            ;

        // Return TX to default mode
        GPIO_SETUP (SERIAL_TX);
    }
}

int main (void)
{
    // Initialize serial port
    serial_init ();
    puts ("USART library demo running");

    systick_init ();
    led_init ();

    uint32_t old_clock = clock;
    uint32_t old_bst = 0;
    for (;;)
    {
        if (usart_rx_ready (USART (SERIAL)))
            switch (usart_getc (USART (SERIAL)))
            {
                case 'x':
                    do_test_xsend ();
                    break;

                default:
                    puts ("\r\nx: Test transmitter with output disabled");
                    break;
            }

        if (old_clock != clock)
        {
            old_clock = clock;
            if (!(old_clock & 15))
            {
                GPIO_SET (GLED);
                printf ("[%d]", old_clock >> 4);
                GPIO_RESET (GLED);
            }

            // Check the button
            uint32_t bst = GPIO_GET (USRBUT);
            if (bst != old_bst)
            {
                old_bst = bst;

                // If pressed, light the blue LED
                if (bst)
                    GPIO_SET (BLED);
                else
                    GPIO_RESET (BLED);

                printf ("[button %d]", bst);
            }
        }
    }
}
