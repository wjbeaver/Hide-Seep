#!/bin/bash

chmod a+x ./ffmpeg/configure
chmod a+x ./ffmpeg/version.sh
chmod a+x ./ffmpeg/doc/texi2pod.pl

rm -Rf ./ffmpeg-obj
rm -Rf ./ffmpeg-out

#####################################################
#powerpc  x86  x86_32  x86_64
# -arch ppc -arch i386 
#####################################################

mkdir -p ffmpeg-obj
cd ffmpeg-obj
../ffmpeg/configure \
  --enable-static --disable-shared \
  --prefix=$PWD/../ffmpeg-out \
  --enable-gpl --enable-swscale \
  --disable-ffserver --disable-ffplay --disable-vhook --disable-network --disable-ffmpeg --disable-devices \
  --arch=generic --enable-cross-compile \
  --extra-cflags="-arch ppc -arch i386" \
  --extra-libs="-headerpad_max_install_names -arch ppc -arch i386"

make all install

cd ..

mkdir -p ./include
cp -Rf ./ffmpeg-out/include/libavcodec ./include/
cp -Rf ./ffmpeg-out/include/libavformat ./include/
cp -Rf ./ffmpeg-out/include/libavutil ./include/
cp -Rf ./ffmpeg-out/include/libswscale ./include/

mkdir -p ../../libs
mkdir -p ../../libs/macosx

rm -f ../../libs/macosx/libavcodec.a
rm -f ../../libs/macosx/libavformat.a
rm -f ../../libs/macosx/libavutil.a
rm -f ../../libs/macosx/libswscale.a

cp ./ffmpeg-out/lib/libavcodec.a ../../libs/macosx/libavcodec.a
cp ./ffmpeg-out/lib/libavformat.a ../../libs/macosx/libavformat.a
cp ./ffmpeg-out/lib/libavutil.a ../../libs/macosx/libavutil.a
cp ./ffmpeg-out/lib/libswscale.a ../../libs/macosx/libswscale.a


