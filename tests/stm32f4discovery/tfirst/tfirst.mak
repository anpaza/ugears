TESTS += tfirst
DESCRIPTION.tfirst = My First Program for STM32F4 :)
FLASH.TARGETS += tfirst
IHEX.TARGETS += tfirst

TARGETS.tfirst = tfirst$E
SRC.tfirst$E = $(wildcard tests/stm32f4discovery/tfirst/*.c)
LIBS.tfirst$E = cmsis$L ugears$L useful$L
