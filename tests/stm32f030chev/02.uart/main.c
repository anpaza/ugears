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

void serial_init ()
{
    // Enable USART and GPIOs
    RCC_BEGIN;
        RCC_ENA_USART (SERIAL);
        RCC_ENA_GPIO (SERIAL_TX);
        RCC_ENA_GPIO (SERIAL_RX);
    RCC_END;

    // Set up USART pins
    GPIO_SETUP (SERIAL_TX);
    GPIO_SETUP (SERIAL_RX);

    // Initialize SERIAL
    usart_init (USART (SERIAL), CLOCK_USART (SERIAL), SERIAL_SETUP);

    // Route printf() via USART
    usart_printf (USART (SERIAL));
}

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

        // Reconfugure USART_TX to mute mode
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

int main ()
{
    // Initialize serial port
    serial_init ();
    puts ("USART library demo running");

    // Enable the LED port
    RCC_ENABLE_GPIO (LED);
    // Configure LED pin
    GPIO_SETUP (LED);

    // Set up system timer for CLOCKS_PER_SEC interrupts per second
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);
    // Enable interrupts from SysTick counter
    systick_int_enable (0);

    for (;;)
    {
        if (usart_rx_ready (USART (SERIAL)))
            switch (usart_getc (USART (SERIAL)))
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
