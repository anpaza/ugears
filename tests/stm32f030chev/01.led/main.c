/*
 * Blink the LED :)
 */

#include "ugears.h"
#include "time.h"

void SysTick_Handler ()
{
    clock++;

    if ((clock & 7) == 0)
    {
        if (GPIO_GET (LED))
            GPIO_RESET (LED);
        else
            GPIO_SET (LED);
    }
}

int main ()
{
    // Enable LED pin
    RCC_ENABLE (RCC_GPIO (LED));

    // Конфигурируем GPIO
    GPIO_SETUP (LED);

    // Set up system timer for CLOCKS_PER_SEC interrupts per second
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);
    // Enable interrupts from SysTick counter
    systick_int_enable (0);

    for (;;)
    {
        // Wait for next interrupt
        __WFI ();
    }
}
