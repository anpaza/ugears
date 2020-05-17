# The uGears library

## What is it

uGears is a small simple library that contains extremely thin wrappers
around STM32 low-level architecture. It is built around the CMSIS library,
providing convenient macros to set up and use GPIOs, timers, i2c, dma and
other peripherials.

uGears doesn't intend to hide all hardware details from you, like
traditional drivers do. If you hide hardware behind a uniform interface,
you inevitably lose functionality and performance.

uGears will allow you easily change hardware configuration without
having to review the whole codebase. For example, if you have a LED
connected to PA12 and you decide to move it to PD5, you just change
the port and pin number in your hardware definition file, and you don't
have to search all your code for instances of `GPIOA` and replace them
with `GPIOD`, `RCC->APB2ENR |= RCC_APB2ENR_IOPAEN` replaced by
`RCC_APB2ENR_IOPDEN` and such.

Similarily, if you decide to change your code from using timer `TIM1`
to `TIM2`, you won't have to remember that `TIM1` is enabled by setting
bit `RCC_APB1ENR_TIM2EN` in `RCC->APB1ENR`, while  `TIM2` is enabled by
setting bit `RCC_APB2ENR_TIM1EN` in `RCC->APB2ENR`. You just change
a single #define in your hardware definition file, and everything
else will happen by magic.

uGears will make it easier to port code from one MCU family to
other, e.g. STM32F1 to STM32F4. It cannot make the transition
completely painless, because those MCU families are too different,
but it will make it simpler.

uGears does not depend on anything, you can start compiling code
right after downloading it. Everything it needs is contained in
this repository.

uGears uses the Apache 2.0 license, so  you can use it even in
proprietary products.

All in all, using uGears will permit you write code with better
portability, easier, faster and less error-prone.


## Documentation

The library is pretty well self-documented through its header files.
There's no much sense to generate separate documentation with doxygen
or something like that, as the output will look pretty much like the
original header files, plus a few unimportant bells and whistles.

Still, comments are formatted using Doxygen guidelines and tags.

### Source tree structure

The source code is separated in modules. A module is an application,
a library, a test and so on. Every module lies in a separate
subdirectory (usually), that makes it easier to define module sources -
just grab all .c files in a certain directory and you don't even have
to change the makefile when you add a new .c file.

Each module has its build rules in a small simple file with the .mak
extension. You just define what the source files for this module are,
what libraries it use, a description and such things. The rest -
defining the right rules for building everything you need - will happen
automatically inside the TIBS build system, which is located under tibs/.
For the beginning, you don't have to know a lot about it, just copy
a suitable .mak from a similar module into your module and modify it.

The include/ subdirectory is for header files. Each library module
usually places its header files in a subdirectory under include/.

The libs/ subdirectory is for library modules.

The tests/ subdirectory is for test applications. Multiplatform tests
are placed directly under tests/, and there are a number of directories
named by platform names - those contains tests, specific for a
hardware platform.

There could be an apps/ subdirectory. It doesn't exist now because
uGears project doesn't provide any apps. You may create it yourself,
and under apps/ you can create subdirectories for your app modules.
The TIBS will automatically load any `apps/*/*.mak` files.


### Where to start

The easiest way to start is to get one of the supported boards.
The tests/ subdirectory contains a number of samples that may give you
an idea of the uGears basics.

So, the first thing you have to do is to define what supported board
you got. Copy local-config-sample.mak to local-config.mak and uncomment
the appropiate HARDWARE definition.

Now run 'make' in root project directory and see what targets it can build.
Look at the 'tests' section, that's where the test apps are grouped.
Go into the tests/$(HARDWARE)/ directory to see the source code.
The tests are sorted by increasing complexity.

Build a test by, say, typing

    make tfirst

If everything goes well, you'll end with tfirst.elf in build directory.
Now you can flash it into your board. Connect the board to SWD debugger
and run:

    make flash-tfirst

This will use the st-flash utility to flash the binary file (which is
converted from the elf file). If st-link doesn't support your SWD debugger,
you may use other tools, but you will have to flash manually. You can
build either a .bin or .hex file from the .elf file manually:

    make ihex-tfirst, or
    make bin-tfirst

Note that you don't have to execute `make tfirst` every time you change
a source file. Just enter `make flash-tfirst` or `make ihex-tfirst` and
everything will proceed as it should.

Also, a very helpful thing to have would be one of those cheap USB-UART
dongles. Connect it to USART1 pins and launch a terminal program,
as many test programs will print useful information there.


### Adding new platforms

Adding a new hardware platform is easy.

1. Choose a name for the platform. Look if your hardware is already listed
in `tibs/extra/stm32-board.csv`. There you will find most well-known STM32
evaluation boards. Assign the board name to HARDWARE variable in
your `local-config.mak` file.

2. If it's not there, it's not a problem. Invent a new board name and assign
it to HARDWARE in your `local-config.mak` file as usual. Then, define what
MCU type your board uses. A list of valid MCU types can be found in
`tibs/extra/stm32-mcu.csv`. Assign MCU type to MCU.TYPE variable just
below HARDWARE in `local-config.mak`. Always use lower case for both
HARDWARE and MCU.TYPE, but if you don't, TIBS will lowercase it for you.

3. Create hardware definitions file include/hardware/$(HARDWARE).h.
You can use include/hardware/template.h as a template.

4. Fill the file as suggested by instructions inside template.h.

5. Now you may create your own tests/$(HARDWARE) subdirectory and start
filling it with simple tests. Start with something simple, and then
move on.


## Copyright

uGears is copyright (c) 2010-2020 by Andrey Zabolotnyi <zapparello@ya.ru>

In an attempt to be useful to a larger audience, the library is licensed
under the Apache License version 2.0.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

A copy of Apache license can be found [in the doc/ directory](doc/LICENSE-2.0).

The only caveat is the tfp_printf routine which is LGPL licensed.
I'll rewrite it from scratch one day when I'll be in a mood for it.

On the compatibility between Apache license and GPL
[you can read here](https://www.apache.org/licenses/GPL-compatibility.html).

In short: you may use Apache-licensed code in GPL-licensed projects,
but not GPL-licensed code in Apache-licensed projects.

The CMSIS library is partly licensed under same Apache 2.0 license
(the code from Arm Limited), and under the so-called 'BSD 3-clause license'
(the code from STMicroelectronics). You can get it either from
[opensource.org](opensource.org/licenses/BSD-3-Clause), and of course it can
be found [in the doc/ directory](doc/BSD-3-Clause).
