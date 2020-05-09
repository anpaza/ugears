TESTS += tled
DESCRIPTION.tled = Flash the LED!
FLASH.TARGETS += tled
IHEX.TARGETS += tled

TARGETS.tled = tled$E
SRC.tled$E = $(wildcard tests/stm32f103bluepill/02.led/*.c) \
	tests/stm32f103bluepill/stm32f103bluepill.c
LIBS.tled$E = cmsis$L useful$L ugears$L
