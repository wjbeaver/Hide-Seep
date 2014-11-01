######################################################################
# Manually generated !!!
# LibBioImage v 1.26 Project file
# run: qmake libbioimg.pro in order to generate Makefile for your platform
# Copyright (c) 2005-08, Bio-Image Informatic Center, UCSB
#
# To generate Makefile on any platform:
#   qmake libbioimg.pro
#
# To generate VisualStudio project file:
#   qmake -t vcapp -spec win32-msvc2005 libbioimg.pro
#   qmake -t vcapp -spec win32-msvc.net libbioimg.pro
#   qmake -t vcapp -spec win32-msvc libbioimg.pro
#   qmake -spec win32-icc libbioimg.pro # to use pure Intel Compiler
#
# To generate xcode project file:
#   qmake -spec macx-xcode libbioimg.pro
#
######################################################################

#---------------------------------------------------------------------
# configuration: editable
#---------------------------------------------------------------------

TEMPLATE = lib
VERSION = 0.0.3
#CONFIG  += dll
CONFIG  += staticlib

CONFIG += console 
CONFIG += release
CONFIG += warn_off

# static library config
# nothing defined uses dynamic system version
# defining "stat_" - uses embedded version
# defining "sts_sys_" - uses static system version

CONFIG += stat_libtiff
#CONFIG += stat_sys_libtiff

CONFIG += stat_libjpeg
#CONFIG += stat_sys_libjpeg

CONFIG += stat_libpng
#CONFIG += stat_sys_libpng

CONFIG += stat_zlib
#CONFIG += stat_sys_zlib

# lib ffmpeg at this point is forced to be local copy since the trunc changes too much
CONFIG += ffmpeg

#---------------------------------------------------------------------
# configuration paths: editable
#---------------------------------------------------------------------

DN_SRC  = ./
DN_LSRC = ../
DN_LIBS = ../../libs
DN_IMGS = ../../images

unix {
  DN_GENS = ../../generated/$(HOSTTYPE)
  # path for object files
  DN_OBJ = $$DN_GENS/obj
  # path for generated binary
  DN_BIN = ../../$(HOSTTYPE)
}
win32 {
  DN_GENS = ../../generated/$(PlatformName)/$(ConfigurationName)
  # path for object files
  DN_OBJ = $$DN_GENS
  # path for generated binary
  DN_BIN = ../../$(PlatformName)/$(ConfigurationName)
}


DN_LIB_TIF = $$DN_LSRC/libtiff
DN_LIB_JPG = $$DN_LSRC/libjpeg
DN_LIB_PNG = $$DN_LSRC/libpng
DN_LIB_Z   = $$DN_LSRC/zlib

DN_CORE     = $$DN_SRC/core_lib
DN_FMTS     = $$DN_SRC/formats
DN_FMTS_API = $$DN_SRC/formats_api

ffmpeg {
  DN_LIB_FFMPEG = $$DN_LSRC/ffmpeg
  DN_FMT_FFMPEG = $$DN_FMTS/mpeg
  DEFINES += DIM_FFMPEG_FORMAT
}

#---------------------------------------------------------------------
# configuration: automatic
#---------------------------------------------------------------------

# enable the following only for 10.4 and universal binary generation
#macx:QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
macx:LIBS += -faltivec -framework vecLib
macx:CONFIG+=x86 ppc
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4

win32 {
  DEFINES += _CRT_SECURE_NO_WARNINGS
}

DN_LIBS_PLTFM = $$DN_LIBS
win32: {
  DN_LIBS_PLTFM = $$DN_LIBS/vc2008
} else:macx {
  DN_LIBS_PLTFM = $$DN_LIBS/macosx
} else:unix {
  DN_LIBS_PLTFM = $$DN_LIBS/linux/$(HOSTTYPE)
} else {
  DN_LIBS_PLTFM = $$DN_LIBS/linux
}  

!exists( $$DN_GENS ) {
  message( "Cannot find directory: $$DN_GENS, creating..." )
  system( mkdir $$DN_GENS )
}

!exists( $$DN_OBJ ) {
  message( "Cannot find directory: $$DN_OBJ, creating..." )
  system( mkdir $$DN_OBJ )
}

# compile ffmpeg
ffmpeg {
 
  #message( "Compiling FFMPEG as requested..." )
  #win32 {
  #  message( "Attention: Cygwin and VC++2005 are required for ffmpeg windows compilation" )  
  #  system( cd $$DN_LIB_FFMPEG ; sh ./build-ffmpeg-cygwin.sh ; cd ../../ )
  #} else:macx {
  #  system( cd $$DN_LIB_FFMPEG ; sh ./build-ffmpeg-macosx.sh ; cd ../../ )
  #} else {
  #  system( cd $$DN_LIB_FFMPEG ; sh ./build-ffmpeg-linux.sh ; cd ../../ )  
  #}  
}

#---------------------------------------------------------------------
# library configuration: automatic
#---------------------------------------------------------------------

# By default all libraries are static, we can check for existing
# once and use dynamic linking for those

unix {

  exists( $$DN_GENS ) {
    #checking for libtiff... -ltiff
    #CONFIG -= stat_libtiff
  }
  
  exists( $$DN_GENS ) {
    #checking for libjpeg... -ljpeg
    #CONFIG -= stat_libjpeg
  }
  
  exists( $$DN_GENS ) {  
    #checking for libpng... -lpng -lz -lm
    #CONFIG -= stat_libpng
  }

  exists( $$DN_GENS ) {
    #checking for libz... -lz
    #CONFIG -= stat_zlib
  }

}

# mac os x
macx {
  CONFIG -= stat_zlib
}


#---------------------------------------------------------------------
# generation: fixed
#---------------------------------------------------------------------

CONFIG  -= qt x11 windows

MOC_DIR = $$DN_GENS
DESTDIR = $$DN_BIN
OBJECTS_DIR = $$DN_OBJ

INCLUDEPATH += $$DN_GENS
DEPENDPATH += $$DN_GENS

INCLUDEPATH += $$DN_FMTS_API
INCLUDEPATH += $$DN_FMTS
INCLUDEPATH += $$DN_CORE

stat_libtiff:INCLUDEPATH += $$DN_LIB_TIF
stat_libjpeg:INCLUDEPATH += $$DN_LIB_JPG
stat_libpng:INCLUDEPATH += $$DN_LIB_PNG
stat_zlib:INCLUDEPATH += $$DN_LIB_Z

#---------------------------------------------------------------------
# bio image formats library           
#---------------------------------------------------------------------

#core
SOURCES += $$DN_CORE/xstring.cpp $$DN_CORE/xtypes.cpp \
           $$DN_CORE/tag_map.cpp $$DN_CORE/xpointer.cpp


#Formats API 
SOURCES += $$DN_FMTS_API/dim_img_format_utils.cpp \
           $$DN_FMTS_API/dim_buffer.cpp \
           $$DN_FMTS_API/dim_histogram.cpp \
           $$DN_FMTS_API/dim_image.cpp \
           $$DN_FMTS_API/dim_image_pyramid.cpp \
           $$DN_FMTS_API/dim_image_stack.cpp
                                                                  
           
#Formats     
SOURCES += $$DN_FMTS/dim_format_manager.cpp \
           $$DN_FMTS/meta_format_manager.cpp\
           $$DN_FMTS/tiff/dim_tiff_format.cpp \
           $$DN_FMTS/tiff/dim_xtiff.c \
           $$DN_FMTS/tiff/memio.c \
           $$DN_FMTS/dmemio.cpp \
           $$DN_FMTS/jpeg/dim_jpeg_format.cpp \
           $$DN_FMTS/biorad_pic/dim_biorad_pic_format.cpp \
           $$DN_FMTS/bmp/dim_bmp_format.cpp \
           $$DN_FMTS/png/dim_png_format.cpp \
           $$DN_FMTS/nanoscope/dim_nanoscope_format.cpp \
           $$DN_FMTS/raw/dim_raw_format.cpp \
           $$DN_FMTS/ibw/dim_ibw_format.cpp \
           $$DN_FMTS/ome/dim_ome_format.cpp\
           $$DN_FMTS/oib/dim_oib_format.cpp
           
           
#---------------------------------------------------------------------        
#ffmpeg

ffmpeg {

  DEFINES  += DIM_FFMPEG_FORMAT FFMPEG_VIDEO_DISABLE_MATLAB
  INCLUDEPATH += $$DN_LIB_FFMPEG/include
  #INCLUDEPATH += $$DN_LIB_FFMPEG/ffmpeg-out/include
  win32:INCLUDEPATH += $$DN_LIB_FFMPEG/include-win32

  SOURCES += $$DN_FMT_FFMPEG/debug.cpp $$DN_FMT_FFMPEG/dim_ffmpeg_format.cpp \
             $$DN_FMT_FFMPEG/FfmpegCommon.cpp $$DN_FMT_FFMPEG/FfmpegIVideo.cpp \
             $$DN_FMT_FFMPEG/FfmpegOVideo.cpp $$DN_FMT_FFMPEG/registry.cpp
	   
	win32 {   
    LIBS += $$DN_LIBS_PLTFM/avcodec-51.lib
    LIBS += $$DN_LIBS_PLTFM/avformat-52.lib
    LIBS += $$DN_LIBS_PLTFM/avutil-49.lib
    LIBS += $$DN_LIBS_PLTFM/swscale-0.lib
  } else {
    #LIBS += $$DN_LIB_FFMPEG/ffmpeg-out/lib/libavformat.a
    #LIBS += $$DN_LIB_FFMPEG/ffmpeg-out/lib/libavcodec.a
    #LIBS += $$DN_LIB_FFMPEG/ffmpeg-out/lib/libswscale.a
    #LIBS += $$DN_LIB_FFMPEG/ffmpeg-out/lib/libavutil.a
    
    LIBS += $$DN_LIBS_PLTFM/libavformat.a
    LIBS += $$DN_LIBS_PLTFM/libavcodec.a
    LIBS += $$DN_LIBS_PLTFM/libswscale.a
    LIBS += $$DN_LIBS_PLTFM/libavutil.a  
  }
}  
            
#---------------------------------------------------------------------        
#Pole

D_LIB_POLE = $$DN_LSRC/pole
INCLUDEPATH += $$D_LIB_POLE

SOURCES += $$D_LIB_POLE/pole.cpp
           
#---------------------------------------------------------------------
# Now adding static libraries
#---------------------------------------------------------------------

#some configs first
unix:DEFINES  += HAVE_UNISTD_H
unix:DEFINES  -= HAVE_IO_H

win32:DEFINES += HAVE_IO_H

macx:DEFINES  += HAVE_UNISTD_H
macx:DEFINES  += WORDS_BIGENDIAN          
macx:DEFINES  -= HAVE_IO_H

#---------------------------------------------------------------------
#LibTiff
stat_libtiff {

SOURCES += $$DN_LIB_TIF/tif_fax3sm.c $$DN_LIB_TIF/tif_aux.c \
           $$DN_LIB_TIF/tif_close.c $$DN_LIB_TIF/tif_codec.c \
           $$DN_LIB_TIF/tif_color.c $$DN_LIB_TIF/tif_compress.c \
           $$DN_LIB_TIF/tif_dir.c $$DN_LIB_TIF/tif_dirinfo.c \
           $$DN_LIB_TIF/tif_dirread.c $$DN_LIB_TIF/tif_dirwrite.c \
           $$DN_LIB_TIF/tif_dumpmode.c $$DN_LIB_TIF/tif_error.c \
           $$DN_LIB_TIF/tif_extension.c $$DN_LIB_TIF/tif_fax3.c \
           $$DN_LIB_TIF/tif_flush.c $$DN_LIB_TIF/tif_getimage.c \
           $$DN_LIB_TIF/tif_jpeg.c $$DN_LIB_TIF/tif_luv.c \
           $$DN_LIB_TIF/tif_lzw.c $$DN_LIB_TIF/tif_next.c \
           $$DN_LIB_TIF/tif_open.c $$DN_LIB_TIF/tif_packbits.c \
           $$DN_LIB_TIF/tif_pixarlog.c $$DN_LIB_TIF/tif_predict.c \
           $$DN_LIB_TIF/tif_print.c $$DN_LIB_TIF/tif_read.c \
           $$DN_LIB_TIF/tif_strip.c $$DN_LIB_TIF/tif_swab.c \
           $$DN_LIB_TIF/tif_thunder.c $$DN_LIB_TIF/tif_tile.c \
           $$DN_LIB_TIF/tif_version.c $$DN_LIB_TIF/tif_warning.c \
           $$DN_LIB_TIF/tif_write.c $$DN_LIB_TIF/tif_zip.c
           
unix:SOURCES  += $$DN_LIB_TIF/tif_unix.c
win32:SOURCES += $$DN_LIB_TIF/tif_win32.c
}

!stat_libtiff {
  unix {
    stat_sys_libtiff {
      LIBS += /usr/lib/libtiff.a
    } else {
      LIBS += -ltiff
    }
  }
  win32:LIBS += $$DN_LIBS_PLTFM/libtiff.lib  
}

#---------------------------------------------------------------------        
#LibPng

# by default disable intel asm code
unix:DEFINES += PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD

# enable only for x86 machines (not x64)
#message( "Enable Intel ASM code for PNG..." )
win32:DEFINES -= PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD
macx:DEFINES -= PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD
linux-g++-32:DEFINES -= PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD

stat_libpng {
SOURCES += $$DN_LIB_PNG/png.c $$DN_LIB_PNG/pngerror.c $$DN_LIB_PNG/pngget.c \
           $$DN_LIB_PNG/pngmem.c $$DN_LIB_PNG/pngpread.c $$DN_LIB_PNG/pngread.c \
           $$DN_LIB_PNG/pngrio.c $$DN_LIB_PNG/pngrtran.c $$DN_LIB_PNG/pngrutil.c \
           $$DN_LIB_PNG/pngset.c $$DN_LIB_PNG/pngtrans.c $$DN_LIB_PNG/pngvcrd.c \
           $$DN_LIB_PNG/pngwio.c $$DN_LIB_PNG/pngwrite.c $$DN_LIB_PNG/pngwtran.c \
           $$DN_LIB_PNG/pngwutil.c $$DN_LIB_PNG/pnggccrd.c
}

!stat_libpng {
  unix {
    stat_sys_libpng {
      LIBS += /usr/lib/libpng.a
    } else {
      LIBS += -lpng
    }
  }
  win32:LIBS += $$DN_LIBS_PLTFM/libpng.lib
}

  
#---------------------------------------------------------------------         
#ZLib
stat_zlib {
SOURCES += $$DN_LIB_Z/adler32.c $$DN_LIB_Z/compress.c $$DN_LIB_Z/crc32.c \
           $$DN_LIB_Z/deflate.c $$DN_LIB_Z/gzio.c $$DN_LIB_Z/infback.c \
           $$DN_LIB_Z/inffast.c $$DN_LIB_Z/inflate.c $$DN_LIB_Z/inftrees.c \
           $$DN_LIB_Z/trees.c $$DN_LIB_Z/uncompr.c $$DN_LIB_Z/zutil.c
}

!stat_zlib {
  unix {
    stat_sys_zlib {
      LIBS += /usr/lib/libz.a
    } else {
      LIBS += -lz
    }
  }
  win32:LIBS += $$DN_LIBS_PLTFM/zlib.lib  
}

#---------------------------------------------------------------------        
#libjpeg
stat_libjpeg {
SOURCES += $$DN_LIB_JPG/jcapimin.c $$DN_LIB_JPG/jcapistd.c $$DN_LIB_JPG/jccoefct.c \
	   $$DN_LIB_JPG/jccolor.c $$DN_LIB_JPG/jcdctmgr.c $$DN_LIB_JPG/jchuff.c \
	   $$DN_LIB_JPG/jcinit.c $$DN_LIB_JPG/jcmainct.c $$DN_LIB_JPG/jcmarker.c \
	   $$DN_LIB_JPG/jcmaster.c $$DN_LIB_JPG/jcomapi.c $$DN_LIB_JPG/jcparam.c \
	   $$DN_LIB_JPG/jcphuff.c $$DN_LIB_JPG/jcprepct.c $$DN_LIB_JPG/jcsample.c \
	   $$DN_LIB_JPG/jctrans.c $$DN_LIB_JPG/jdapimin.c $$DN_LIB_JPG/jdapistd.c \
	   $$DN_LIB_JPG/jdatadst.c $$DN_LIB_JPG/jdatasrc.c $$DN_LIB_JPG/jdcoefct.c \
	   $$DN_LIB_JPG/jdcolor.c $$DN_LIB_JPG/jddctmgr.c $$DN_LIB_JPG/jdhuff.c \
	   $$DN_LIB_JPG/jdinput.c $$DN_LIB_JPG/jdmainct.c $$DN_LIB_JPG/jdmarker.c \
	   $$DN_LIB_JPG/jdmaster.c $$DN_LIB_JPG/jdmerge.c $$DN_LIB_JPG/jdphuff.c \
	   $$DN_LIB_JPG/jdpostct.c $$DN_LIB_JPG/jdsample.c $$DN_LIB_JPG/jdtrans.c \
	   $$DN_LIB_JPG/jerror.c $$DN_LIB_JPG/jfdctflt.c $$DN_LIB_JPG/jfdctfst.c \
	   $$DN_LIB_JPG/jfdctint.c $$DN_LIB_JPG/jidctflt.c $$DN_LIB_JPG/jidctfst.c \
	   $$DN_LIB_JPG/jidctint.c $$DN_LIB_JPG/jidctred.c $$DN_LIB_JPG/jquant1.c \
	   $$DN_LIB_JPG/jquant2.c $$DN_LIB_JPG/jutils.c $$DN_LIB_JPG/jmemmgr.c \
	   $$DN_LIB_JPG/jmemansi.c
}
   
!stat_libjpeg {
  unix {
    stat_sys_libjpeg {
      LIBS += /usr/lib/libjpeg.a
    } else {
      LIBS += -ljpeg
    }
  }
  win32:LIBS += $$DN_LIBS_PLTFM/libjpeg.lib
}   
 


