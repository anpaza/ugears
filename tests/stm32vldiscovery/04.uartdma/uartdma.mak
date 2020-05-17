TESTS += tuartdma
DESCRIPTION.tuartdma = Test UART DMA transfers
FLASH.TARGETS += tuartdma
IHEX.TARGETS += tuartdma

TARGETS.tuartdma = tuartdma$E
SRC.tuartdma$E = $(wildcard tests/stm32vldiscovery/04.uartdma/*.c) \
	tests/stm32vldiscovery/hw.c
LIBS.tuartdma$E = cmsis$L ugears$L useful$L
