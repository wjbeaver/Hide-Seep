#!/bin/bash

echo 
echo ----------------------------------------------------------------------
echo Building FFMPEG
echo 
cd ./libsrc/ffmpeg
sh build-ffmpeg-linux.sh
cd ../../

echo ----------------------------------------------------------------------
echo Building bioView
echo ----------------------------------------------------------------------
qmake wv.pro
make

echo ----------------------------------------------------------------------
echo Building bioView Remote
echo ----------------------------------------------------------------------
qmake wvremote.pro
make



