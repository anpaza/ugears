# uGears the thin microcontroller library

ifneq ($(MCU.BRAND),)

LIBS += ugears
DESCRIPTION.ugears = A thin shim around low-level platform hardware
TARGETS.ugears = ugears$L

SRC.ugears$L := $(wildcard libs/ugears/*.c libs/ugears/$(MCU.BRAND)/*.c libs/ugears/$(MCU.BRAND)/*.S)
LIBS.ugears$L = cmsis$L useful$L

# Include MCU.BRAND-specific makefile, if any
-include libs/ugears/$(MCU.BRAND)/ugears.mak

endif # ifneq ($(MCU.BRAND),)
