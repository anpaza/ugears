ifdef MKFRULES.$(TOOLKIT)

# Generate the firmware flashing rules for given module
# $1 - module name
define MODULE.FLASHRULES
.PHONY: flash-$1
flash: flash-$1
flash-$1: outdirs $(foreach 2,$(TARGETS.$1),$(patsubst %$(_EX),%.bin,$(call XFNAME.$(.TKNAME),$2)))\
$(foreach 2,$(TARGETS.$1),$(call MKFRULES.$(.TKNAME),$1,$2,$(patsubst %$(_EX),%.bin,$(call XFNAME.$(.TKNAME),$2))))

endef

define FLASHHELP
	$(call SAY,flash - Flash all firmware modules from this project)
	$(foreach x,$(filter-out $(FLASH.EXCLUDE),$(FLASH.TARGETS)),\
		$(if $(DESCRIPTION.$x),\
			$(call SAY,    flash-$x - Flash the $(DESCRIPTION.$x))$(NL)))
endef
GENHELP += $(FLASHHELP)

showrules::
	@echo -e '$(subst $(NL),\n,$(foreach x,$(filter-out $(FLASH.EXCLUDE),$(FLASH.TARGETS)),$(call MODULE.FLASHRULES,$x)))'

# Evaluate flash rules for flashable modules
$(eval $(foreach x,$(filter-out $(FLASH.EXCLUDE),$(FLASH.TARGETS)),$(call MODULE.FLASHRULES,$x)))

endif # ifdef MKFRULES.$(TOOLKIT)
