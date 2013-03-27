#!/bin/bash
for i in $(cd ..; ls *.c *.h); do ln -s ../$i; done
ln -s ../Graphics .
ln -s ../Libraries .
ln -s ../VectorLibrary .
ln -s ../bezier .
ln -sf VGA_dummy.h VGA.h
ln -sf Button_dummy.h Button.h
