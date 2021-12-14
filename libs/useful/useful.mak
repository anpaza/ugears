# This library provides various unclassified utility functions

LIBS += useful
DESCRIPTION.useful = A library with useful mostly hardware-independent functions
TARGETS.useful = useful$L

SRC.useful$L := $(wildcard libs/useful/*.c)

# Choose from alternative implementations the one that fits best current target
useful.ALTDIR = c $(ARCH)
useful.ALTFUN = semihosting memcpy memcmp memset memchr memrchr strlen assert_abort \
    strcpy strncpy

ifeq ($(MCU.BRAND),stm32)
useful.ALTDIR += thumb
endif

define useful.FINDFILE
$(eval _fn=)\
$(foreach _,$(useful.ALTDIR),$(call CASSIGN,_fn,$(wildcard libs/useful/$_/$1.*)))\
$(_fn)
endef

SRC.useful$L += $(foreach _,$(useful.ALTFUN),$(call useful.FINDFILE,$_))
