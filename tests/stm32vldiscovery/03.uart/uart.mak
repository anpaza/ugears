TESTS += tuart
DESCRIPTION.tuart = Test the UART library
FLASH.TARGETS += tuart
IHEX.TARGETS += tuart

TARGETS.tuart = tuart$E
SRC.tuart$E = tests/stm32vldiscovery/stm32vldiscovery.c $(wildcard tests/stm32vldiscovery/03.uart/*.c)
LIBS.tuart$E = cmsis$L ugears$L useful$L
