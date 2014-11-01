#!/bin/bash

echo 
echo ----------------------------------------------------------------------
echo Building FFMPEG
echo 
cd ./libsrc/ffmpeg
sh build-ffmpeg-linux.sh
cd ../../

echo 
echo ----------------------------------------------------------------------
echo Creating Makefile
echo 
cd src
qmake imgcnv.pro

echo 
echo ----------------------------------------------------------------------
echo Building image convert
echo 
make -j 2

cd ..
