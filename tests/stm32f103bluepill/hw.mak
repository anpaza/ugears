# This directory contains sample code for the STM32VL-DISCOVERY board

ifeq ($(HARDWARE),stm32f103bluepill)
DIR.INCLUDE.C += tests/$(HARDWARE)
include $(wildcard tests/$(HARDWARE)/*/*.mak)
endif
