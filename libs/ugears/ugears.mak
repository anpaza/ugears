# This library provides various unclassified utility functions

ifneq ($(MCU.BRAND),)

LIBS += ugears
DESCRIPTION.ugears = A thin shim around low-level platform hardware
TARGETS.ugears = ugears$L

SRC.ugears$L := $(wildcard libs/ugears/*.c libs/ugears/$(MCU.BRAND)/*.c libs/ugears/$(MCU.BRAND)/*.S)
LIBS.ugears$L = cmsis$L useful$L
CFLAGS.ugears$L = -Iinclude/ugears -Iinclude/ugears/$(MCU.BRAND)

# Include MCU.BRAND-specific makefile, if any
-include libs/ugears/$(MCU.BRAND)/ugears.mak

endif # ifneq ($(MCU.BRAND),)
