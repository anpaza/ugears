/*
 * Flash LEDs with system clock timer
 */

#include "ugears.h"

void SysTick_Handler ()
{
    clock++;

    if ((clock & 7) == 1)
    {
        if (GPIO_GET (GLED))
        {
            GPIO_RESET (GLED);
            GPIO_SET (BLED);
        }
        else
        {
            GPIO_SET (GLED);
            GPIO_RESET (BLED);
        }
    }
}

int main ()
{
    // Enable used peripherials
    RCC_BEGIN;
        RCC_ENA_GPIO (BLED);
        RCC_ENA_GPIO (GLED);
    RCC_END;

    // Configure GPIO
    GPIO_SETUP (GLED);
    GPIO_SETUP (BLED);

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
