STM8_serial_flasher
-------------------

PC tool for uplading hexfiles to the STM8 microcontroller via COM port or USB using the built-in ROM bootloader (BSL) of the STM8. I wrote this tool, because the similar "Flash Loader Demonstrator" tool by STMicroelectronics (STM) only supports Windows.

Notes:
  - for the popular STM8 Discovery Board an additional UART->USB adapter is required (e.g. see UM232R by Farnell), since the board connects to the PC only via SWIM (=debug interface)
  - the tool has been tested under various operating systems. For a complete list of supported OS'es see https://github.com/gicking/STM8_serial_flasher/wiki/Supported-Platforms
  - the STM8 bootloader has to be enabled for uploading code
    - for a virgin device this is automatically the case
    - uploading code via STM8_serial_flasher activates the BSL by default (see 'usage')
    - for other devices the BSL can be enabled via the free "ST Visual Programmer" tool (Windows only) by STM
  - this software includes some RAM code by STM, which is required for flash programming. This code was copied from the freely available "Flash Loader Demonstrator" tool by STM. All rights to the contained RAM code remain with STM.

For more details and instructions on building and using the tool see the Wiki under https://github.com/gicking/STM8_serial_flasher/wiki

====================================

Revision History
----------------

1.0 (2014-12-21): initial release by Georg Icking-Konert under the Apache License 2.0
