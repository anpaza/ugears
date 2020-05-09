TESTS += tled
DESCRIPTION.tled = Flash the LED!
FLASH.TARGETS += tled
IHEX.TARGETS += tled

TARGETS.tled = tled$E
SRC.tled$E = $(wildcard tests/stm32f030chev/01.led/*.c)
LIBS.tled$E = cmsis$L ugears$L useful$L
