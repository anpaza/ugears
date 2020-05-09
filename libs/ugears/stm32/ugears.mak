# MCU startup file: interrupt vector table, pre-main() initialization
_ := $(wildcard libs/ugears/$(MCU.BRAND)/clock/$(MCU.SERIES).c)
ifeq ($_,)
$(error Cannot find clock setup routines for $(MCU.TYPE): ($(MCU.SERIES).c))
endif
SRC.ugears$L += $_
