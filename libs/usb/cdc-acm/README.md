## USB CDC ACM driver for uGears library

This is an extremely small library implementing an USB CDC ACM device
(about 1.5k code footprint + 128 bytes RAM). The library is using some
features from the uGears library, although it's not a big problem to make
it run just on top of CMSIS.

This library was inspired by STM32F103-USB-CDC-CMSIS by Samoilov Alexey:

    https://github.com/saewave/STM32F103-USB-CDC-CMSIS).

But was rewritten from scratch to make it possible to release it under
the Apache license.

The library currently supports only STM32F1xx and STM32F3xx
microcontrollers.

Sample code to run on the so-called "Blue Pill" board (with an
STM32F103C8T6 MCU) can be found in tests/stm32f103bluepill/07.usbcdc/
subdirectory.

## License

    Copyright 2020 by Andrey Zabolotnyi <zapparello@ya.ru>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
