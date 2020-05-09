TESTS += tuart
DESCRIPTION.tuart = Test the UART library
FLASH.TARGETS += tuart
IHEX.TARGETS += tuart

TARGETS.tuart = tuart$E
SRC.tuart$E = $(wildcard tests/stm32f103bluepill/03.uart/*.c) \
	tests/stm32f103bluepill/stm32f103bluepill.c
LIBS.tuart$E = cmsis$L ugears$L useful$L
