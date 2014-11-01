#!/bin/bash

chmod a+x ./ffmpeg/configure
chmod a+x ./ffmpeg/version.sh
chmod a+x ./ffmpeg/doc/texi2pod.pl

rm -R ./ffmpeg-obj
rm -R ./ffmpeg-out

#powerpc  x86  x86_32  x86_64
# -arch ppc -arch i386 

#####################################################
# ppc
#####################################################

mkdir -p ffmpeg-obj
cd ffmpeg-obj
../ffmpeg/configure \
  --enable-static --disable-shared \
  --prefix=$PWD/../ffmpeg-out \
  --enable-gpl --enable-swscale \
  --disable-ffserver --disable-ffplay --disable-vhook --disable-network --disable-ffmpeg --disable-devices \
  --arch=powerpc --enable-cross-compile \
  --extra-cflags="-arch ppc" \
  --extra-libs="-headerpad_max_install_names -arch ppc"

#make all install
#
##rm -R ../include
#mkdir ../include
#mkdir ../include/ffmpeg
#cp -R ./include/ffmpeg ../include/ffmpeg
#
#cd ..
#mkdir ../../libs
#mkdir ../../libs/macosx
#
#rm ../../libs/macosx/libavcodec_ppc.a
#rm ../../libs/macosx/libavformat_ppc.a
#rm ../../libs/macosx/libavutil_ppc.a
#rm ../../libs/macosx/libswscale_ppc.a
#
#cp ./ffmpeg-out/lib/libavcodec.a ../../libs/macosx/libavcodec_ppc.a
#cp ./ffmpeg-out/lib/libavformat.a ../../libs/macosx/libavformat_ppc.a
#cp ./ffmpeg-out/lib/libavutil.a ../../libs/macosx/libavutil_ppc.a
#cp ./ffmpeg-out/lib/libswscale.a ../../libs/macosx/libswscale_ppc.a

