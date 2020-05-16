ifdef MKBINRULES.$(TOOLKIT)

# Generate the rules for building a binary image of given module
# $1 - module name
define MODULE.BINRULES
.PHONY: bin-$1
bin: bin-$1
bin-$1: outdirs $(foreach 2,$(TARGETS.$1),$(patsubst %$(_EX),%.bin,$(call XFNAME.$(.TKNAME),$2)))
$(foreach 2,$(TARGETS.$1),$(call MKBINRULES.$(.TKNAME),$1,$2,$(patsubst %$(_EX),%.bin,$(call XFNAME.$(.TKNAME),$2)),$(call XFNAME.$(.TKNAME),$2)))

endef

define BINHELP
	$(call SAY,bin - Create binary images from all firmware modules)
	$(foreach x,$(filter-out $(IHEX.EXCLUDE),$(IHEX.TARGETS)),\
		$(if $(DESCRIPTION.$x),\
			$(call SAY,    bin-$x - Build binary image of $(DESCRIPTION.$x))$(NL)))
endef
GENHELP += $(BINHELP)

showrules::
	@echo -e '$(subst $(NL),\n,$(foreach x,$(filter-out $(IHEX.EXCLUDE),$(IHEX.TARGETS)),$(call MODULE.BINRULES,$x)))'

# Evaluate bin rules for enabled modules
$(eval $(foreach x,$(filter-out $(IHEX.EXCLUDE),$(IHEX.TARGETS)),$(call MODULE.BINRULES,$x)))

endif # ifdef MKBINRULES.$(TOOLKIT)
