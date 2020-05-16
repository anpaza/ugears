TESTS += tusbcdc
DESCRIPTION.tusbcdc = Test the USB-CDC driver from
FLASH.TARGETS += tusbcdc
IHEX.TARGETS += tusbcdc

TARGETS.tusbcdc = tusbcdc$E
SRC.tusbcdc$E = $(wildcard tests/stm32f103bluepill/07.usbcdc/*.c) \
	tests/stm32f103bluepill/hw.c
LIBS.tusbcdc$E = usb$L cmsis$L ugears$L useful$L
