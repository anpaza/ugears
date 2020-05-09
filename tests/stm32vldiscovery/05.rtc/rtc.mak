TESTS += trtc
DESCRIPTION.trtc = Test for Real-Time-Clock library
FLASH.TARGETS += trtc
IHEX.TARGETS += trtc

TARGETS.trtc = trtc$E
SRC.trtc$E = $(wildcard tests/stm32vldiscovery/05.rtc/*.c) \
	tests/stm32vldiscovery/stm32vldiscovery.c
LIBS.trtc$E = cmsis$L ugears$L useful$L
