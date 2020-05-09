ifdef MKIXRULES.$(TOOLKIT)

# Generate the firmware ihexing rules for given module
# $1 - module name
define MODULE.IHEXRULES
.PHONY: ihex-$1
ihex: ihex-$1
ihex-$1: outdirs $(foreach 2,$(TARGETS.$1),$(patsubst %$(_EX),%.hex,$(call XFNAME.$(.TKNAME),$2)))
$(foreach 2,$(TARGETS.$1),$(call MKIXRULES.$(.TKNAME),$1,$2,$(patsubst %$(_EX),%.hex,$(call XFNAME.$(.TKNAME),$2)),$(call XFNAME.$(.TKNAME),$2)))

endef

define IHEXHELP
	$(call SAY,ihex - Convert to IHEX all firmware modules)
	$(foreach x,$(filter-out $(IHEX.EXCLUDE),$(IHEX.TARGETS)),\
		$(if $(DESCRIPTION.$x),\
			$(call SAY,    ihex-$x - Create HEX of $(DESCRIPTION.$x))$(NL)))
endef
GENHELP += $(IHEXHELP)

showrules::
	@echo -e '$(subst $(NL),\n,$(foreach x,$(filter-out $(IHEX.EXCLUDE),$(IHEX.TARGETS)),$(call MODULE.IHEXRULES,$x)))'

# Evaluate ihex rules for ihexable modules
$(eval $(foreach x,$(filter-out $(IHEX.EXCLUDE),$(IHEX.TARGETS)),$(call MODULE.IHEXRULES,$x)))

endif # ifdef MKIXRULES.$(TOOLKIT)
