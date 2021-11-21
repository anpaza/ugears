# GNU compiler suite definitions

.SUFFIXES: .c .cpp .o .lo .a .pc .pc.in

GCC.CC ?= $(GCC.PREFIX)gcc -c
GCC.CFLAGS += -pipe -MMD -Wall \
    $(GCC.CFLAGS.$(MODE)) $(GCC.CFLAGS.DEF) $(GCC.CFLAGS.INC) $(CFLAGS)
GCC.CFLAGS.DEF = $(CFLAGS.DEF) $(addprefix -D,$(DEFINES))
GCC.CFLAGS.INC = $(addprefix -I,$(DIR.INCLUDE.C))

ifneq ($(TARGET),windows)
    GCC.CFLAGS.SHARED ?= -fPIC
else
    GCC.CFLAGS.SHARED ?= -D__DLL_EXPORT__
endif

GCC.CFLAGS.release ?= -s -O3 -fomit-frame-pointer -funroll-loops
GCC.CFLAGS.debug ?= -D__DEBUG__ -g


GCC.ASFLAGS += -pipe -MMD -D__ASSEMBLY__ \
    $(GCC.CFLAGS.DEF) $(GCC.CFLAGS.INC) $(CFLAGS)
GCC.ASFLAGS.SHARED ?= $(GCC.CFLAGS.SHARED)

GCC.CXX ?= $(GCC.PREFIX)g++ -c
GCC.CXXFLAGS += $(GCC.CFLAGS) $(CXXFLAGS)
GCC.CXXFLAGS.SHARED = $(GCC.CFLAGS.SHARED)

GCC.CPP = $(GCC.PREFIX)gcc -E
GCC.CPPFLAGS += -pipe -x c-header $(GCC.CFLAGS.DEF) $(GCC.CFLAGS.INC) $(CPPFLAGS)

GCC.LD ?= $(GCC.PREFIX)g++
GCC.LDFLAGS += $(GCC.LDFLAGS.$(MODE)) $(LDFLAGS)
GCC.LDFLAGS.LIBS = $(LDLIBS) -lm

GCC.LDFLAGS.release = -s
GCC.LDFLAGS.debug = -gdwarf-2 -g3

GCC.LINKLIB = $(if $(findstring $L,$1),,$(if $(findstring /,$1),$1,-l$1))

GCC.AR ?= $(GCC.PREFIX)ar
GCC.ARFLAGS = crs

# Translate application/library pseudo-name into an actual file name
XFNAME.GCC = $(addprefix $$(OUT),\
    $(strip $(patsubst %$E,%$(_EX),\
    $(if $(findstring $L,$1),\
        $(if $(subst STATIC,,$(subst static,,$(SHARED.$1))),\
            $(addprefix $(if $(LIBPREFIX.$1),$(LIBPREFIX.$1),$(SO_)),$(patsubst %$L,%$(_SO),$1)),\
            $(addprefix $(if $(LIBPREFIX.$1),$(LIBPREFIX.$1),lib),$(patsubst %$L,%.a,$1))\
        ),\
    $1)\
)))

MKDEPS.GCC = \
	$(call MKDEPS.DEFAULT,\
	$(patsubst %.c,$(if $(SHARED.$2),%.lo,%.o),\
	$(patsubst %.cpp,$(if $(SHARED.$2),%.lo,%.o),\
	$(patsubst %.asm,$(if $(SHARED.$2),%.lo,%.o),\
	$(patsubst %.S,$(if $(SHARED.$2),%.lo,%.o),\
	$1)))))

MKDEPSD.ARM-NONE-EABI-GCC = \
	$(call MKDEPSD.DEFAULT,\
	$(patsubst %.c,%.d,\
	$(patsubst %.cpp,%.d,\
	$(patsubst %.S,%.d,\
	$(filter %.c %.cpp %.S,$1)))))

COMPILE.GCC.CXX  = $(GCC.CXX) -o $@ $(strip $(GCC.CXXFLAGS) $1) $<
COMPILE.GCC.CC   = $(GCC.CC) -o $@ $(strip $(GCC.CFLAGS) $1) $<
COMPILE.GCC.S    = $(GCC.CC) -o $@ $(strip $(GCC.ASFLAGS) $1) $<

# Compilation rules ($1 = source file list, $2 = source file directories,
# $3 = module name, $4 = target name)
define MKCRULES.GCC
$(if $(filter %.c,$1),$(foreach z,$2,
$(addsuffix $(if $(SHARED.$4),%.lo,%.o),$(addprefix $$(OUT),$z)): $(addsuffix %.c,$z)
	$(if $V,,@echo COMPILE.GCC.CC$(if $(SHARED.$4),.SHARED) $$< &&)$$(call COMPILE.GCC.CC,$(CFLAGS.$3) $(CFLAGS.$4) $(if $(SHARED.$4),$(GCC.CFLAGS.SHARED)) $(call .LIBFLAGS,CFLAGS,$3,$4))))
$(if $(filter %.cpp,$1),$(foreach z,$2,
$(addsuffix $(if $(SHARED.$4),%.lo,%.o),$(addprefix $$(OUT),$z)): $(addsuffix %.cpp,$z)
	$(if $V,,@echo COMPILE.GCC.CXX$(if $(SHARED.$4),.SHARED) $$< &&)$$(call COMPILE.GCC.CXX,$(CXXFLAGS.$3) $(CXXFLAGS.$4) $(if $(SHARED.$4),$(GCC.CXXFLAGS.SHARED)) $(call .LIBFLAGS,CFLAGS,$3,$4))))
$(if $(filter %.S,$1),$(foreach z,$2,
$(addsuffix $(if $(SHARED.$4),%.lo,%.o),$(addprefix $$(OUT),$z)): $(addsuffix %.S,$z)
	$(if $V,,@echo COMPILE.GCC.S$(if $(SHARED.$4),.SHARED) $$< &&)$$(call COMPILE.GCC.S,$(ASFLAGS.$3) $(ASFLAGS.$4) $(if $(SHARED.$4),$(GCC.ASFLAGS.SHARED)) $(call .LIBFLAGS,CFLAGS,$3,$4))))
$(GCC.EXTRA.MKCRULES)
endef

LINK.GCC.AR = $(GCC.AR) $(GCC.ARFLAGS) $@ $^
LINK.GCC.EXEC = $(GCC.LD) -o $@ $(GCC.LDFLAGS) $1 $^ $(GCC.LDFLAGS.LIBS) $(LDFLAGS.LIBS) $2
define LINK.GCC.SO.VER
	$(GCC.LD) -o $@.$(SHARED.$3) $(call GCC.LDFLAGS.SHARED,$(notdir $@).$(basename $(basename $(SHARED.$3))),$(dir $@)) $(GCC.LDFLAGS) $1 $^ $(GCC.LDFLAGS.LIBS) $(LDFLAGS.LIBS) $2
	ln -fs $(notdir $@.$(SHARED.$3)) $@.$(basename $(basename $(SHARED.$3)))
	ln -fs $(notdir $@.$(basename $(basename $(SHARED.$3)))) $@
endef
define LINK.GCC.SO.NOVER
	$(GCC.LD) -o $@ $(call GCC.LDFLAGS.SHARED,$(notdir $@),$(dir $@)) $(GCC.LDFLAGS) $1 $^ $(GCC.LDFLAGS.LIBS) $(LDFLAGS.LIBS) $2
endef
# If SHARED.$3 equals 'static' or 'STATIC', create a static library from PIC
# object files (these libraries are supposed to be linked into other shared libs).
# If SHARED.$3 is a valid version number and target platform supports versioned
# shared libraries, invokde LINK.GCC.SO.VER.
# Otherwise create a non-versioned variant of the shared library
define LINK.GCC.SO
$(if $(findstring /$(SHARED.$3)/,/STATIC/static/),$(call LINK.GCC.AR),$(call LINK.GCC.SO.$(if $(call VALID_VERSION,$(SHARED.$3)),,NO)VER,$1,$2,$3))
endef

# Linking rules ($1 = target full filename, $2 = dependency list,
# $3 = module name, $4 = unexpanded target name)
define MKLRULES.GCC
$1: $2\
$(if $(findstring $L,$4),\
$(if $(SHARED.$4),
	$(if $V,,@echo LINK.GCC.SHARED $$@ &&)$$(call LINK.GCC.SO,$(subst $(COMMA),$$(COMMA),$(LDFLAGS.$3) $(LDFLAGS.$4)),$(call .LIBFLAGS,LDLIBS,$3,$4) $(foreach z,$(LIBS.$3) $(LIBS.$4),$(call GCC.LINKLIB,$z)),$4),
	$(if $V,,@echo LINK.GCC.AR $$@ &&)$$(LINK.GCC.AR)))\
$(if $(findstring $E,$4),
	$(if $V,,@echo LINK.GCC.EXEC $$@ &&)$$(call LINK.GCC.EXEC,$(subst $(COMMA),$$(COMMA),$(LDFLAGS.$3) $(LDFLAGS.$4)),$(call .LIBFLAGS,LDLIBS,$3,$4) $(foreach z,$(LIBS.$3) $(LIBS.$4),$(call GCC.LINKLIB,$z))))
$(GCC.EXTRA.MKLRULES)
endef

# Install rules ($1 = module name, $2 = unexpanded target file,
# $3 = full target file name)
define MKIRULES.GCC
$(if $(findstring $L,$2),\
$(foreach _,$3 $(if $(call VALID_VERSION,$(SHARED.$2)),$3.$(basename $(basename $(SHARED.$2))) $3.$(SHARED.$2)),
	$(if $V,,@echo INSTALL $_ to $(call .INSTDIR,$1,$2,LIB,$(CONF_LIBDIR)) &&)\
	$$(call INSTALL,$_,$(call .INSTDIR,$1,$2,LIB,$(CONF_LIBDIR)),$(call .INSTMODE,$1,$2,$(if $(SHARED.$2),0755,0644)))))\
$(if $(findstring $E,$2),
	$(if $V,,@echo INSTALL $3 to $(call .INSTDIR,$1,$2,BIN,$(CONF_BINDIR)) &&)\
	$$(call INSTALL,$3,$(call .INSTDIR,$1,$2,BIN,$(CONF_BINDIR)),$(call .INSTMODE,$1,$2,0755)))\
$(if $(INSTALL.INCLUDE.$2),
	$(if $V,,@echo INSTALL $(INSTALL.INCLUDE.$2) to $(call .INSTDIR,$1,$2,INCLUDE,$(CONF_INCLUDEDIR)) &&)\
	$$(call INSTALL,$(INSTALL.INCLUDE.$2),$(call .INSTDIR,$1,$2,INCLUDE,$(CONF_INCLUDEDIR)),$(call .INSTMODE,$1,$2,0644)))
$(GCC.EXTRA.MKIRULES)
endef
