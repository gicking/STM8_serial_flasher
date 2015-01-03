#!/bin/bash 

# change to current working directory
cd `dirname $0`

# just for output
echo off
clear

# make application
make -f Makefile

read -p "press key to close window..."
echo on

