#!/bin/tcsh 

# change to current working directory
cd `dirname $0`

# just for output
echo off
clear

# make hexfile
make -f Makefile clean

# also delete other output folders
rm -fr ./doxygen/html
rm -fr ./doxygen/latex
rm -fr ./doxygen/man
rm -fr ./doxygen/rtf
rm -fr ./doxygen/xml

echo "press key to close window...\c"
set dummy = $<
echo on
