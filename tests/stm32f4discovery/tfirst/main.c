/*
 * Простая тестовая программа для платы STM32F4DISCOVERY
 */

#include "ugears.h"
#include "printf.h"

static const uint32_t first_gpio_config [] =
{
    GPIO_CONF (LED3),
    GPIO_CONF (LED4),
    GPIO_CONF (LED5),
    GPIO_CONF (LED6),
    GPIO_CONF (BUT1),
    GPIO_CONF (USART1_TX),
    GPIO_CONF (USART1_RX),
};

int main ()
{
    RCC_BEGIN;
        RCC_ENA (_GPIOA);
        RCC_ENA (_GPIOB);
        RCC_ENA (_GPIOD);
        RCC_ENA (_GPIOE);
        RCC_ENA (_USART1);
    RCC_END;

    gpio_setups (first_gpio_config, ARRAY_LEN (first_gpio_config));

    // 16 interrupts per second
    systick_config (sysclk_t2c (1.0/16.0));

    usart_init (USART1, PCLK2_FREQ, USART1_SETUP);

    __enable_irq ();

    usart_printf (USART1);

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
