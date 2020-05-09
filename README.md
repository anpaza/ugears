# What is it

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
have to search all your code for instances of GPIOA and replace them
with GPIOD, RCC->APB2ENR |= RCC_APB2ENR_IOPAEN replaced by
RCC_APB2ENR_IOPDEN and such.

Similarily, if you decide to change your code from using timer TIM1
to TIM2, you won't have to remember that TIM1 is enabled by setting
bit RCC_APB1ENR_TIM2EN in RCC->APB1ENR, while  TIM2 is enabled by
setting bit RCC_APB2ENR_TIM1EN in RCC->APB2ENR.

uGears will make it easier to port code from one MCU family to
other, e.g. STM32 F1 to STM32 F4. It cannot make the transition
completely painless, because those MCU families are too different,
but it will make it simpler.

All in all, using uGears will permit you write code with better
portability, easier, faster and less error-prone.


# Documentation

The library is pretty well self-documented through its header files.
There's no much sense to generate separate documentation with doxygen
or something like that, as the output will look pretty much like the
original header files, plus a few unimportant bells and whistles.

Still, comments are formatted using Doxygen guidelines and tags.


# Copyright

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

The only caveat is the tfp_printf routine which is LGPL licensed.
I'll rewrite it from scratch one day when I'll be in a mood for it.

On the compatibility between Apache license and GPL see here:

    https://www.apache.org/licenses/GPL-compatibility.html

In short: you may use Apache-licensed code in GPL-licensed projects,
but not GPL-licensed code in Apache-licensed projects.
