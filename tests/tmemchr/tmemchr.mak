# Build with: make TARGET=posix ARCH=x86_64 ...

ifeq ($(TARGET),posix)

TESTS += tmemchr
DESCRIPTION.tmemchr = Проверка корректности реализации функции memchr()

TARGETS.tmemchr = tmemchr$E
SRC.tmemchr$E = $(wildcard tests/tmemchr/*.c)
LIBS.tmemchr$E = useful$L

endif
