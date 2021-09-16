TESTS += tmemchr
DESCRIPTION.tmemchr = Проверка корректности реализации функции memchr()

TARGETS.tmemchr = tmemchr$E
SRC.tmemchr$E = $(wildcard tests/tmemchr/*.c)
LIBS.tmemchr$E = useful$L
