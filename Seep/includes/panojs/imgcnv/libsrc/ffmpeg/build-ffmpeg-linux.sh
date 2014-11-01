#!/bin/bash

chmod a+x ./ffmpeg/configure
chmod a+x ./ffmpeg/version.sh
chmod a+x ./ffmpeg/doc/texi2pod.pl

rm -Rf ./ffmpeg-obj
rm -Rf ./ffmpeg-out

mkdir -p ffmpeg-obj
cd ffmpeg-obj
../ffmpeg/configure \
	--enable-static --disable-shared \
	--prefix=$PWD/../ffmpeg-out \
	--enable-gpl --enable-swscale \
	--disable-ffserver --disable-ffplay --disable-network --disable-ffmpeg --disable-devices

make all install

cd ..

mkdir -p ./include
cp -Rf ./ffmpeg-out/include/libavcodec ./include/
cp -Rf ./ffmpeg-out/include/libavformat ./include/
cp -Rf ./ffmpeg-out/include/libavutil ./include/
cp -Rf ./ffmpeg-out/include/libswscale ./include/

mkdir -p ../../libs
mkdir -p ../../libs/linux

rm -f ../../libs/linux/libavcodec.a
rm -f ../../libs/linux/libavformat.a
rm -f ../../libs/linux/libavutil.a
rm -f ../../libs/linux/libswscale.a

cp ./ffmpeg-out/lib/libavcodec.a ../../libs/linux/libavcodec.a
cp ./ffmpeg-out/lib/libavformat.a ../../libs/linux/libavformat.a
cp ./ffmpeg-out/lib/libavutil.a ../../libs/linux/libavutil.a
cp ./ffmpeg-out/lib/libswscale.a ../../libs/linux/libswscale.a


