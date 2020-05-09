/*
 * This program does almost the same as the default STM32VLDISCOVERY firmware :)
 *
 * Push the "User" button (blue) to change LED flashing period.
 */

#include "ugears.h"

int main ()
{
    // Enable the GPIO ports for both LEDs
    RCC_ENABLE (RCC_GPIO (BLED));
    RCC_ENABLE (RCC_GPIO (GLED));
    RCC_ENABLE (RCC_GPIO (USRBUT));

    // Set up LED pins
    GPIO_SETUP (GLED);
    GPIO_SETUP (BLED);
    // Set up button pin
    GPIO_SETUP (USRBUT);

    // Enable SysTick timer so that delay() works
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);

    uint32_t counter = 0, speed = 0;
    uint32_t old_bst = 0;
    for (;;)
    {
        delay (0.125);

        // Check the button
        uint32_t bst = GPIO_GET (USRBUT);
        if (bst != old_bst)
        {
            old_bst = bst;
            // If pressed, light the blue LED
            if (bst)
            {
                GPIO_SET (BLED);
                speed = (speed + 1) & 3;
            }
            else
                GPIO_RESET (BLED);
        }

        counter++;
        if (counter & (1 << speed))
            GPIO_SET (GLED);
        else
            GPIO_RESET (GLED);
    }
}
