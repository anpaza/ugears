# USB library for microcontrollers

DESCRIPTION.usb = USB library
TARGETS.usb = usb$L

LIBS.usb$L += cmsis$L useful$L ugears$L

include $(wildcard libs/usb/*/*.mak)

ifneq ($(SRC.usb$L),)
LIBS += usb
endif
