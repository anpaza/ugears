ifeq ($(ARCH),arm)

TESTS += tsh
DESCRIPTION.tsh = Test semi-hosting functions
FLASH.TARGETS += tsh
IHEX.TARGETS += tsh

TARGETS.tsh = tsh$E
SRC.tsh$E = $(wildcard tests/tsh/*.c)
LIBS.tsh$E = cmsis$L ugears$L useful$L

endif
