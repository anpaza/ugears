# NOTE: This QMake project file is 'fake' and is provided just
# to make it easier editing uGears sourcecode in QtCreator IDE.
#
# You can use it for building in a limited way:
# - Go to Projects -> Build & Run
# - Enable any existing Qt toolkit
# - Uncheck the 'Shadow Build' checkbox
# - In the Build Steps remove the qMake step
# - in 'Make arguments' enter the target you want to build
#   (e.g. tfirst or whatever)
#
# Also set up the proper HARDWARE below. In a terminal, run
# make V=1 <sometarget> and pick up the defines from -D options
# passed to gcc, and enter them below in DEFINES.

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

# Define some macros & include paths to mimic TIBS
HARDWARE = stm32f103bluepill
DEFINES += HARDWARE_H=\\\"$${HARDWARE}.h\\\"
DEFINES += STM32 STM32F1 STM32F103C8Tx STM32F103xB CORTEX_M3
DEFINES += STM32_STACK_ADDRESS=0x20005000
DEFINES += STM32_MIN_HEAP_SIZE=0 STM32_MIN_STACK_SIZE=0x200
DEFINES += STM32_FLASH_ORIGIN=0x08000000 STM32_FLASH_SIZE=0x10000
DEFINES += STM32_RAM_ORIGIN=0x20000000 STM32_RAM_SIZE=0x5000
INCLUDEPATH += include
INCLUDEPATH += include/hardware
INCLUDEPATH += include/cmsis
INCLUDEPATH += include/cmsis/stm32
INCLUDEPATH += include/useful
INCLUDEPATH += include/ugears
INCLUDEPATH += include/ugears/stm32
INCLUDEPATH += include/usb
INCLUDEPATH += tests/$$HARDWARE

SOURCES += \
    libs/ugears/stm32/adc.c \
    libs/ugears/stm32/bkp.c \
    libs/ugears/stm32/can.c \
    libs/ugears/stm32/clock/stm32f0.c \
    libs/ugears/stm32/clock/stm32f1.c \
    libs/ugears/stm32/clock/stm32f4.c \
    libs/ugears/stm32/dma.c \
    libs/ugears/stm32/exti.c \
    libs/ugears/stm32/flash-storage.c \
    libs/ugears/stm32/flash.c \
    libs/ugears/stm32/gpio.c \
    libs/ugears/stm32/i2c.c \
    libs/ugears/stm32/iwdg.c \
    libs/ugears/stm32/nvic.c \
    libs/ugears/stm32/rtc.c \
    libs/ugears/stm32/sysinit.c \
    libs/ugears/stm32/tim.c \
    libs/ugears/stm32/usart-stdio.c \
    libs/ugears/stm32/usart.c \
    libs/usb/cdc-acm/cdcacm-desc.c \
    libs/usb/cdc-acm/cdcacm-printf.c \
    libs/usb/cdc-acm/cdcacm-str.c \
    libs/usb/cdc-acm/cdcacm.c \
    libs/useful/arm/semihosting.c \
    libs/useful/atan2_16.c \
    libs/useful/clock.c \
    libs/useful/datetime.c \
    libs/useful/ip_crc.c \
    libs/useful/leb128.c \
    libs/useful/ost.c \
    libs/useful/printf.c \
    libs/useful/rand.c \
    libs/useful/sin.c \
    libs/useful/sqrt_X.c \
    libs/useful/udiv64_32.c \
    libs/useful/unaligned.c \
    tests/atomic/atomic.c \
    tests/stm32f030chev/01.led/main.c \
    tests/stm32f030chev/02.uart/main.c \
    tests/stm32f030chev/03.dynclk/main.c \
    tests/stm32f103bluepill/01.first/main.c \
    tests/stm32f103bluepill/02.led/main.c \
    tests/stm32f103bluepill/03.uart/main.c \
    tests/stm32f103bluepill/04.uartdma/main.c \
    tests/stm32f103bluepill/05.rtc/main.c \
    tests/stm32f103bluepill/06.dynclk/main.c \
    tests/stm32f103bluepill/07.usbcdc/main.c \
    tests/stm32f103bluepill/hw.c \
    tests/stm32f4discovery/tfirst/main.c \
    tests/stm32vldiscovery/01.first/main.c \
    tests/stm32vldiscovery/02.led/main.c \
    tests/stm32vldiscovery/03.uart/main.c \
    tests/stm32vldiscovery/04.uartdma/main.c \
    tests/stm32vldiscovery/05.rtc/main.c \
    tests/stm32vldiscovery/06.dynclk/main.c \
    tests/stm32vldiscovery/07.libfun/main.c \
    tests/stm32vldiscovery/hw.c \
    tests/tsh/main.c \
    tibs/extra/stm32/__libc_init_array.c

DISTFILES += \
    Makefile \
    README.md \
    doc/BSD-3-Clause \
    doc/LICENSE-2.0 \
    doc/debug/HOWTO.md \
    doc/debug/gdbinit-stm32f0 \
    doc/debug/gdbinit-stm32f1 \
    doc/hardware/stm32f030chev.jpg \
    doc/hardware/stm32f103bluepill.jpg \
    libs/cmsis/License.md \
    libs/cmsis/cmsis.mak \
    libs/cmsis/stm32/startup_stm32f030x6.s \
    libs/cmsis/stm32/startup_stm32f030x8.s \
    libs/cmsis/stm32/startup_stm32f030xc.s \
    libs/cmsis/stm32/startup_stm32f031x6.s \
    libs/cmsis/stm32/startup_stm32f038xx.s \
    libs/cmsis/stm32/startup_stm32f042x6.s \
    libs/cmsis/stm32/startup_stm32f048xx.s \
    libs/cmsis/stm32/startup_stm32f051x8.s \
    libs/cmsis/stm32/startup_stm32f058xx.s \
    libs/cmsis/stm32/startup_stm32f070x6.s \
    libs/cmsis/stm32/startup_stm32f070xb.s \
    libs/cmsis/stm32/startup_stm32f071xb.s \
    libs/cmsis/stm32/startup_stm32f072xb.s \
    libs/cmsis/stm32/startup_stm32f078xx.s \
    libs/cmsis/stm32/startup_stm32f091xc.s \
    libs/cmsis/stm32/startup_stm32f098xx.s \
    libs/cmsis/stm32/startup_stm32f100xb.s \
    libs/cmsis/stm32/startup_stm32f100xe.s \
    libs/cmsis/stm32/startup_stm32f101x6.s \
    libs/cmsis/stm32/startup_stm32f101xb.s \
    libs/cmsis/stm32/startup_stm32f101xe.s \
    libs/cmsis/stm32/startup_stm32f101xg.s \
    libs/cmsis/stm32/startup_stm32f102x6.s \
    libs/cmsis/stm32/startup_stm32f102xb.s \
    libs/cmsis/stm32/startup_stm32f103x6.s \
    libs/cmsis/stm32/startup_stm32f103xb.s \
    libs/cmsis/stm32/startup_stm32f103xe.s \
    libs/cmsis/stm32/startup_stm32f103xg.s \
    libs/cmsis/stm32/startup_stm32f105xc.s \
    libs/cmsis/stm32/startup_stm32f107xc.s \
    libs/cmsis/stm32/startup_stm32f205xx.s \
    libs/cmsis/stm32/startup_stm32f207xx.s \
    libs/cmsis/stm32/startup_stm32f215xx.s \
    libs/cmsis/stm32/startup_stm32f217xx.s \
    libs/cmsis/stm32/startup_stm32f301x8.s \
    libs/cmsis/stm32/startup_stm32f302x8.s \
    libs/cmsis/stm32/startup_stm32f302xc.s \
    libs/cmsis/stm32/startup_stm32f302xe.s \
    libs/cmsis/stm32/startup_stm32f303x8.s \
    libs/cmsis/stm32/startup_stm32f303xc.s \
    libs/cmsis/stm32/startup_stm32f303xe.s \
    libs/cmsis/stm32/startup_stm32f318xx.s \
    libs/cmsis/stm32/startup_stm32f328xx.s \
    libs/cmsis/stm32/startup_stm32f334x8.s \
    libs/cmsis/stm32/startup_stm32f358xx.s \
    libs/cmsis/stm32/startup_stm32f373xc.s \
    libs/cmsis/stm32/startup_stm32f378xx.s \
    libs/cmsis/stm32/startup_stm32f398xx.s \
    libs/cmsis/stm32/startup_stm32f401xc.s \
    libs/cmsis/stm32/startup_stm32f401xe.s \
    libs/cmsis/stm32/startup_stm32f405xx.s \
    libs/cmsis/stm32/startup_stm32f407xx.s \
    libs/cmsis/stm32/startup_stm32f410cx.s \
    libs/cmsis/stm32/startup_stm32f410rx.s \
    libs/cmsis/stm32/startup_stm32f410tx.s \
    libs/cmsis/stm32/startup_stm32f411xe.s \
    libs/cmsis/stm32/startup_stm32f412cx.s \
    libs/cmsis/stm32/startup_stm32f412rx.s \
    libs/cmsis/stm32/startup_stm32f412vx.s \
    libs/cmsis/stm32/startup_stm32f412zx.s \
    libs/cmsis/stm32/startup_stm32f413xx.s \
    libs/cmsis/stm32/startup_stm32f415xx.s \
    libs/cmsis/stm32/startup_stm32f417xx.s \
    libs/cmsis/stm32/startup_stm32f423xx.s \
    libs/cmsis/stm32/startup_stm32f427xx.s \
    libs/cmsis/stm32/startup_stm32f429xx.s \
    libs/cmsis/stm32/startup_stm32f437xx.s \
    libs/cmsis/stm32/startup_stm32f439xx.s \
    libs/cmsis/stm32/startup_stm32f446xx.s \
    libs/cmsis/stm32/startup_stm32f469xx.s \
    libs/cmsis/stm32/startup_stm32f479xx.s \
    libs/cmsis/stm32/startup_stm32h742xx.s \
    libs/cmsis/stm32/startup_stm32h743xx.s \
    libs/cmsis/stm32/startup_stm32h745xx.s \
    libs/cmsis/stm32/startup_stm32h747xx.s \
    libs/cmsis/stm32/startup_stm32h750xx.s \
    libs/cmsis/stm32/startup_stm32h753xx.s \
    libs/cmsis/stm32/startup_stm32h755xx.s \
    libs/cmsis/stm32/startup_stm32h757xx.s \
    libs/cmsis/stm32/startup_stm32h7a3xx.s \
    libs/cmsis/stm32/startup_stm32h7a3xxq.s \
    libs/cmsis/stm32/startup_stm32h7b0xx.s \
    libs/cmsis/stm32/startup_stm32h7b0xxq.s \
    libs/cmsis/stm32/startup_stm32h7b3xx.s \
    libs/cmsis/stm32/startup_stm32h7b3xxq.s \
    libs/ugears/License.md \
    libs/ugears/stm32/delay_clocks.S \
    libs/ugears/stm32/ugears.mak \
    libs/ugears/ugears.mak \
    libs/usb/cdc-acm/README.md \
    libs/usb/cdc-acm/uGears.inf \
    libs/usb/cdc-acm/cdcacm.mak \
    libs/usb/cdc-acm/cdcacm.mak \
    libs/usb/usb.mak \
    libs/useful/thumb/memcpy.S \
    libs/useful/thumb/memset.S \
    libs/useful/useful.mak \
    local-config-sample.mak \
    local-config.mak \
    tests/atomic/atomic.mak \
    tests/stm32f030chev/01.led/led.mak \
    tests/stm32f030chev/02.uart/uart.mak \
    tests/stm32f030chev/03.dynclk/tdynclk.mak \
    tests/stm32f030chev/hw.mak \
    tests/stm32f030chev/stm32f030chev.mak \
    tests/stm32f103bluepill/01.first/first.mak \
    tests/stm32f103bluepill/02.led/led.mak \
    tests/stm32f103bluepill/03.uart/uart.mak \
    tests/stm32f103bluepill/04.uartdma/uartdma.mak \
    tests/stm32f103bluepill/05.rtc/rtc.mak \
    tests/stm32f103bluepill/06.dynclk/tdynclk.mak \
    tests/stm32f103bluepill/07.usbcdc/tusbcdc.mak \
    tests/stm32f103bluepill/README.md \
    tests/stm32f103bluepill/hw.mak \
    tests/stm32f103bluepill/stm32f103bluepill.mak \
    tests/stm32f4discovery/hw.mak \
    tests/stm32f4discovery/stm32f4discovery.mak \
    tests/stm32f4discovery/tfirst/tfirst.mak \
    tests/stm32vldiscovery/01.first/first.mak \
    tests/stm32vldiscovery/02.led/led.mak \
    tests/stm32vldiscovery/03.uart/uart.mak \
    tests/stm32vldiscovery/04.uartdma/uartdma.mak \
    tests/stm32vldiscovery/05.rtc/rtc.mak \
    tests/stm32vldiscovery/06.dynclk/tdynclk.mak \
    tests/stm32vldiscovery/07.libfun/libfun.mak \
    tests/stm32vldiscovery/hw.mak \
    tests/stm32vldiscovery/stm32vldiscovery.mak \
    tests/tsh/tsh.mak \
    tibs/README.md \
    tibs/compiler/data.mak \
    tibs/compiler/doxygen.mak \
    tibs/compiler/gcc-nasm.mak \
    tibs/compiler/gcc.mak \
    tibs/compiler/msvc.mak \
    tibs/compiler/pkgconfig.mak \
    tibs/extra/arm-none-eabi-gcc.mak \
    tibs/extra/bin-rules.mak \
    tibs/extra/flash-rules.mak \
    tibs/extra/ihex-rules.mak \
    tibs/extra/mkcsv \
    tibs/extra/stm32-board.csv \
    tibs/extra/stm32-mcu.csv \
    tibs/extra/stm32-targets.py \
    tibs/extra/stm32.mak \
    tibs/extra/stm32/arm-none-eabi-gcc.mak \
    tibs/extra/stm32/bin-rules.mak \
    tibs/extra/stm32/board.csv \
    tibs/extra/stm32/flash-rules.mak \
    tibs/extra/stm32/flash.ld.in \
    tibs/extra/stm32/ihex-rules.mak \
    tibs/extra/stm32/mcu.csv \
    tibs/extra/stm32/mkcsv \
    tibs/extra/stm32/stm32-targets.py \
    tibs/extra/stm32/stm32.mak \
    tibs/extra/stm32_flash.ld.in \
    tibs/extra/useful.mak \
    tibs/host/linux.mak \
    tibs/host/mac.mak \
    tibs/host/posix.mak \
    tibs/host/windows.mak \
    tibs/init.mak \
    tibs/rules.mak \
    tibs/target/linux.mak \
    tibs/target/mac.mak \
    tibs/target/none-eabi.mak \
    tibs/target/posix.mak \
    tibs/target/windows.mak

HEADERS += \
    include/cmsis/cmsis_compiler.h \
    include/cmsis/cmsis_gcc.h \
    include/cmsis/cmsis_version.h \
    include/cmsis/core_cm0.h \
    include/cmsis/core_cm3.h \
    include/cmsis/core_cm4.h \
    include/cmsis/core_cm7.h \
    include/cmsis/mpu_armv7.h \
    include/cmsis/stm32/stm32f030x6.h \
    include/cmsis/stm32/stm32f030x8.h \
    include/cmsis/stm32/stm32f030xc.h \
    include/cmsis/stm32/stm32f031x6.h \
    include/cmsis/stm32/stm32f038xx.h \
    include/cmsis/stm32/stm32f042x6.h \
    include/cmsis/stm32/stm32f048xx.h \
    include/cmsis/stm32/stm32f051x8.h \
    include/cmsis/stm32/stm32f058xx.h \
    include/cmsis/stm32/stm32f070x6.h \
    include/cmsis/stm32/stm32f070xb.h \
    include/cmsis/stm32/stm32f071xb.h \
    include/cmsis/stm32/stm32f072xb.h \
    include/cmsis/stm32/stm32f078xx.h \
    include/cmsis/stm32/stm32f091xc.h \
    include/cmsis/stm32/stm32f098xx.h \
    include/cmsis/stm32/stm32f0xx.h \
    include/cmsis/stm32/stm32f100xb.h \
    include/cmsis/stm32/stm32f100xe.h \
    include/cmsis/stm32/stm32f101x6.h \
    include/cmsis/stm32/stm32f101xb.h \
    include/cmsis/stm32/stm32f101xe.h \
    include/cmsis/stm32/stm32f101xg.h \
    include/cmsis/stm32/stm32f102x6.h \
    include/cmsis/stm32/stm32f102xb.h \
    include/cmsis/stm32/stm32f103x6.h \
    include/cmsis/stm32/stm32f103xb.h \
    include/cmsis/stm32/stm32f103xe.h \
    include/cmsis/stm32/stm32f103xg.h \
    include/cmsis/stm32/stm32f105xc.h \
    include/cmsis/stm32/stm32f107xc.h \
    include/cmsis/stm32/stm32f1xx.h \
    include/cmsis/stm32/stm32f205xx.h \
    include/cmsis/stm32/stm32f207xx.h \
    include/cmsis/stm32/stm32f215xx.h \
    include/cmsis/stm32/stm32f217xx.h \
    include/cmsis/stm32/stm32f2xx.h \
    include/cmsis/stm32/stm32f301x8.h \
    include/cmsis/stm32/stm32f302x8.h \
    include/cmsis/stm32/stm32f302xc.h \
    include/cmsis/stm32/stm32f302xe.h \
    include/cmsis/stm32/stm32f303x8.h \
    include/cmsis/stm32/stm32f303xc.h \
    include/cmsis/stm32/stm32f303xe.h \
    include/cmsis/stm32/stm32f318xx.h \
    include/cmsis/stm32/stm32f328xx.h \
    include/cmsis/stm32/stm32f334x8.h \
    include/cmsis/stm32/stm32f358xx.h \
    include/cmsis/stm32/stm32f373xc.h \
    include/cmsis/stm32/stm32f378xx.h \
    include/cmsis/stm32/stm32f398xx.h \
    include/cmsis/stm32/stm32f3xx.h \
    include/cmsis/stm32/stm32f401xc.h \
    include/cmsis/stm32/stm32f401xe.h \
    include/cmsis/stm32/stm32f405xx.h \
    include/cmsis/stm32/stm32f407xx.h \
    include/cmsis/stm32/stm32f410cx.h \
    include/cmsis/stm32/stm32f410rx.h \
    include/cmsis/stm32/stm32f410tx.h \
    include/cmsis/stm32/stm32f411xe.h \
    include/cmsis/stm32/stm32f412cx.h \
    include/cmsis/stm32/stm32f412rx.h \
    include/cmsis/stm32/stm32f412vx.h \
    include/cmsis/stm32/stm32f412zx.h \
    include/cmsis/stm32/stm32f413xx.h \
    include/cmsis/stm32/stm32f415xx.h \
    include/cmsis/stm32/stm32f417xx.h \
    include/cmsis/stm32/stm32f423xx.h \
    include/cmsis/stm32/stm32f427xx.h \
    include/cmsis/stm32/stm32f429xx.h \
    include/cmsis/stm32/stm32f437xx.h \
    include/cmsis/stm32/stm32f439xx.h \
    include/cmsis/stm32/stm32f446xx.h \
    include/cmsis/stm32/stm32f469xx.h \
    include/cmsis/stm32/stm32f479xx.h \
    include/cmsis/stm32/stm32f4xx.h \
    include/cmsis/stm32/stm32h742xx.h \
    include/cmsis/stm32/stm32h743xx.h \
    include/cmsis/stm32/stm32h745xx.h \
    include/cmsis/stm32/stm32h747xx.h \
    include/cmsis/stm32/stm32h750xx.h \
    include/cmsis/stm32/stm32h753xx.h \
    include/cmsis/stm32/stm32h755xx.h \
    include/cmsis/stm32/stm32h757xx.h \
    include/cmsis/stm32/stm32h7a3xx.h \
    include/cmsis/stm32/stm32h7a3xxq.h \
    include/cmsis/stm32/stm32h7b0xx.h \
    include/cmsis/stm32/stm32h7b0xxq.h \
    include/cmsis/stm32/stm32h7b3xx.h \
    include/cmsis/stm32/stm32h7b3xxq.h \
    include/cmsis/stm32/stm32h7xx.h \
    include/hardware/stm32f030chev.h \
    include/hardware/stm32f103bluepill.h \
    include/hardware/stm32f4discovery.h \
    include/hardware/stm32vldiscovery.h \
    include/hardware/template.h \
    include/ugears/stm32/adc.h \
    include/ugears/stm32/atomic.h \
    include/ugears/stm32/can.h \
    include/ugears/stm32/clocks-stm32f0.h \
    include/ugears/stm32/clocks-stm32f1.h \
    include/ugears/stm32/clocks-stm32f4.h \
    include/ugears/stm32/clocks.h \
    include/ugears/stm32/dma.h \
    include/ugears/stm32/exti.h \
    include/ugears/stm32/flash.h \
    include/ugears/stm32/gpio.h \
    include/ugears/stm32/i2c.h \
    include/ugears/stm32/iwdg.h \
    include/ugears/stm32/nvic.h \
    include/ugears/stm32/pwr.h \
    include/ugears/stm32/rcc.h \
    include/ugears/stm32/rtc.h \
    include/ugears/stm32/spi.h \
    include/ugears/stm32/systick.h \
    include/ugears/stm32/tim.h \
    include/ugears/stm32/ugears.h \
    include/ugears/stm32/usart.h \
    include/usb/usb-cdc-acm.h \
    include/usb/usb-cdc.h \
    include/usb/usb.h \
    include/useful/datetime.h \
    include/useful/fpmath.h \
    include/useful/ost.h \
    include/useful/printf.h \
    include/useful/semihosting.h \
    include/useful/time.h \
    include/useful/useful-arm.h \
    include/useful/useful-generic.h \
    include/useful/useful-x86.h \
    include/useful/useful.h \
    include/useful/usefun.h \
    libs/ugears/stm32/dmafun.h \
    libs/ugears/stm32/i2cmd.h \
    libs/usb/cdc-acm/cdcacm-priv.h \
    libs/usb/cdcacm-priv.h \
    libs/usb/cdcacm-str.h \
    tests/stm32f103bluepill/hw.h \
    tests/stm32vldiscovery/hw.h
