/**
 * Test Real-Time clock routines.
 */

#include "hw.h"
#include <useful/datetime.h>

int main (void)
{
    serial_init ();
    printf ("Real-Time-Clock demo started\r\n");

    systick_init ();
    led_init ();
    rtc_init ();

    for (;;)
    {
        if ((clock & (CLOCKS_PER_SEC - 1)) == 0)
        {
            GPIO_SET (LED);

            datetime_t dt;
            uint32_t ut = rtc_counter ();
            ut2dt (RTC_BASETIME + (ut >> 4), &dt);
            printf ("(%u) %04d/%02d/%02d %02d:%02d:%02d.%d\r\n",
                ut,
                dt.year, dt.mon + 1, dt.day,
                dt.hour, dt.min, dt.sec,
                ((ut & 15) * 10) / 16);

            GPIO_RESET (LED);
        }

        // Nothing to do till next interrupt
        WFI ();
    }
}
