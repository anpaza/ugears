# CMSIS library

DESCRIPTION.cmsis = CMSIS library
TARGETS.cmsis = cmsis$L


SRC.cmsis$L =
CFLAGS.cmsis$L += -Iinclude/cmsis

ifeq ($(MCU.BRAND),stm32)
CFLAGS.cmsis$L += -Iinclude/cmsis/stm32
_ := $(strip $(foreach x,$(call asciidown,$(MCU.DEFINES)),$(wildcard libs/cmsis/stm32/startup_$x.s)))
ifeq ($_,)
$(error Startup file for MCU $(MCU.TYPE) not found)
endif
SRC.cmsis$L += $_
endif

# Add to list of buildtable targets only if there are source files for current target
ifneq ($(SRC.cmsis$L),)
LIBS += cmsis
endif
