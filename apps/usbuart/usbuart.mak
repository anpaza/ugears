ifneq ($(USB_CDC_ACM),)

APPS += usbuart
DESCRIPTION.usbuart = USB-UART dongle from STM32!
FLASH.TARGETS += usbuart
IHEX.TARGETS += usbuart

TARGETS.usbuart = usbuart$E
SRC.usbuart$E = $(wildcard apps/usbuart/*.c)
LIBS.usbuart$E = usb$L cmsis$L ugears$L useful$L

endif
