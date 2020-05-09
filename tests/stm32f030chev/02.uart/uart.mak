TESTS += tuart
DESCRIPTION.tuart = Test basic UART functions
FLASH.TARGETS += tuart
IHEX.TARGETS += tuart

TARGETS.tuart = tuart$E
SRC.tuart$E = $(wildcard tests/stm32f030chev/02.uart/*.c)
LIBS.tuart$E = cmsis$L ugears$L useful$L
