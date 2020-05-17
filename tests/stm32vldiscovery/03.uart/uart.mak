TESTS += tuart
DESCRIPTION.tuart = Test the UART library
FLASH.TARGETS += tuart
IHEX.TARGETS += tuart

TARGETS.tuart = tuart$E
SRC.tuart$E = $(wildcard tests/stm32vldiscovery/03.uart/*.c) \
	tests/stm32vldiscovery/hw.c
LIBS.tuart$E = cmsis$L ugears$L useful$L
