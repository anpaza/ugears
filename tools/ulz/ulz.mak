# This is meant to be executed/debugged under Linux or Windows.
#
# Build it like this:
# make ARCH=x86_64 TARGET=posix TOOLKIT=GCC DEFINES="USE_LIBC _GNU_SOURCE" ulz

ifeq ($(TOOLKIT)-$(filter none-eabi,$(TARGET)),GCC-)
TOOLS += ulz
DESCRIPTION.ulz = Test the uLZ de/compression routines

TARGETS.ulz = ulz$E
SRC.ulz$E = $(wildcard tools/ulz/*.c)
LIBS.ulz$E = useful$L
endif
