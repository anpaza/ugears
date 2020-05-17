TESTS += tlibfun
DESCRIPTION.tlibfun = Test for some library functions
FLASH.TARGETS += tlibfun
IHEX.TARGETS += tlibfun

TARGETS.tlibfun = tlibfun$E
SRC.tlibfun$E = $(wildcard tests/stm32vldiscovery/07.libfun/*.c) \
	tests/stm32vldiscovery/hw.c
LIBS.tlibfun$E = cmsis$L ugears$L useful$L
