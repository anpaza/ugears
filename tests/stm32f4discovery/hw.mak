# This directory contains sample code for the STM32F4-DISCOVERY board

ifeq ($(HARDWARE),stm32f4discovery)
DIR.INCLUDE.C += tests/$(HARDWARE)
include $(wildcard tests/$(HARDWARE)/*/*.mak)
endif
