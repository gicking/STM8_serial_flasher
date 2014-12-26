#!/bin/tcsh 

# change to current working directory
cd `dirname $0`

# just for output
echo off
clear

# make application
make -f Makefile

echo "press key to close window...\c"
set dummy = $<
echo on
