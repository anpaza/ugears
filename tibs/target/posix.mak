# Settings for building for the POSIX target

.SUFFIXES: .so

# OS-dependent extensions

# Shared library filenames end in this
_SO = .so
# Shared library filename prefix
SO_ = lib
# Executables end in this
_EX =

# How to tell the linker to set shared library name
GCC.LDFLAGS.SHARED = -shared -Wl,"-soname=$1"
# Set to non-empty if platform supports versioned shared libraries
SO.VERSION = 1

# Detect a library using pkgconfig
# arg $1 - makefile library name (uppercase) $2 - pkgconfig lib name
define DETECT_SYSLIB
ifndef HAVE_$1
ifeq ($$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --exists $2 2>/dev/null && echo Y),Y)
HAVE_$1=1
CFLAGS.$1=$$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $2)
LDLIBS.$1=$$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $2)
endif
endif
ifndef HAVE_$1
ifeq ($$(shell which $2-config 2>&1 >/dev/null && echo Y),Y)
HAVE_$1=1
CFLAGS.$1=$$(shell $2-config --cflags)
LDLIBS.$1=$$(shell $2-config --libs)
endif
endif

endef
