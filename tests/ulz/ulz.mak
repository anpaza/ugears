# This is meant to be executed/debugged under Linux or Windows.
#
# Build it like this: make ARCH=x86_64 TARGET=posix TOOLKIT=GCC USING_LIBC=1 ulz

ifeq ($(TOOLKIT)-$(filter none-eabi,$(TARGET)),GCC-)
TESTS += ulz
DESCRIPTION.ulz = Test the uLZ de/compression routines

TARGETS.ulz = ulz$E
SRC.ulz$E = $(wildcard tests/ulz/*.c)
LIBS.ulz$E = useful$L
endif
