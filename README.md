STM8_serial_flasher
-------------------

PC tool for uplading hexfiles to the STM8 microcontroller via COM port or USB using the built-in ROM bootloader of the STM8. I wrote this tool, because the similar "Flash Loader Demonstrator" tool by STMicroelectronics (STM) only supports Windows.

Notes:
  - the tool has been tested under various Windows, MacOSX and Linux operating systems
  - for the STM8 Discovery Board, an additional UART->USB adapter is required, since the board  connects to the PC only via SWIM (=debug interface)
  - the STM8 bootloader has to be enabled via option bytes. This can be done using the "ST Visual Programmer“ (Windows only)
  - the STM8 bootloader protocol is described in application note UM0560, wich is available from the STM homepage

The tool includes some STM8 RAM code by STM, which is required for flash programming. This code was copied from the freely available "Flash Loader Demonstrator" by STM. All rights of this RAM code remain with STM.

For more details see the Wiki under https://github.com/gicking/STM8_serial_flasher/wiki

====================================

Revision History
----------------

1.0 (2014-12-21): initial release by Georg Icking-Konert under the Apache License 2.0

====================================

How to Compile
--------------

Windows: open file "STM_BSL_flasher.dev" with DevCpp and compile. Alternatively double-click '_compile.bat' (requires gcc and mingw32-make to be in PATH)

Posix (e.g. MacOSX or Linux): type "make" in project directory. Alternatively double-click '_compile.command' (requires gcc and make to be in PATH)

====================================

How to generate Reference
--------------

run Doxygen (www.doxygen.org) with input file 'Doxyfile'. Then open './doxygen/html/index.html'. For other output formats, e.g. PDF modify
'Doxyfile' accordingly.

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

  -x    don't enable ROM bootloader after upload (default: enable)
      
  -j    don't jump to flash after upload

  -q    don't prompt for <return> prior to exit
