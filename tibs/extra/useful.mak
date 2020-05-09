# A couple of useful macros

asciiup = $(shell echo $1 | tr a-z A-Z)
asciidown = $(shell echo $1 | tr A-Z a-z)

EMPTY =
SPACE = $(EMPTY) $(EMPTY)
COMMA = ,
define NL


endef
