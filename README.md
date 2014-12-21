STM8_serial_flasher
-------------------

PC tool for uplading hexfiles to the STM8 microcontroller via COM port or USB using the built-in ROM bootloader of the STM8. I wrote this tool, because the similar "Flash Loader Demonstrator" tool by STMicroelectronics (STM) only supports Windows.

Notes:
  - the tool works under Windows, MacOSX and Linux operating systems
  - for the STM8 Discovery Board, you have to connect the STM8 to the PC via a UART->USB adapter. The board itself connects to the PC via SWIM (=debug interface) only
  - the STM8 bootloader has to be enabled via option bytes. This can be done using the "ST Visual Programmer“ (Windows only)

The tool includes some STM8 code by STM, which is required for programming the flash. This code was copied from the freely available "Flash Loader Demonstrator" by STM. Rights to the contained STM8 code remain with STM.

====================================

Revision History
----------------

2014-12-21: initial release by Georg Icking-Konert under the Apache License 2.0

====================================

How to Compile
--------------

Windows: open file "STM_BSL_flasher.dev" with DevCpp and compile

Posix (e.g. MacOSX or Linux): type "make" in project directory

====================================

How to Use
----------

call this program from the command line or via command batchfile using the following syntax. For details on modes see STM8 bootloader manual on STM homepage.

usage: STM8_serial_flasher [-h] [-p COMx] [-b BR] [r] [-f hexfile] [-j] [-q]

  -h    print this help

  -p    name of communication port (default: list all ports and query)

  -b    communication baudrate in Baud (default: 115200)

  -r    use LIN reply mode

  -f    name of hexfile to flash (default: none)

  -j    don't jump to flash after upload

  -q    don't prompt for <return> prior to exit
