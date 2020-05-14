To debug an STM32 system through JTAG or SWD interface:

* Launch the gdb debugging agent program (for example, st-util)
* Copy gdbinit-[mcu] to .gdbinit into the directory from where you launch gdb
* Launch arm-none-eabi-gdb <your ARM executable>. When gdb will load .gdbinit,
  it should contact your debugging agent and connect the board.
* Run the 'load' command to load the program into flash memory
* Debug as usual

Additionaly, the gdbinit template contains some useful macros to inspect
hardware state. You may type commands like "GPIOA", "RCC", "DMA1" etc
to see register contents of the respective peripherial.
