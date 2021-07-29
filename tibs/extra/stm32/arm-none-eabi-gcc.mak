# tibs macros for arm-none-eabi-gcc toolkit

ifeq ($(ARCH),arm)

.SUFFIXES: .c .cpp .o .lo .a .pc .pc.in

# Language standards
C.STD ?= gnu99
CXX.STD ?= c++11

ARM-NONE-EABI-GCC.PFX ?= arm-none-eabi-

ARM-NONE-EABI-GCC.CC ?= $(ARM-NONE-EABI-GCC.PFX)gcc -c -std=$(C.STD)
ARM-NONE-EABI-GCC.CFLAGS ?= -pipe -Wall -Wextra -Werror -MMD \
    -fno-common -ftrack-macro-expansion=0 \
    $(ARM-NONE-EABI-GCC.CFLAGS.$(MCU.CORE)) \
    $(ARM-NONE-EABI-GCC.CFLAGS.$(MODE)) \
    $(ARM-NONE-EABI-GCC.CFLAGS.DEF) $(ARM-NONE-EABI-GCC.CFLAGS.INC) $(CFLAGS)
ARM-NONE-EABI-GCC.SFLAGS ?= -pipe -D__ASSEMBLY__ -D__ASSEMBLER__ \
    $(ARM-NONE-EABI-GCC.CFLAGS.$(MCU.CORE)) \
    $(ARM-NONE-EABI-GCC.CFLAGS.$(MODE)) \
    $(ARM-NONE-EABI-GCC.CFLAGS.DEF) $(ARM-NONE-EABI-GCC.CFLAGS.INC) $(CFLAGS)
ARM-NONE-EABI-GCC.CFLAGS.DEF = $(CFLAGS.DEF) $(addprefix -D,$(DEFINES))
ARM-NONE-EABI-GCC.CFLAGS.INC = $(addprefix -I,$(DIR.INCLUDE.C))

# You might want to use -Os instead of -O2, depending on your priorities
ARM-NONE-EABI-GCC.CFLAGS.release ?= -g3 -O2 -mabi=aapcs -fomit-frame-pointer \
    -fno-builtin -ffunction-sections -fdata-sections \
    -fpeel-loops -ffast-math $(ARM-NONE-EABI-GCC.FLTO)
ARM-NONE-EABI-GCC.CFLAGS.debug ?= -g3 -Og -D__DEBUG__ -mabi=aapcs \
    -fno-builtin -ffunction-sections -fdata-sections \
    -ffast-math

ARM-NONE-EABI-GCC.CFLAGS.cortex-m0 = -mcpu=cortex-m0 -mthumb $(if $(MCU.FPU),-mfpu=$(MCU.FPU) -mfloat-abi=hard)
ARM-NONE-EABI-GCC.CFLAGS.cortex-m3 = -mcpu=cortex-m3 -mthumb $(if $(MCU.FPU),-mfpu=$(MCU.FPU) -mfloat-abi=hard)
ARM-NONE-EABI-GCC.CFLAGS.cortex-m4 = -mcpu=cortex-m4 -mthumb $(if $(MCU.FPU),-mfpu=$(MCU.FPU) -mfloat-abi=hard)
ARM-NONE-EABI-GCC.CFLAGS.cortex-m7 = -mcpu=cortex-m7 -mthumb $(if $(MCU.FPU),-mfpu=$(MCU.FPU) -mfloat-abi=hard)

ifeq ($(ARM-NONE-EABI-GCC.CFLAGS.$(MCU.CORE)),)
$(error "Unknown MCU core $(MCU.CORE), cannot set -mcpu= C compiler flag")
endif

ARM-NONE-EABI-GCC.CXX.OK := $(shell which $(ARM-NONE-EABI-GCC.PFX)g++ 2>/dev/null)

ifneq ($(ARM-NONE-EABI-GCC.CXX.OK),)
ARM-NONE-EABI-GCC.CXX ?= $(ARM-NONE-EABI-GCC.PFX)g++ -c -std=$(CXX.STD)
ARM-NONE-EABI-GCC.CXXFLAGS ?= $(ARM-NONE-EABI-GCC.CFLAGS) $(CXXFLAGS) \
    -fno-exceptions -fno-rtti
else
ARM-NONE-EABI-GCC.CXX ?= echo "C++ compiler is not installed"; false
endif

ARM-NONE-EABI-GCC.CPP ?= $(ARM-NONE-EABI-GCC.PFX)gcc -E
ARM-NONE-EABI-GCC.CPPFLAGS ?= -pipe -x c-header $(ARM-NONE-EABI-GCC.CFLAGS.DEF) $(ARM-NONE-EABI-GCC.CFLAGS.INC)

ARM-NONE-EABI-GCC.LD ?= $(ARM-NONE-EABI-GCC.PFX)gcc
ARM-NONE-EABI-GCC.LDFLAGS ?= -pipe $(ARM-NONE-EABI-GCC.CFLAGS.$(MCU.CORE)) \
    -Wl,--gc-sections -mabi=aapcs -nostartfiles -nostdlib \
    $(ARM-NONE-EABI-GCC.LDFLAGS.$(MODE))
ARM-NONE-EABI-GCC.LDFLAGS.LIBS ?= $(LDLIBS) -lgcc

ARM-NONE-EABI-GCC.LDFLAGS.release ?= -g3 $(ARM-NONE-EABI-GCC.FLTO)
ARM-NONE-EABI-GCC.LDFLAGS.debug ?= -g3

ARM-NONE-EABI-GCC.LINKLIB = $(if $(findstring $L,$1),,$(if $(findstring /,$1),$1,-l$1))

ARM-NONE-EABI-GCC.AR ?= $(ARM-NONE-EABI-GCC.PFX)ar
ARM-NONE-EABI-GCC.ARFLAGS ?= crs

ARM-NONE-EABI-GCC.FLASH ?= st-flash
ifneq ($(findstring openocd,$(ARM-NONE-EABI-GCC.FLASH)),)
ARM-NONE-EABI-GCC.FLASHFLAGS ?= $(OPENOCD.FLAGS)
FLASH.ARM-NONE-EABI-GCC ?= $(ARM-NONE-EABI-GCC.FLASH) $2 -c "source [find board/stm32vldiscovery.cfg]" -c "program $1 reset exit $(STM32.FLASH.ORIGIN)"
else
# Use st-util by default
ARM-NONE-EABI-GCC.FLASHFLAGS ?= $(STM32.FLASH.ORIGIN)
FLASH.ARM-NONE-EABI-GCC ?= $(ARM-NONE-EABI-GCC.FLASH) write $1 $(if $2,$2,$(ARM-NONE-EABI-GCC.FLASHFLAGS))
endif

ARM-NONE-EABI-GCC.OBJCOPY ?= $(ARM-NONE-EABI-GCC.PFX)objcopy
ARM-NONE-EABI-GCC.OCFLAGS ?= -j .isr_vector -j .text -j .ARM.extab -j .preinit_array -j .init_array -j .fini_array -j .data

# Translate application/library pseudo-name into an actual file name
XFNAME.ARM-NONE-EABI-GCC = $(addprefix $$(OUT),\
    $(patsubst %$E,%$(_EX),\
    $(if $(findstring $L,$1),$(addprefix lib,$(patsubst %$L,%.a,$1)),$1)\
))

MKDEPS.ARM-NONE-EABI-GCC = \
	$(call MKDEPS.DEFAULT,\
	__libc_init_array.o \
	$(patsubst %.c,%.o,\
	$(patsubst %.cpp,%.o,\
	$(patsubst %.asm,%.o,\
	$(patsubst %.S,%.o,\
	$(patsubst %.s,%.o,\
	$1))))))

MKDEPSD.ARM-NONE-EABI-GCC = \
	$(call MKDEPSD.DEFAULT,\
	$(patsubst %.c,%.d,\
	$(patsubst %.cpp,%.d,\
	$(patsubst %.S,%.d,\
	$(filter %.c %.cpp %.S,$1))))

COMPILE.ARM-NONE-EABI-GCC.CXX  = $(ARM-NONE-EABI-GCC.CXX) -o $@ $(strip $(ARM-NONE-EABI-GCC.CXXFLAGS) $1) $<
COMPILE.ARM-NONE-EABI-GCC.CC   = $(ARM-NONE-EABI-GCC.CC) -o $@ $(strip $(ARM-NONE-EABI-GCC.CFLAGS) $1) $<
COMPILE.ARM-NONE-EABI-GCC.S    = $(ARM-NONE-EABI-GCC.CC) -o $@ $(strip $(ARM-NONE-EABI-GCC.SFLAGS) $1) $<

# Compilation rules ($1 = source file list, $2 = source file directories,
# $3 = module name, $4 = target name)
define MKCRULES.ARM-NONE-EABI-GCC
$(if $(filter %.c,$1),$(foreach z,$2,
$(addsuffix %.o,$(addprefix $$(OUT),$z)): $(addsuffix %.c,$z)
	$(if $V,,@echo COMPILE.ARM-NONE-EABI-GCC.CC $$< &&)\
	$$(call COMPILE.ARM-NONE-EABI-GCC.CC,$(CFLAGS.$3) $(CFLAGS.$4) $(call .LIBFLAGS,CFLAGS,$3,$4))))
$(if $(filter %.cpp,$1),$(foreach z,$2,
$(addsuffix %.o,$(addprefix $$(OUT),$z)): $(addsuffix %.cpp,$z)
	$(if $V,,@echo COMPILE.ARM-NONE-EABI-GCC.CXX $$< &&)\
	$$(call COMPILE.ARM-NONE-EABI-GCC.CXX,$(CXXFLAGS.$3) $(CXXFLAGS.$4) $(call .LIBFLAGS,CFLAGS,$3,$4))))
$(if $(filter %.S,$1),$(foreach z,$2,
$(addsuffix %.o,$(addprefix $$(OUT),$z)): $(addsuffix %.S,$z)
	$(if $V,,@echo COMPILE.ARM-NONE-EABI-GCC.S $$< &&)\
	$$(call COMPILE.ARM-NONE-EABI-GCC.S,$(CFLAGS.$3) $(CFLAGS.$4) $(call .LIBFLAGS,CFLAGS,$3,$4))))
$(if $(filter %.s,$1),$(foreach z,$2,
$(addsuffix %.o,$(addprefix $$(OUT),$z)): $(addsuffix %.s,$z)
	$(if $V,,@echo COMPILE.ARM-NONE-EABI-GCC.S $$< &&)\
	$$(call COMPILE.ARM-NONE-EABI-GCC.S,$(CFLAGS.$3) $(CFLAGS.$4) $(call .LIBFLAGS,CFLAGS,$3,$4))))
$(foreach z,$2,
$(addsuffix %.d,$(addprefix $$(OUT),$z)): $(addsuffix %.o,$(addprefix $$(OUT),$z)))
$(ARM-NONE-EABI-GCC.EXTRA.MKCRULES)
endef

LINK.ARM-NONE-EABI-GCC.AR = $(ARM-NONE-EABI-GCC.AR) $(ARM-NONE-EABI-GCC.ARFLAGS) $@ $^
define LINK.ARM-NONE-EABI-GCC.EXEC
    $(ARM-NONE-EABI-GCC.LD) -o $@ \
        $(if $(filter %.ld,$^),-Wl$(COMMA)-T$(filter %.ld,$^)) \
        $(ARM-NONE-EABI-GCC.LDFLAGS) $(LDFLAGS) $1 $(filter-out %.ld,$^) \
        $(ARM-NONE-EABI-GCC.LDFLAGS.LIBS) $(LDFLAGS.LIBS) $2 -Wl,-Map,$@.map
    size $@
endef

# Linking rules ($1 = target full filename, $2 = dependency list,
# $3 = module name, $4 = unexpanded target name)
define MKLRULES.ARM-NONE-EABI-GCC
$1: $2\
$(if $(findstring $L,$4),
	$(if $V,,@echo LINK.ARM-NONE-EABI-GCC.AR $$@ &&)$$(LINK.ARM-NONE-EABI-GCC.AR))\
$(if $(findstring $E,$4), $(ARM-NONE-EABI-GCC.LDSCRIPT)
	$(if $V,,@echo LINK.ARM-NONE-EABI-GCC.EXEC $$@ &&)\
	$$(call LINK.ARM-NONE-EABI-GCC.EXEC,$(subst $(COMMA),$$(COMMA),$(LDFLAGS.$3) $(LDFLAGS.$4)) $(call .LIBFLAGS,LDLIBS,$3,$4),$(foreach z,$(LIBS.$3) $(LIBS.$4),$(call ARM-NONE-EABI-GCC.LINKLIB,$z))))
$(ARM-NONE-EABI-GCC.EXTRA.MKLRULES)
endef

# Install rules ($1 = module name, $2 = unexpanded target file,
# $3 = full target file name)
define MKIRULES.ARM-NONE-EABI-GCC
$(if $(findstring $L,$2),\
$(foreach _,$3,
	$(if $V,,@echo INSTALL $_ to $(call .INSTDIR,$1,$2,LIB,$(CONF_LIBDIR)) &&)\
	$$(call INSTALL,$_,$(call .INSTDIR,$1,$2,LIB,$(CONF_LIBDIR)),0644)))\
$(if $(findstring $E,$2),
	$(if $V,,@echo INSTALL $3 to $(call .INSTDIR,$1,$2,BIN,$(CONF_BINDIR)) &&)\
	$$(call INSTALL,$3,$(call .INSTDIR,$1,$2,BIN,$(CONF_BINDIR)),0755))\
$(if $(INSTALL.INCLUDE.$2),
	$(if $V,,@echo INSTALL $(INSTALL.INCLUDE.$2) to $(call .INSTDIR,$1,$2,INCLUDE,$(CONF_INCLUDEDIR)) &&)\
	$$(call INSTALL,$(INSTALL.INCLUDE.$2),$(call .INSTDIR,$1,$2,INCLUDE,$(CONF_INCLUDEDIR)),0644))
endef

# Flashing rules ($1 = module name, $2 = unexpanded target file,
# $3 = full target file name)
define MKFRULES.ARM-NONE-EABI-GCC
$(if $(filter %$E,$2),
	$(if $V,,@echo FLASH.ARM-NONE-EABI-GCC $3 &&)\
	$$(call FLASH.ARM-NONE-EABI-GCC,$3,$(strip $(FLASHFLAGS.$1) $(FLASHFLAGS.$2))))
endef

LINK.ARM-NONE-EABI-GCC.HEX = $(ARM-NONE-EABI-GCC.OBJCOPY) $< $(ARM-NONE-EABI-GCC.OCFLAGS) $1 -O ihex $@

# iHEXing rules ($1 = module name, $2 = unexpanded target file,
# $3 = full target file name, $4 = executable name)
define MKIXRULES.ARM-NONE-EABI-GCC
$(if $(findstring $E,$2),
$3: $4
	$(if $V,,@echo LINK.ARM-NONE-EABI-GCC.HEX $$@ &&)\
	$$(call LINK.ARM-NONE-EABI-GCC.HEX,$(OCFLAGS.$3) $(OCFLAGS.$4)))
endef

LINK.ARM-NONE-EABI-GCC.BIN ?= $(ARM-NONE-EABI-GCC.OBJCOPY) $< $(ARM-NONE-EABI-GCC.OCFLAGS) $1 -O binary $@

# Rules to build binary images ($1 = module name, $2 = unexpanded target file,
# $3 = full target file name, $4 = executable name)
define MKBINRULES.ARM-NONE-EABI-GCC
$(if $(findstring $E,$2),
$3: $4
	$(if $V,,@echo LINK.ARM-NONE-EABI-GCC.BIN $$@ &&)\
	$$(call LINK.ARM-NONE-EABI-GCC.BIN,$(OCFLAGS.$3) $(OCFLAGS.$4),$(strip $(FLASHFLAGS.$1) $(FLASHFLAGS.$2))))
endef

# Правила генерации скрипта для линкера
ARM-NONE-EABI-GCC.LDSCRIPT = $(OUT)stm32_flash.ld

$(ARM-NONE-EABI-GCC.LDSCRIPT): $(DIR.TIBS)/extra/stm32/flash.ld.in
	$(if $V,,@echo ARM-NONE-EABI-GCC.CPP $@ &&)$(ARM-NONE-EABI-GCC.CPP) \
		$(ARM-NONE-EABI-GCC.CPPFLAGS) -P -o $@ $<

$(OUT)__libc_init_array.o: $(DIR.TIBS)/extra/stm32/__libc_init_array.c
	$(if $V,,@echo COMPILE.ARM-NONE-EABI-GCC.CC $< &&)$(call COMPILE.ARM-NONE-EABI-GCC.CC)

endif # ifeq ($(ARCH),arm)
