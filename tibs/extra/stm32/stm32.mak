# Various macros for the STM32 MCU family.
#
# MCU.BRAND = stm32
# MCU.TYPE = one of values from first column of stm32-mcu.csv.
#       Example: STM32F103CBTx, STM32F407VGTx etc
# MCU.CORE = Core architecture of the CPU, third column of stm32-mcu.csv.
#       Example: cortex-m3, cortex-m0plus etc
# MCU.FPU = Floating-point unit type or 'None'
#       Example: fpv4-sp-d16, fpv5-d16 etc
# MCU.SERIES = Microcontroller series
#       Example: stm32f0, stm32u5 etc
# MCU.PACKAGE = microcontroller package
#       Example: LQFP48, TFBGA100 etc
#
# The following macros are also passed to source code as preprocessor defines,
# uppercased and with dots and dashes replaced by underscores:
#
# MCU_CORE, MCU_FPU
#

# Hardware name must be always defined
ifndef HARDWARE
$(error HARDWARE not defined, please configure the project properly!)
endif

HARDWARE_H = hardware/$(HARDWARE).h
ifeq ($(wildcard include/$(HARDWARE_H)),)
$(error Hardware definition file $(HARDWARE_H) does not exits!)
endif

# If MCU type not defined, try to find it by board name
ifeq ($(MCU.TYPE),)
MCU.TYPE = $(word 2,$(subst $(COMMA),$(SPACE),$(shell grep -i '^$(HARDWARE),' $(DIR.TIBS)/extra/stm32/board.csv)))
ifeq ($(MCU.TYPE),)
$(error Board $(HARDWARE) not defined in $(DIR.TIBS)/extra/stm32/board.csv!)
endif
endif

# Just in case, always use lower case
HARDWARE := $(call ASCIILOW,$(HARDWARE))
MCU.TYPE := $(call ASCIILOW,$(MCU.TYPE))

# Replace empty fields (",,") with "None" (",None,") otherwise we'll lose correct field numbering
MCU_DESC = $(subst $(COMMA),$(SPACE),\
           $(subst $(COMMA)$(COMMA),$(COMMA)None$(COMMA),\
           $(shell grep -i '^$(MCU.TYPE),' $(DIR.TIBS)/extra/stm32/mcu.csv)))
ifeq ($(MCU_DESC),)
$(error MCU type $(MCU.TYPE) not defined in $(DIR.TIBS)/extra/stm32/mcu.csv!)
endif

# Extract fields from CSV
MCU.BRAND = stm32
MCU.SERIES = $(word 2,$(MCU_DESC))
MCU.CORE = $(word 3,$(MCU_DESC))
MCU.FPU = $(word 4,$(MCU_DESC))
MCU.PACKAGE = $(word 5,$(MCU_DESC))
MCU.DEFINES = $(subst ;,$(SPACE),$(word 6,$(MCU_DESC)))

ifeq ($(MCU.FPU),None)
MCU.FPU =
endif

STM32.RAM.ORIGIN = $(word 7,$(MCU_DESC))
STM32.RAM.SIZE = $(word 8,$(MCU_DESC))
STM32.RAM.END = $(word 9,$(MCU_DESC))
STM32.FLASH.ORIGIN = $(word 10,$(MCU_DESC))
STM32.FLASH.SIZE = $(word 11,$(MCU_DESC))
STM32.FLASH.END = $(word 12,$(MCU_DESC))

# User may override these, if needed
STM32.HEAP.MIN_SIZE ?= 0
STM32STACK.MIN_SIZE ?= 0x200

# Pass MCU definitions to compiler
DEFINES += $(MCU.DEFINES) \
	$(call ASCIIUP,$(subst -,_,$(MCU.CORE))) \
	$(if $(MCU.FPU),$(call ASCIIUP,$(subst -,_,$(MCU.FPU)))) \
	STM32_STACK_ADDRESS=$(STM32.RAM.END) \
	STM32_MIN_HEAP_SIZE=$(STM32.HEAP.MIN_SIZE) \
	STM32_MIN_STACK_SIZE=$(STM32STACK.MIN_SIZE) \
	STM32_FLASH_ORIGIN=$(STM32.FLASH.ORIGIN) \
	STM32_FLASH_SIZE=$(STM32.FLASH.SIZE) \
	STM32_RAM_ORIGIN=$(STM32.RAM.ORIGIN) \
	STM32_RAM_SIZE=$(STM32.RAM.SIZE) \
	HARDWARE_H=\"$(HARDWARE_H)\"
