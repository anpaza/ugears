/*
 * Test UART functions
 */

#include "hw.h"

static void do_test_xsend ()
{
    for (;;)
    {
        if (usart_rx_ready (USART1))
            switch (usart_getc (USART1))
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
        while (!usart_tx_complete (USART1))
            ;

        // Reconfugure USART1_TX to mute mode
        GPIO_SETUP (USART1_TX_MUTE);

        // You should see none of these
        for (unsigned i = 0; i < 1000; i++)
            putc ('@');

        // Wait until data leaves output FIFO and DR.
        while (!usart_tx_complete (USART1))
            ;

        // Return USART1_TX to default mode
        GPIO_SETUP (USART1_TX);
    }
}

int main (void)
{
    // Initialize USART1
    usart1_init ();
    puts ("USART library demo running");

    systick_init ();
    led_init ();

    uint32_t old_clock = clock;
    uint32_t old_bst = 0;
    for (;;)
    {
        if (usart_rx_ready (USART1))
            switch (usart_getc (USART1))
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
