# This is meant to be executed/debugged under Linux or Windows.
#
# Build it like this: make ARCH=x86_64 TARGET=posix TOOLKIT=GCC atomic

ifeq ($(TOOLKIT)-$(filter none-eabi,$(TARGET)),GCC-)
TESTS += atomic
DESCRIPTION.atomic = Test atomic macros in a friendly environment

TARGETS.atomic = atomic$E
SRC.atomic$E = $(wildcard tests/atomic/*.c)
LIBS.atomic$E = useful$L
endif
