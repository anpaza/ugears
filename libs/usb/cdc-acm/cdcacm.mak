ifneq ($(findstring /$(MCU.SERIES)/,/stm32f1/stm32f3/),)

SRC.usb$L += $(wildcard libs/usb/cdc-acm/*.c)

endif
