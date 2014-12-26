REM just for output
echo off
cls

REM set make tool (if not in PATH, set complete path)
set MAKE=mingw32-make

REM use SDCC makefile to delete output
%MAKE% -f Makefile clean

# also delete other output folders
rd /S /Q  .\doxygen\html
rd /S /Q  .\doxygen\latex
rd /S /Q  .\doxygen\man
rd /S /Q  .\doxygen\rtf
rd /S /Q  .\doxygen\xml

echo on
