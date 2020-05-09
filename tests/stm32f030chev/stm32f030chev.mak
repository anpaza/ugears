# This directory contains sample code for the STM32F030CHEV board

ifeq ($(HARDWARE),stm32f030chev)
DIR.INCLUDE.C += tests/$(HARDWARE)
include $(wildcard tests/$(HARDWARE)/*/*.mak)
endif
