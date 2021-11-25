# Build with: make TARGET=posix ARCH=x86_64 ...

ifeq ($(TARGET),posix)

TESTS += tmemset
DESCRIPTION.tmemset = Check implementation of memset() in libuseful

TARGETS.tmemset = tmemset$E
SRC.tmemset$E = $(wildcard tests/tmemset/*.c)
LIBS.tmemset$E = useful$L

endif
