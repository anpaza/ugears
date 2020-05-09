### Flashing problems

There's a problem that sometimes may manifest with these boards
(I've seen that with other boards too, but here that happens quite often).

Sometimes, when you run `make flash-something`, you may get an error like this:

    FLASH.ARM-NONE-EABI-GCC out/stm32f103bluepill/debug/tled.bin
    st-flash 1.6.0-32-gcda2215
    2020-05-09T23:11:50 INFO common.c: Loading device parameters....
    2020-05-09T23:11:50 WARN common.c: Invalid flash type, please check device declaration
      core status: unknown
    Unknown memory region

In this case do the following:

* Press the RESET button and hold it with one hand
* With other hand run the `make flash-...` command
* When you see output from st-flash, release the button

That could take some time to practice, but once you get it that works well.

I'm not sure what could be the cause of this, but it happens.
