TESTS += tled
DESCRIPTION.tled = Flash the LED!
FLASH.TARGETS += tled
IHEX.TARGETS += tled

TARGETS.tled = tled$E
SRC.tled$E = $(wildcard tests/stm32vldiscovery/02.led/*.c)
LIBS.tled$E = cmsis$L useful$L ugears$L
