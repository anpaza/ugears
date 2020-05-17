ifeq ($(USB_CDC_ACM),zzzzzzzzz)

APPS += usbuart
DESCRIPTION.usbuart = USB-UART dongle from STM32!
FLASH.TARGETS += usbuart

TARGETS.usbuart = usbuart$E
SRC.usbuart$E = $(wildcard apps/usbuart/*.c)
LIBS.usbuart$E = cmsis$L ugears$L useful$L usb$L

endif
