# Settings for building on a Windows host

# Use cmd.exe as shell
SHELL := $(if $(ComSpec),$(ComSpec),$(COMSPEC))

# The SAY function must print its arguments to the terminal
SAY = @echo $(subst |,!,$1)
# This function should be able to create a multi-level directory
MKDIR = $(foreach x,$1,mkdir "$(subst /,\,$x)")
# A function to remove one or more files
RM = $(foreach x,$1,del /q "$(subst /,\,$x)" 2>nul)
# Copy a file over other, if contents differ
UPDATE = fc /b "$(subst /,\,$1)" "$(subst /,\,$2)" >nul 2>nul && del /q "$(subst /,\,$1)" || move /y "$(subst /,\,$1)" "$(subst /,\,$2)"

SINGLE.INSTALL=echo Installing files on Windows is not supported
SINGLE.INSTALLDIR=echo Installing directories on Windows is not supported
