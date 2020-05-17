/*
 * This program does almost the same as the default firmware :)
 */

#include "ugears.h"

int main ()
{
    // Enable the GPIO ports for both LEDs
    RCC_ENABLE_GPIO (LED);

    // Set up LED pin
    GPIO_SETUP (LED);

    // Enable SysTick timer so that delay() works
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);

    uint32_t counter = 0;
    for (;;)
    {
        delay (0.125);

        counter++;
#if 1
        GPIO_TOGGLE (LED);
#else
        // Other way of doing it, better flexibility but does the same :)
        if (counter & 1)
            GPIO_SET (LED);
        else
            GPIO_RESET (LED);
#endif
    }
}
