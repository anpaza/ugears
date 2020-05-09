TESTS += tdynclk
DESCRIPTION.tdynclk = Demonstrate dynamic change of clock speed on STM32F1
FLASH.TARGETS += tdynclk
IHEX.TARGETS += tdynclk

TARGETS.tdynclk = tdynclk$E
SRC.tdynclk$E = $(wildcard tests/stm32vldiscovery/06.dynclk/*.c) \
	tests/stm32vldiscovery/stm32vldiscovery.c
LIBS.tdynclk$E = cmsis$L ugears$L useful$L
