# The directory where tibs makefiles are located
DIR.TIBS ?= tibs

# Initialize TIBS
include $(DIR.TIBS)/init.mak

# Project name and version
CONF_PACKAGE = ugears
CONF_VERSION = 0.1.0
# Default build mode
MODE = release
# Target architecture
ARCH = arm
# Target environment
TARGET = none-eabi

# Autodetect 'windows' build environment
ifndef HOST
ifneq ($(if $(ComSpec),$(ComSpec),$(COMSPEC)),)
HOST = win32
else
HOST = $(call ASCIILOW,$(shell uname -s))
endif
endif

CONF_VER_H = $(word 1,$(subst ., ,$(CONF_VERSION)))
CONF_VER_L = $(word 2,$(subst ., ,$(CONF_VERSION)))
CONF_VER_R = $(word 3,$(subst ., ,$(CONF_VERSION)))

# Build date
BDATE = $(shell date '+%Y-%m-%d %H:%M:%S')
# If RTC Clock uninitialized, initialize to this value (3 is TZ)
RTC_INITTIME = $(shell expr `date +%s` + 3 '*' 60 '*' 60)

# Local user file which can be used to override some of the settings (e.g. MODE)
-include local-config.mak

ifeq ($(ARCH),arm)

# Hardware definitions for STM32 microcontroller family
include $(DIR.TIBS)/extra/stm32/stm32.mak

# Use adapted output directory hierarchy
OUT = $(OUTBASE)/$(HARDWARE)/$(MODE)/

else

# Override HARDWARE to avoid board-specific stuff
HARDWARE = pc

endif

# We use non-standard prototypes for well-known libc functions
CFLAGS += -Wno-builtin-declaration-mismatch

# Additional compiler defines
DEFINES += \
	ARCH_$(call ASCIIUP,$(ARCH)) \
	TARGET_$(subst -,_,$(call ASCIIUP,$(TARGET))) \
	CONF_VER_H=$(CONF_VER_H) \
	CONF_VER_L=$(CONF_VER_L) \
	CONF_VER_R=$(CONF_VER_R) \
	RTC_INITTIME="$(RTC_INITTIME)-RTC_BASETIME" \
	BDATE="\"$(subst $(SPACE),;,$(BDATE))\""

# Additiona files (except sources) to include in distribution
DISTEXTRA += include/ libs/ tibs/ config.mak local-config-sample.mak

ifeq ($(ARCH),arm)

# Default toolkit
TOOLKIT ?= ARM-NONE-EABI-GCC

# Include the arm-none-eabi toolkit from rules.mak
SUBMAKEFILES += $(DIR.TIBS)/extra/stm32/arm-none-eabi-gcc.mak

else

TOOLKIT ?= GCC

endif

# The black magic that generates build rules
include $(DIR.TIBS)/rules.mak

ifeq ($(ARCH),arm)

# Additional build rules. These must always come last
include \
    $(DIR.TIBS)/extra/stm32/flash-rules.mak \
    $(DIR.TIBS)/extra/stm32/ihex-rules.mak \
    $(DIR.TIBS)/extra/stm32/bin-rules.mak

showhelp::
	$(call SAY,Target hardware: $(HARDWARE)$(COMMA) mcu: $(MCU.TYPE)$(COMMA) core: $(MCU.CORE))
	$(call SAY,$-)

endif
