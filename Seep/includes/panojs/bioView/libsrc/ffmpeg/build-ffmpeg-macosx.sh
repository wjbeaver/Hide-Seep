#!/bin/bash

chmod a+x ./ffmpeg/configure
chmod a+x ./ffmpeg/version.sh
chmod a+x ./ffmpeg/doc/texi2pod.pl

rm -R ./ffmpeg-obj
rm -R ./ffmpeg-out

#powerpc  x86  x86_32  x86_64
# -arch ppc -arch i386 

#####################################################
# The default compile is now Intel
#####################################################

mkdir -p ffmpeg-obj
cd ffmpeg-obj
../ffmpeg/configure \
  --enable-static --disable-shared \
  --prefix=$PWD/../ffmpeg-out \
  --enable-gpl --enable-swscale \
  --disable-ffserver --disable-ffplay --disable-network --disable-ffmpeg --disable-devices #\
  #--arch=x86 --enable-cross-compile \
  #--extra-cflags="-arch i386" \
  #--extra-libs="-headerpad_max_install_names -arch i386"

#--arch=x86 --cpu=i686 --enable-cross-compile \
#--enable-cross-compile

make all install

#rm -R ../include
mkdir ../include
mkdir ../include/ffmpeg
cp -R ./include/ffmpeg ../include/ffmpeg

cd ..
mkdir ../../libs
mkdir ../../libs/macosx

rm ../../libs/macosx/libavcodec.a
rm ../../libs/macosx/libavformat.a
rm ../../libs/macosx/libavutil.a
rm ../../libs/macosx/libswscale.a

cp ./ffmpeg-out/lib/libavcodec.a ../../libs/macosx/libavcodec.a
cp ./ffmpeg-out/lib/libavformat.a ../../libs/macosx/libavformat.a
cp ./ffmpeg-out/lib/libavutil.a ../../libs/macosx/libavutil.a
cp ./ffmpeg-out/lib/libswscale.a ../../libs/macosx/libswscale.a

