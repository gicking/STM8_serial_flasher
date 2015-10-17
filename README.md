STM8_serial_flasher
-------------------

PC tool for uplading hexfiles to the STM8 microcontroller via COM port or USB using the built-in ROM bootloader (BSL) of the STM8. I wrote this tool, because the similar "Flash Loader Demonstrator" tool by STMicroelectronics supports Windows only.

Content:
  - STM8_serial_flasher -> program STM8 via bootloader and USB or UART interface
  - BSL_activate        -> STM8 project for activating bootloader. For details check 'README'

For more details and instructions on building and using the tool see the Wiki under https://github.com/gicking/STM8_serial_flasher/wiki

If you find any bugs or for feature requests, please send me a note.

Have fun!
Georg

====================================

Revision History
----------------

1.0.0 (2014-12-21): initial release by Georg Icking-Konert under the Apache License 2.0
----------------
1.1.0 (2015-06-22): - add support for STM8 bootloader “reply mode“
                    - add optional reset of STM8 via DTR (RS232/USB) or GPIO18 (Raspberry)
