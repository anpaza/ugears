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

void SysTick_Handler ()
{
    clock++;

    if ((clock & (CLOCKS_PER_SEC - 1)) == 0)
    {
        if (GPIO_GET (LED))
            GPIO_RESET (LED);
        else
            GPIO_SET (LED);

        printf ("[%d]", clock / CLOCKS_PER_SEC);
    }
}

int main (void)
{
    led_init ();
    systick_init ();
    usart1_init ();

    puts ("\r\nUSART library demo running");

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
    }
}
