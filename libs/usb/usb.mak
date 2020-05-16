# USB library for microcontrollers

DESCRIPTION.usb = USB library
TARGETS.usb = usb$L

CFLAGS.usb$L = -Iinclude/usb
LIBS.usb$L += cmsis$L useful$L ugears$L

include $(wildcard libs/usb/*/*.mak)

ifneq ($(SRC.usb$L),)
LIBS += usb
endif
