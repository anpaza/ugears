/*
 * Mixed LED + UART example
 */

#include "ugears.h"
#include "printf.h"

uint8_t led_override = 0;

void SysTick_Handler ()
{
    clock++;

    if (((clock & 7) == 0) && (led_override == 0))
    {
        if (GPIO_GET (LED))
            GPIO_RESET (LED);
        else
            GPIO_SET (LED);
    }

    if ((clock & 31) == 0)
        printf ("[%d]", clock >> 5);
}

void usart1_init ()
{
    // Enable USART1 and GPIOs
    RCC_BEGIN;
        RCC_ENA (_USART1);
        RCC_ENA (RCC_GPIO (USART1_TX));
        RCC_ENA (RCC_GPIO (USART1_RX));
    RCC_END;

    // Set up USART1 pins
    GPIO_SETUP (USART1_TX);
    GPIO_SETUP (USART1_RX);

    // Initialize USART1
    usart_init (USART1, USART1_CLOCK, USART1_SETUP);

    // Route printf() via USART1
    usart_printf (USART1);
}

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

int main ()
{
    // Initialize USART1
    usart1_init ();
    puts ("USART library demo running");

    // Enable the LED port
    RCC_ENABLE (RCC_GPIO (LED));
    // Configure LED pin
    GPIO_SETUP (LED);

    // Set up system timer for CLOCKS_PER_SEC interrupts per second
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);
    // Enable interrupts from SysTick counter
    systick_int_enable (0);

    for (;;)
    {
        if (usart_rx_ready (USART1))
            switch (usart_getc (USART1))
            {
                case 'x':
                    do_test_xsend ();
                    break;

                case '0':
                    GPIO_RESET (LED);
                    led_override = 0;
                    break;

                case '1':
                    GPIO_SET (LED);
                    led_override = 1;
                    break;

                default:
                    puts ("\r\nx: Test transmitter with output disabled"
                          "\r\n0: Release LED control"
                          "\r\n1: Grab LED and turn it on"
                         );
                    break;
            }
    }
}
