# Build with: make TARGET=posix ARCH=x86_64 ...

ifeq ($(TARGET),posix)

TESTS += tmemchr
DESCRIPTION.tmemchr = Check implementation of memchr() in libuseful

TARGETS.tmemchr = tmemchr$E
SRC.tmemchr$E = $(wildcard tests/tmemchr/*.c)
LIBS.tmemchr$E = useful$L

endif
