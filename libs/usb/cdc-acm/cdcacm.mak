ifneq ($(findstring /$(MCU.SERIES)/,/stm32f1/stm32f3/),)

# Apps and tests may depend on this variable
USB_CDC_ACM = 1

SRC.usb$L += $(wildcard libs/usb/cdc-acm/*.c)

endif
