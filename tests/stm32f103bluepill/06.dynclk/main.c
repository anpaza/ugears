/*
 * Test dynamic clock routines.
 * Please connect to USART1, useful output goes there.
 */

#include "hw.h"

uint32_t clk_errors = 0;
uint32_t systick = 0;

void SysTick_Handler ()
{
    systick++;

    if ((systick & 3) == 1)
        GPIO_TOGGLE (LED);
}

uint64_t systicks ()
{
    // Return a uniformly growing more or less accurate 32-bit CPU clock counter
    uint32_t reload = systick_reload ();
    return (systick * reload) + (reload - systick_counter ());
}

uint32_t saved_systicks = 0;
uint32_t saved_clocks = 0;

void RTC_IRQHandler ()
{
    RTC32->CRL &= ~RTC_CRL_SECF;
    clock++;

#ifdef CLOCK_DYNAMIC
    if ((clock & 255) == 1)
    {
        // Fires every (64/RTC_TICKS_PER_SEC) seconds
        if (saved_systicks != 0)
        {
            uint32_t elapsed = systicks () - saved_systicks;
            uint32_t clocks = rtc_counter () - saved_clocks;
            printf ("RTC says CPU clock was %uHz\r\n",
                (uint32_t)(((uint64_t)elapsed * RTC_TICKS_PER_SEC) / clocks));
        }

        const char *clksrc = "?";
        uint32_t prev_clk_errors = clk_errors;
        uint32_t expected_freq = 0;

        // STM32F103 is 72MHz maximum
        // and supports only HSE PLL pre-divider only 1 or 2
        switch ((clock >> 8) & 7)
        {
            case 0:
                clksrc = "HSI";
                expected_freq = HSI_VALUE;
                if (sysclk_HSI () != 0)
                    clk_errors |= 1;
                break;
            case 1:
                clksrc = "HSE";
                expected_freq = HSE_VALUE;
                if (sysclk_HSE () != 0)
                    clk_errors |= 2;
                break;
            case 2:
                clksrc = "PLL = (HSI / 2) * 2";
                expected_freq = (HSI_VALUE / 2) * 2;
                if (sysclk_PLL (CLKSRC_HSI, 2, 2) != 0)
                    clk_errors |= 4;
                break;
            case 3:
                clksrc = "PLL = (HSI / 2) * 3";
                expected_freq = (HSI_VALUE / 2) * 3;
                if (sysclk_PLL (CLKSRC_HSI, 2, 3) != 0)
                    clk_errors |= 4;
                break;
            case 4:
                // We can divide HSE only by 1 or 2, start from the lowest clock
                clksrc = "PLL = (HSE / 2) * 2";
                expected_freq = (HSE_VALUE / 2) * 2;
                if (sysclk_PLL (CLKSRC_HSE, 2, 2) != 0)
                    clk_errors |= 4;
                break;
            case 5:
                clksrc = "PLL = (HSE / 2) * 5";
                expected_freq = (HSE_VALUE / 2) * 5;
                if (sysclk_PLL (CLKSRC_HSE, 2, 5) != 0)
                    clk_errors |= 4;
                break;
            case 6:
                clksrc = "PLL = HSE * 6";
                expected_freq = HSE_VALUE * 6;
                if (sysclk_PLL (CLKSRC_HSE, 1, 6) != 0)
                    clk_errors |= 4;
                break;
            case 7:
                clksrc = "PLL = HSE * 9";
                expected_freq = HSE_VALUE * 9;
                if (sysclk_PLL (CLKSRC_HSE, 1, 9) != 0)
                    clk_errors |= 4;
                break;
        }

        // APB1 bus is 36MHz max, so div by 2 if sysclock is too high
        clock_APB1 ((SYSCLK_FREQ > 36000000) ? PCLK1_DIV_FLAGS (2) : PCLK1_DIV_FLAGS (1));

        // Count CPU clocks till next freq change
        saved_systicks = systicks ();
        saved_clocks = rtc_counter ();

        // Reinitialize USART after frequency change
        usart_init (USART (SERIAL), USART_CLOCK_FREQ (SERIAL), SERIAL_SETUP);

        printf ("Clock source %s, CPU %uHz, HCLK %uHz, PCLK %uHz, PCLK2 %uHz\r\n",
                clksrc, SYSCLK_FREQ, HCLK_FREQ, PCLK1_FREQ, PCLK2_FREQ);
        if (clk_errors != prev_clk_errors)
            printf ("\tERROR: Frequency switching failed\r\n");
        if (expected_freq != SYSCLK_FREQ)
            printf ("\tERROR: Was expecting %uHz CPU clock\r\n", expected_freq);
    }
#endif
}

int main ()
{
    clock = 0;

    systick_init ();
    led_init ();
    serial_init ();

    puts ("Dynamic CPU clock example\r\n");

    rtc_init ();
    nvic_setup (RTC_IRQn, 255);
    RTC_WRITE
    {
        // Clear IRQ flags
        RTC32->CRL &= ~(RTC_CRL_SECF | RTC_CRL_ALRF | RTC_CRL_OWF);
        // Enable interrupts
        rtc_irqs (RTC_CRH_SECIE);
    }

#ifndef CLOCK_DYNAMIC
    puts ("Dynamic clocking not enabled, please add #define CLOCK_DYNAMIC to HARDWARE_H!\r\n");
#endif

    for (;;) WFI ();
}
