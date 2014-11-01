#!/bin/bash

echo 
echo ----------------------------------------------------------------------
echo Building FFMPEG
echo 
cd ./libsrc/ffmpeg
sh build-ffmpeg-macosx.sh
cd ../../

echo 
echo ----------------------------------------------------------------------
echo Creating Makefile
echo 
cd src
qmake -spec macx-g++ imgcnv.pro

echo 
echo ----------------------------------------------------------------------
echo Building image convert
echo 
make -j 2
cd ..


