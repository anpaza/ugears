# What is TIBS

The Instant Build System (TIBS) is a small fast build system I use for my hobby
projects. You don't need it to use libugears, but its used so that you can
quickly build and test the library.

Just run 'make' in root directory and it will display a list of build targets.


# Q & A

Q: I'm getting:
    ld: warning: cannot find entry symbol Reset_Handler; defaulting to 0000000008000000
A: Please link with the CMSIS library which provides the startup files:
    LIBS.myapp$E = cmsis$L


# How to add a new test/application

TIBS has been designed to easily add new build modules. A 'build module' is a library,
a test, an application or something like that.

When you run 'make' TIBS scans for all .mak files in directories:
    apps/*/*.mak
    tests/*/*.mak
    libs/*/*.mak

and a few other.

So, all you have to do is to create a subdirectory either under tests/ or apps/, put
a few .c/.cpp files there, and create a .mak file with similar content:

    # This adds your module to list of "applications" modules
    APPS += myapp
    # This adds a description for you module, to see in help
    DESCRIPTION.myapp = My shiny new app
    # This adds your app to list of modules that can be 'flashed' into MCU
    FLASH.TARGETS += myapp
    # This adds rules to build a .hex file for your app too
    IHEX.TARGETS += myapp

    # Now here you define the target file (or files) for your module.
    # $E is a placeholder for "executable" extension (.elf for arm-none-eabi).
    # Other usefule extension is $L which stands for "library"
    TARGETS.myapp = myapp$E
    # Enumerate all source files for your application
    SRC.myapp$E = $(wildcard apps/myapp/*.c)
    # Also list the libraries linked with your executable
    LIBS.myapp$E = cmsis$L useful$L ugears$L

That's all, now you can run 'make' and see your application in the list of buildable
application modules.
