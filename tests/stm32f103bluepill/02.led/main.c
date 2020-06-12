/*
 * Blink the LED using SysTick interrupt :)
 */

#include "hw.h"

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
    // see ../stm32f103bluepill.c
    systick_init ();
    led_init ();

    for (;;) WFI ();
}
