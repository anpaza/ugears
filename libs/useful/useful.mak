# This library provides various unclassified utility functions

LIBS += useful
DESCRIPTION.useful = A library with useful mostly hardware-independent functions
TARGETS.useful = useful$L

CFLAGS.useful$L = -Iinclude/useful
SRC.useful$L := $(wildcard libs/useful/*.c libs/useful/$(ARCH)/*.c)
ifeq ($(MCU.BRAND),stm32)
SRC.useful$L += $(wildcard libs/useful/thumb/*.S)
endif
