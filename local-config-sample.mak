# Select your hardware
#HARDWARE = stm32f030chev
#HARDWARE = stm32vldiscovery
#HARDWARE = stm32f103bluepill
#HARDWARE = stm32f4discovery

# Define your MCU in the case your board is not listed in tibs/extra/stm32-board.csv
# A list of supported MCUs is in tibs/extra/stm32-mcu.csv
#MCU.TYPE = STM32F103C8Tx

# Default build mode
MODE = debug

# If needed, you may override the minimum heap size (checked at link time)
#STM32.HEAP.MIN_SIZE ?= 0
# If needed, you may override the minimum stack size (checked at link time)
#STM32.STACK.MIN_SIZE ?= 0x200
