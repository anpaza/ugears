# The Instant Build System: initialization
# Copyright (c) 2004-2020 by Andrey Zabolotnyi
# Include this at top of your Makefile to define some useful variables.
# If you don't, it will be included from rules.mak

ifndef TIBS.INIT
TIBS.INIT = 1

DIR.TIBS ?= /usr/share/tibs
TIBS.VERSION ?= 0.3.0

# Give sane defaults to installation directories, if not defined by user
CONF_PREFIX ?= /usr/local/
CONF_BINDIR ?= $(CONF_PREFIX)bin/
CONF_SYSCONFDIR ?= $(CONF_PREFIX)etc/
CONF_DATADIR ?= $(CONF_PREFIX)share/$(CONF_PACKAGE)/
CONF_LIBDIR ?= $(CONF_PREFIX)lib$(if $(findstring 64,$(ARCH)),64)/
CONF_INCLUDEDIR ?= $(CONF_PREFIX)include/
CONF_DOCDIR ?= $(CONF_PREFIX)share/doc/$(CONF_PACKAGE)-$(CONF_VERSION)/
CONF_LIBEXECDIR ?= $(CONF_PREFIX)libexec/$(CONF_PACKAGE)/

.PHONY: default help all clean dist distclean cleandep cleangen install
default: showhelp

# Forget all the built-in exotic suffixes
.SUFFIXES:

# Default groups of build targets
GROUPS := LIBS APPS DRIVERS TOOLS TESTS DATA DOCS $(GROUPS)
LIBS.dir = libs
LIBS.desc = Project libraries
APPS.dir = apps
APPS.desc = Application programs
TOOLS.dir = tools
TOOLS.desc = Miscelaneous tools
TESTS.dir = tests
TESTS.desc = Small programs for testing
DATA.dir = data
DATA.desc = Project data files
DOCS.dir = docs
DOCS.desc = Documentation files
DRIVERS.dir = drivers
DRIVERS.desc = Drivers

# A newline
define NL


endef
# A comma
COMMA=,
# A empty
EMPTY=
# A space
SPACE=$(EMPTY) $(EMPTY)
# Translate every char of a string to upper case
#ASCIIUP = $(shell echo $1 | tr a-z A-Z)
ASCIIUP = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))
# Translate every char of a string to lower case
#ASCIILOW = $(shell echo $1 | tr A-Z a-z)
ASCIILOW = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))
# Useful macro to strip extension from a file
NOSUFFIX=$(subst $(suffix $1),,$1)

# Pseudo-file-extensions (for cross-platform filenames)
# For executable files:
E = @exe@
# Library files (either static or dynamic, depends on SHARED.$(libname))
L = @lib@
# Pseudo-extension for documentation modules
D = @doc@

endif # TIBS.INIT
