TESTS += tfirst
DESCRIPTION.tfirst = Simplest example, similar to default firmware
FLASH.TARGETS += tfirst
IHEX.TARGETS += tfirst

TARGETS.tfirst = tfirst$E
SRC.tfirst$E = $(wildcard tests/stm32vldiscovery/01.first/*.c)
LIBS.tfirst$E = cmsis$L ugears$L useful$L
