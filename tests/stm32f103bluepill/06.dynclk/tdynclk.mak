TESTS += tdynclk
DESCRIPTION.tdynclk = Demonstrate dynamic change of clock speed on STM32F1
FLASH.TARGETS += tdynclk
IHEX.TARGETS += tdynclk

TARGETS.tdynclk = tdynclk$E
SRC.tdynclk$E = $(wildcard tests/stm32f103bluepill/06.dynclk/*.c) \
	tests/stm32f103bluepill/hw.c
LIBS.tdynclk$E = cmsis$L ugears$L useful$L
