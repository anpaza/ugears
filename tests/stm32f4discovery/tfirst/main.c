/*
 * Простая тестовая программа для платы STM32F4DISCOVERY
 */

#include <ugears/ugears.h>
#include <useful/printf.h>

static const uint32_t first_gpio_config [] =
{
    GPIO_CONF (LED3),
    GPIO_CONF (LED4),
    GPIO_CONF (LED5),
    GPIO_CONF (LED6),
    GPIO_CONF (BUT1),
    GPIO_CONF (SERIAL_TX),
    GPIO_CONF (SERIAL_RX),
};

int main ()
{
    RCC_BEGIN;
        RCC_ENA_GPIO (LED3);
        RCC_ENA_GPIO (LED4);
        RCC_ENA_GPIO (LED5);
        RCC_ENA_GPIO (LED6);
        RCC_ENA_GPIO (BUT1);
        RCC_ENA_USART (SERIAL);
        RCC_ENA_USART (SERIAL);
    RCC_END;

    gpio_setups (first_gpio_config, ARRAY_LEN (first_gpio_config));

    // 16 interrupts per second
    systick_config (sysclk_t2c (1.0/16.0));

    usart_init (USART (SERIAL), CLOCK_USART (SERIAL), SERIAL_SETUP);

    __enable_irq ();

    usart_printf (USART (SERIAL));

    printf ("Hello from test app!\r\n");

    for (;;)
    {
        GPIO_SET (LED3);
        delay (0.5);
        GPIO_RESET (LED3);
        GPIO_SET (LED4);
        delay (0.5);
        GPIO_RESET (LED4);
        GPIO_SET (LED6);
        delay (0.5);
        GPIO_RESET (LED6);
        GPIO_SET (LED5);
        delay (0.5);
        GPIO_RESET (LED5);
    }
}
