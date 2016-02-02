echo off
cls

REM download STM8 serial flasher from https://github.com/gicking/STM8_serial_flasher

REM set path to flash loader, COM port number, etc.
set LOADER=".\STM8_serial_flasher.exe"
set PORT=12
set FIRMWARE=".\dummy.s19"

REM set UART mode: 0=duplex, 1=1-wire reply, 2=2-wire reply
set MODE=0

REM acccording to STM8 bootloader manual section 2.1, the minimum baudrate is 4800Baud
set BAUD=4800
REM set BAUD=9600
REM set BAUD=19200
REM set BAUD=38400
REM set BAUD=57600 
REM set BAUD=115200 
REM set BAUD=230400 

echo.
echo.
echo enter STM8 bootloader and press return
echo.
echo.
PAUSE 

REM use flash loader to upload new SW
%LOADER% -p COM%PORT% -b %BAUD% -f %FIRMWARE% -u %MODE% -Q -v

echo.
PAUSE 
