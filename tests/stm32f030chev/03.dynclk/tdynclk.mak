TESTS += tdynclk
DESCRIPTION.tdynclk = Flash the LED!
FLASH.TARGETS += tdynclk
IHEX.TARGETS += tdynclk

TARGETS.tdynclk = tdynclk$E
SRC.tdynclk$E = $(wildcard tests/stm32f030chev/03.dynclk/*.c)
LIBS.tdynclk$E = cmsis$L ugears$L useful$L
