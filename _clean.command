#!/bin/bash 

# change to current working directory
cd `dirname $0`

# just for output
echo off
clear

# use makefile to delete gcc output
make -f Makefile clean

# delete DevC++ output
rm -fr ./Objects

# delete doxygen output
rm -fr ./doxygen/html
rm -fr ./doxygen/latex
rm -fr ./doxygen/man
rm -fr ./doxygen/rtf
rm -fr ./doxygen/xml

# delete other output
rm -f ./.DS_Store
rm -f ./doxygen/.DS_Store
rm -f ./doxygen/images/.DS_Store
rm -f ./STM8_Routines/.DS_Store

echo " "
read -p "press key to close window..."
echo on

