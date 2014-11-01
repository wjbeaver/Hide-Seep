#!/bin/bash

cd ffmpeg

export PATH=/cygdrive/c/Programs/Develop/mingw64/bin:$PATH


#set MINGWDIR=c:\Programs\Develop\mingw64
#set PATH=%MINGWDIR%\bin;%PATH%
#set INCLUDE=%MINGWDIR%\INCLUDE%INCLUDE%
#set LIB=%MINGWDIR%\lib;%LIB%
#set LIBPATH=%MINGWDIR%\LIB;%LIBPATH%

make all install
