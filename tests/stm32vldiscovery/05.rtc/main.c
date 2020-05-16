/**
 * Test Real-Time clock routines.
 */

#include "hw.h"
#include "datetime.h"

int main (void)
{
    usart1_init ();
    printf ("Real-Time-Clock demo started\r\n");

    systick_init ();
    led_init ();
    rtc_init ();

    uint32_t old_bst = 0;
    uint32_t old_clock = clock;
    for (;;)
    {
        // flash the green LED!
        if (old_clock != clock)
        {
            old_clock = clock;

            // Check the button
            uint32_t bst = GPIO_GET (USRBUT);
            if (bst != old_bst)
            {
                old_bst = bst;
                // If pressed, light the blue LED
                if (bst)
                    GPIO_SET (BLED);
                else
                    GPIO_RESET (BLED);
            }

            if ((old_clock & (CLOCKS_PER_SEC - 1)) == 0)
            {
                GPIO_SET (GLED);

                datetime_t dt;
                uint32_t ut = rtc_counter ();
                ut2dt (RTC_BASETIME + (ut >> 4), &dt);
                printf ("(%u) %04d/%02d/%02d %02d:%02d:%02d.%d\r\n",
                    ut,
                    dt.year, dt.mon + 1, dt.day,
                    dt.hour, dt.min, dt.sec,
                    ((ut & 15) * 10) / 16);

                GPIO_RESET (GLED);
            }
        }

        // Nothing to do till next interrupt
        __WFI ();
    }
}
