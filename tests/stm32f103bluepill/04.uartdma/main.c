#include "hw.h"

static const char test_data [] = "[This data came via DMA controller]\r\n";
static char rxbuff [10];
static volatile bool dma_tx_flag, dma_rx_flag;

void DMA_IRQ_HANDLER (SERIAL_TX) ()
{
    if (DMA (SERIAL_TX)->ISR & DMA_ISR_IF (G, SERIAL_TX))
    {
        // Acknowledge the interrupt
        DMA (SERIAL_TX)->IFCR = DMA_IFCR_IF (G, SERIAL_TX);

        // Disable USART -> DMA transmission
        usart_dma_tx (USART (SERIAL), false);

        printf ("DMA TX IRQ\r\n");

        dma_tx_flag = true;
    }
}

static void do_test_send ()
{
    printf ("Sending some data via DMA ... NOW!\r\n");

    dma_tx_flag = false;

    // Use a more general form of dma1_copy
    DMA_COPY (SERIAL_TX,
        DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8 | DMA_CCR_MINC | DMA_CCR_PL_VERYHIGH | DMA_CCR_TCIE,
        (void *)test_data, &USART (SERIAL)->DR, sizeof (test_data) - 1);

    // Enable USART -> DMA transmission
    usart_dma_tx (USART (SERIAL), true);

    // Wait until the data finishes transfer
    while (!dma_tx_flag)
        ;

    printf ("Got something?\r\n");
}

void DMA_IRQ_HANDLER (SERIAL_RX) ()
{
    if (DMA (SERIAL_RX)->ISR & DMA_ISR_IF (G, SERIAL_RX))
    {
        // Acknowledge the interrupt
        DMA (SERIAL_RX)->IFCR = DMA_IFCR_IF (G, SERIAL_RX);

        // Disable USART -> DMA transmission
        usart_dma_rx (USART (SERIAL), false);

        printf ("DMA RX IRQ\r\n");

        dma_rx_flag = true;
    }
}

static void do_test_recv ()
{
    printf ("Waiting for %d bytes from USART using DMA...\r\n"
            "Go on, send something\r\n", sizeof (rxbuff));

    dma_rx_flag = false;

    // Use a more general form of dma1_copy
    DMA_COPY (SERIAL_RX,
        DMA_CCR_PSIZE_8 | DMA_CCR_MSIZE_8 | DMA_CCR_MINC | DMA_CCR_PL_VERYHIGH | DMA_CCR_TCIE,
        (void *)&USART (SERIAL)->DR, (void *)rxbuff, sizeof (rxbuff));

    // Enable DMA -> USART transmission
    usart_dma_rx (USART (SERIAL), true);

    // Wait until DMA finishes
    while (!dma_rx_flag)
        ;

    printf ("Got [%10s]\r\n", rxbuff);
}

int main (void)
{
    // Enable DMA
    RCC_ENABLE_DMA (SERIAL_TX);

    serial_init ();
    printf ("DMA demo started\r\n");

    // Set up and enable interrupts
    nvic_setup (DMA_IRQ_NUM (SERIAL_TX), DMA_IRQ_PRIO (SERIAL_TX));
    nvic_setup (DMA_IRQ_NUM (SERIAL_RX), DMA_IRQ_PRIO (SERIAL_RX));
    __enable_irq ();

    for (;;)
    {
        if (usart_rx_ready (USART (SERIAL)))
            switch (usart_getc (USART (SERIAL)))
            {
                case 's':
                    do_test_send ();
                    break;

                case 'r':
                    do_test_recv ();
                    break;

                default:
                    printf ("s: Send test data via SERIAL+DMA\r\n"
                            "r: Receive data via SERIAL+DMA\r\n");
                    break;
            }

        // We can't _WFI() because usart_rx_ready() polls
        //WFI ();
    }
}
