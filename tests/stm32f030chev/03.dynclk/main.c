/*
 * Test dynamic CPU clock routines
 */

#include "ugears.h"
#include "printf.h"

uint32_t clk_errors;

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

#ifdef CLOCK_DYNAMIC
    if ((clock & 255) == 1)
    {
        const char *clksrc = "?";
        uint32_t prev_clk_errors = clk_errors;
        unsigned expected_freq = 0;

        // STM32F0 is 48MHz maximum
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
                clksrc = "PLL = (HSI/2) * 3";
                expected_freq = (HSI_VALUE/2) * 3;
                // Divider has no effect on HSI on STM32F030 (always 2)
                if (sysclk_PLL (CLKSRC_HSI, 2, 3) != 0)
                    clk_errors |= 4;
                break;
            case 3:
                // Max clock for STM32F0 series is 48MHz, according to manual
                clksrc = "PLL = (HSI/2) * 12";
                expected_freq = (HSI_VALUE/2) * 12;
                if (sysclk_PLL (CLKSRC_HSI, 2, 12) != 0)
                    clk_errors |= 4;
                break;
            case 4:
                // This is the minimum clock we can get.
                // I failed to launch PLL clocked from HSE at <=4MHz
                clksrc = "PLL = (HSE / 15) * 8";
                expected_freq = (HSE_VALUE / 15) * 8;
                if (sysclk_PLL (CLKSRC_HSE, 15, 8) != 0)
                    clk_errors |= 4;
                break;
            case 5:
                clksrc = "PLL = (HSE / 3) * 5";
                expected_freq = (HSE_VALUE / 3) * 5;
                if (sysclk_PLL (CLKSRC_HSE, 3, 5) != 0)
                    clk_errors |= 4;
                break;
            case 6:
                clksrc = "PLL = (HSE / 5) * 10";
                expected_freq = (HSE_VALUE / 5) * 10;
                if (sysclk_PLL (CLKSRC_HSE, 5, 10) != 0)
                    clk_errors |= 4;
                break;
            case 7:
                clksrc = "PLL = HSE * 3";
                expected_freq = HSE_VALUE * 3;
                if (sysclk_PLL (CLKSRC_HSE, 1, 3) != 0)
                    clk_errors |= 4;
                break;
        }

        // Re-initialize USART1 after clock change
        usart_init (USART1, USART1_CLOCK, USART1_SETUP);

        printf ("CPU clocked from %s at %uHz, HCLK %uHz, PCLK %uHz\r\n",
                clksrc, SYSCLK_FREQ, HCLK_FREQ, PCLK_FREQ);
        if (clk_errors != prev_clk_errors)
            printf ("\tERROR: Frequency switching failed\r\n");
        if (expected_freq != SYSCLK_FREQ)
            printf ("\tERROR: Was expecting %uHz CPU clock\r\n", expected_freq);
    }
#endif
}

void usart1_init ()
{
    // Enable USART1 and GPIOs
    RCC_BEGIN;
        RCC_ENA (_USART1);
        RCC_ENA (RCC_GPIO (USART1_TX));
        RCC_ENA (RCC_GPIO (USART1_RX));
    RCC_END;

    // Set up USART1 pins
    GPIO_SETUP (USART1_TX);
    GPIO_SETUP (USART1_RX);

    // Initialize USART1
    usart_init (USART1, USART1_CLOCK, USART1_SETUP);

    // Route printf() via USART1
    usart_printf (USART1);
}

int main ()
{
    // Enable and configure LED pin
    RCC_ENABLE (RCC_GPIO (LED));
    GPIO_SETUP (LED);

    usart1_init ();
    puts ("Dynamic CPU clock example\r\n");

    // Set up system timer for CLOCKS_PER_SEC interrupts per second
    systick_config (SYSCLK_FREQ / CLOCKS_PER_SEC);
    // Enable interrupts from SysTick counter
    systick_int_enable (0);

#ifndef CLOCK_DYNAMIC
    puts ("Dynamic clocking not enabled, please add #define CLOCK_DYNAMIC to HARDWARE_H!\r\n");
#endif

    for (;;)
    {
        // Wait for interrupts
        __WFI ();
    }
}
