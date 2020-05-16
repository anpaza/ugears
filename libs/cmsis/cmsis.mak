# CMSIS library

DESCRIPTION.cmsis = CMSIS library
TARGETS.cmsis = cmsis$L


SRC.cmsis$L =
CFLAGS.cmsis$L += -Iinclude/cmsis

ifeq ($(MCU.BRAND),stm32)
CFLAGS.cmsis$L += -Iinclude/cmsis/stm32
_ := $(strip $(foreach x,$(call ASCIILOW,$(MCU.DEFINES)),$(wildcard libs/cmsis/stm32/startup_$x.s)))
ifneq ($_,)
SRC.cmsis$L += $_
else
$(warning Startup file for MCU $(MCU.TYPE) not found)
endif
endif

# Add to list of buildtable targets only if there are source files for current target
ifneq ($(SRC.cmsis$L),)
LIBS += cmsis
endif
