#!/bin/bash

echo 
echo ----------------------------------------------------------------------
echo Building bioView
echo 
qmake wv.pro
make

echo 
echo ----------------------------------------------------------------------
echo Building bioView Remote
echo 
qmake wvremote.pro
make


