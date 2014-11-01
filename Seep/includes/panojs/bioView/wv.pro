######################################################################
# Manually generated !!!
# BioView v 1.0.0 Project file
# run: qmake wv.pro in order to generate Makefile for your platform
# Copyright (c) 2005-07, Bio-Image Informatic Center, UCSB
#
# To generate Makefile on any platform:
#   qmake wv.pro
#
# To generate VisualStudio project file:
#   qmake -t vcapp -spec win32-msvc2005 wv.pro
#   qmake -t vcapp -spec win32-msvc.net wv.pro
#   qmake -t vcapp -spec win32-msvc wv.pro
#   qmake -spec win32-icc wv.pro # to use pure Intel Compiler
#
# To generate xcode project file:
#   qmake -spec macx-xcode wv.pro 
#  
# To generate Makefile on MacOSX with binary install:
#   qmake -spec macx-g++ wv.pro
#
######################################################################

#---------------------------------------------------------------------
# configuration: editable
#---------------------------------------------------------------------
VERSION = 1.1.18

CONFIG += release
CONFIG += warn_off

# mac universal binaries, qt MUST to be configured with "universal",
# otherwise comment out two following lines
#macx:QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
#macx:CONFIG+=x86 ppc
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4

win32:CONFIG += embed_manifest_exe # VC2005 manifests

# static library config, comment to disable
CONFIG += stat_libtiff
CONFIG += stat_libjpeg
CONFIG += stat_libpng
CONFIG += stat_zlib
CONFIG += ffmpeg
CONFIG += bisquik

#CONFIG += stat_bzlib
#CONFIG += sys_bzlib
macx:CONFIG += sys_bzlib

#---------------------------------------------------------------------
# configuration paths: editable
#---------------------------------------------------------------------

DN_SRC  = ./src
DN_LSRC = ./libsrc
DN_LIBS = ./libs
DN_IMGS = ./images
DN_GENS = ./GeneratedFiles
DN_ICNS = ./icons

# path for object files
DN_OBJ = $$DN_GENS/obj
# path for generated binary
DN_BIN = ./

DN_LIB_TIF = $$DN_LSRC/libtiff
DN_LIB_JPG = $$DN_LSRC/libjpeg
DN_LIB_PNG = $$DN_LSRC/libpng
DN_LIB_Z   = $$DN_LSRC/zlib
DN_LIB_BIO = $$DN_LSRC/libbioimg

DN_UI       = $$DN_SRC/ui
DN_VIEW     = $$DN_SRC/viewer
DN_NETW     = $$DN_SRC/network
DN_WIDG     = $$DN_SRC/widgets

DN_FMTS     = $$DN_LIB_BIO/formats
DN_FMTS_API = $$DN_LIB_BIO/formats_api
DN_CORE     = $$DN_LIB_BIO/core_lib

#---------------------------------------------------------------------
# configuration: automatic
#---------------------------------------------------------------------

DN_LIBS_PLTFM = $$DN_LIBS
win32: {
  DN_LIBS_PLTFM = $$DN_LIBS/vc2008
} else:macx {
  DN_LIBS_PLTFM = $$DN_LIBS/macosx
} else {
  DN_LIBS_PLTFM = $$DN_LIBS/linux
}  

#macx:LIBS += -faltivec -framework vecLib

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

TEMPLATE = app

CONFIG += qt
CONFIG += thread 

QT += xml network webkit

macx:RC_FILE = $$DN_ICNS/wv.icns

MOC_DIR = $$DN_GENS
DESTDIR = $$DN_BIN
OBJECTS_DIR = $$DN_OBJ
UI_DIR = $$DN_GENS

INCLUDEPATH += $$DN_GENS
DEPENDPATH += $$DN_GENS

INCLUDEPATH += $$DN_SRC
INCLUDEPATH += $$DN_FMTS
INCLUDEPATH += $$DN_FMTS_API
INCLUDEPATH += $$DN_CORE
INCLUDEPATH += $$DN_VIEW
INCLUDEPATH += $$DN_NETW
INCLUDEPATH += $$DN_WIDG

# if need to include private qt headers
qt_noprivate {
  INCLUDEPATH += $$DN_SRC/models
}

# Main headers are needed to invoke moc where needed
HEADERS += $$DN_SRC/qresource.h

HEADERS += $$DN_VIEW/controlsWidget.h\
           $$DN_VIEW/tileviewer.h\
           $$DN_VIEW/wvimagemanager.h\
           $$DN_VIEW/scrollImageView.h\
           $$DN_VIEW/subAreaLabel.h

# Main app
SOURCES += $$DN_SRC/main.cpp\
           $$DN_VIEW/controlsWidget.cpp\
           $$DN_VIEW/tileviewer.cpp\
           $$DN_VIEW/wvimagemanager.cpp\
           $$DN_VIEW/scrollImageView.cpp\
           $$DN_VIEW/subAreaLabel.cpp
           
#############################################                      
# Widgets
HEADERS += $$DN_WIDG/notifyWidget.h\
           $$DN_WIDG/progresscircle.h\
           $$DN_WIDG/scalebar.h\
           $$DN_WIDG/dsingleapplication.h\
           $$DN_WIDG/d_group_box.h\
           $$DN_WIDG/d_range_slider.h\
           $$DN_WIDG/d_histogram_widget.h

# Main app
SOURCES += $$DN_WIDG/notifyWidget.cpp\
           $$DN_WIDG/appconfig.cpp\
           $$DN_WIDG/progresscircle.cpp\
           $$DN_WIDG/scalebar.cpp\
           $$DN_WIDG/dsingleapplication.cpp\
           $$DN_WIDG/d_group_box.cpp\
           $$DN_WIDG/d_range_slider.cpp\
           $$DN_WIDG/d_histogram_widget.cpp\
           $$DN_WIDG/d_reg_exp.cpp         
           
#############################################           
# Network headers
HEADERS += $$DN_NETW/wvmessenger.h\
           $$DN_NETW/wvclient.h\
           $$DN_NETW/wvserver.h\
           $$DN_NETW/wvmessageparser.h

# Network part
SOURCES += $$DN_NETW/wvmessenger.cpp\
           $$DN_NETW/wvclient.cpp\
           $$DN_NETW/wvserver.cpp\
           $$DN_NETW/wvmessageparser.cpp
                 
#---------------------------------------------------------------------
# Forms
#---------------------------------------------------------------------           

FORMS = $$DN_UI/controls.ui

#---------------------------------------------------------------------
# bisquik    
#---------------------------------------------------------------------

bisquik {
  DN_LIB_BISQUIK = $$DN_LSRC/bisquik

  INCLUDEPATH += $$DN_LIB_BISQUIK

  HEADERS += $$DN_LIB_BISQUIK/bisquikAccess.h $$DN_LIB_BISQUIK/bisquikWebAccess.h
  SOURCES += $$DN_LIB_BISQUIK/bisquikAccess.cpp $$DN_LIB_BISQUIK/bisquikWebAccess.cpp
  FORMS += $$DN_LIB_BISQUIK/bisquikAccess.ui $$DN_LIB_BISQUIK/bisquikWebAccess.ui
}

#---------------------------------------------------------------------        
#Pole

D_LIB_POLE = $$DN_LSRC/pole
INCLUDEPATH += $$D_LIB_POLE

SOURCES += $$D_LIB_POLE/pole.cpp
           
#---------------------------------------------------------------------
# libbioimage    
#---------------------------------------------------------------------

INCLUDEPATH += $$DN_LIB_BIO
INCLUDEPATH += $$DN_FMTS
INCLUDEPATH += $$DN_FMTS_API
INCLUDEPATH += $$DN_CORE

DEFINES  += DIM_USE_QT

#core
SOURCES += $$DN_CORE/xstring.cpp $$DN_CORE/xtypes.cpp \
           $$DN_CORE/tag_map.cpp $$DN_CORE/xpointer.cpp

#Formats API 
SOURCES += $$DN_FMTS_API/dim_buffer.cpp \
           $$DN_FMTS_API/dim_histogram.cpp \
           $$DN_FMTS_API/dim_image.cpp \
           $$DN_FMTS_API/dim_image_stack.cpp \
           $$DN_FMTS_API/dim_image_pyramid.cpp \
           $$DN_FMTS_API/dim_img_format_utils.cpp \
           $$DN_FMTS_API/dim_qt_utils.cpp
           
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

  DN_LIB_FFMPEG = $$DN_LSRC/ffmpeg
  DN_FMT_FFMPEG = $$DN_FMTS/mpeg
  DEFINES += DIM_FFMPEG_FORMAT

  DEFINES  += DIM_FFMPEG_FORMAT FFMPEG_VIDEO_DISABLE_MATLAB
  INCLUDEPATH += $$DN_LIB_FFMPEG/include
  win32:INCLUDEPATH += $$DN_LIB_FFMPEG/include-win

  SOURCES += $$DN_FMT_FFMPEG/debug.cpp $$DN_FMT_FFMPEG/dim_ffmpeg_format.cpp \
             $$DN_FMT_FFMPEG/FfmpegCommon.cpp $$DN_FMT_FFMPEG/FfmpegIVideo.cpp \
             $$DN_FMT_FFMPEG/FfmpegOVideo.cpp $$DN_FMT_FFMPEG/registry.cpp
	   
	win32 {   
    LIBS += $$DN_LIBS_PLTFM/avcodec-51.lib
    LIBS += $$DN_LIBS_PLTFM/avformat-52.lib
    LIBS += $$DN_LIBS_PLTFM/avutil-50.lib
    LIBS += $$DN_LIBS_PLTFM/swscale-0.lib
  } else {
    LIBS += $$DN_LIBS_PLTFM/libavformat.a
    LIBS += $$DN_LIBS_PLTFM/libavcodec.a
    LIBS += $$DN_LIBS_PLTFM/libswscale.a
    LIBS += $$DN_LIBS_PLTFM/libavutil.a  
  }
}    
        
#---------------------------------------------------------------------
# Now adding static libraries
#---------------------------------------------------------------------

#some configs first
unix:DEFINES  += HAVE_UNISTD_H
unix:DEFINES  -= HAVE_IO_H

win32:DEFINES += HAVE_IO_H

macx:DEFINES  += HAVE_UNISTD_H
#macx:DEFINES  += WORDS_BIGENDIAN          
macx:DEFINES  -= HAVE_IO_H

#---------------------------------------------------------------------
#LibTiff

stat_libtiff {
  INCLUDEPATH += $$DN_LIB_TIF
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
  unix:LIBS += -ltiff
  win32:LIBS += $$DN_LSRC/libtiff.lib
}

#---------------------------------------------------------------------        
#LibPng

stat_libpng {
  INCLUDEPATH += $$DN_LIB_PNG

  # by default disable intel asm code
  unix:DEFINES += PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD

  # enable only for x86 machines (not x64)
  #message( "Enable Intel ASM code for PNG..." )
  win32:DEFINES -= PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD
  macx:DEFINES -= PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD
  linux-g++-32:DEFINES -= PNG_NO_ASSEMBLER_CODE PNG_USE_PNGVCRD

  SOURCES += $$DN_LIB_PNG/png.c $$DN_LIB_PNG/pngerror.c $$DN_LIB_PNG/pngget.c \
             $$DN_LIB_PNG/pngmem.c $$DN_LIB_PNG/pngpread.c $$DN_LIB_PNG/pngread.c \
             $$DN_LIB_PNG/pngrio.c $$DN_LIB_PNG/pngrtran.c $$DN_LIB_PNG/pngrutil.c \
             $$DN_LIB_PNG/pngset.c $$DN_LIB_PNG/pngtrans.c \
             $$DN_LIB_PNG/pngwio.c $$DN_LIB_PNG/pngwrite.c $$DN_LIB_PNG/pngwtran.c \
             $$DN_LIB_PNG/pngwutil.c
}
 
!stat_libpng {
  unix:LIBS += -lpng
  win32:LIBS += $$DN_LSRC/libpng.lib
} 
 
#---------------------------------------------------------------------         
#ZLib

stat_zlib {
  INCLUDEPATH += $$DN_LIB_Z
  SOURCES += $$DN_LIB_Z/adler32.c $$DN_LIB_Z/compress.c $$DN_LIB_Z/crc32.c \
             $$DN_LIB_Z/deflate.c $$DN_LIB_Z/gzio.c $$DN_LIB_Z/infback.c \
             $$DN_LIB_Z/inffast.c $$DN_LIB_Z/inflate.c $$DN_LIB_Z/inftrees.c \
             $$DN_LIB_Z/trees.c $$DN_LIB_Z/uncompr.c $$DN_LIB_Z/zutil.c
}

!stat_zlib {
  unix:LIBS += -lz
  win32:LIBS += $$DN_LSRC/zlib.lib  
}

#---------------------------------------------------------------------        
#libjpeg

stat_libjpeg {
  INCLUDEPATH += $$DN_LIB_JPG

  SOURCES += $$DN_LIB_JPG/jaricom.c $$DN_LIB_JPG/jcapimin.c $$DN_LIB_JPG/jcapistd.c \
       $$DN_LIB_JPG/jcarith.c $$DN_LIB_JPG/jccoefct.c \
	     $$DN_LIB_JPG/jccolor.c $$DN_LIB_JPG/jcdctmgr.c $$DN_LIB_JPG/jchuff.c \
	     $$DN_LIB_JPG/jcinit.c $$DN_LIB_JPG/jcmainct.c $$DN_LIB_JPG/jcmarker.c \
	     $$DN_LIB_JPG/jcmaster.c $$DN_LIB_JPG/jcomapi.c $$DN_LIB_JPG/jcparam.c \
	     $$DN_LIB_JPG/jcprepct.c $$DN_LIB_JPG/jcsample.c $$DN_LIB_JPG/jctrans.c \
	     $$DN_LIB_JPG/jdapimin.c $$DN_LIB_JPG/jdapistd.c $$DN_LIB_JPG/jdarith.c \
	     $$DN_LIB_JPG/jdatadst.c $$DN_LIB_JPG/jdatasrc.c $$DN_LIB_JPG/jdcoefct.c \
	     $$DN_LIB_JPG/jdcolor.c $$DN_LIB_JPG/jddctmgr.c $$DN_LIB_JPG/jdhuff.c \
	     $$DN_LIB_JPG/jdinput.c $$DN_LIB_JPG/jdmainct.c $$DN_LIB_JPG/jdmarker.c \
	     $$DN_LIB_JPG/jdmaster.c $$DN_LIB_JPG/jdmerge.c \
	     $$DN_LIB_JPG/jdpostct.c $$DN_LIB_JPG/jdsample.c $$DN_LIB_JPG/jdtrans.c \
	     $$DN_LIB_JPG/jerror.c $$DN_LIB_JPG/jfdctflt.c $$DN_LIB_JPG/jfdctfst.c \
	     $$DN_LIB_JPG/jfdctint.c $$DN_LIB_JPG/jidctflt.c $$DN_LIB_JPG/jidctfst.c \
	     $$DN_LIB_JPG/jidctint.c $$DN_LIB_JPG/jmemmgr.c \
	     $$DN_LIB_JPG/jquant1.c $$DN_LIB_JPG/jquant2.c $$DN_LIB_JPG/jutils.c \
	     $$DN_LIB_JPG/jmemansi.c
}

!stat_libjpeg {
  unix:LIBS += -ljpeg
  win32:LIBS += $$DN_LSRC/libjpeg.lib
}

#---------------------------------------------------------------------
#bzlib
   
stat_bzlib {
  SOURCES += $$DN_LIB_BZ2/blocksort.c $$DN_LIB_Z/bzip2.c $$DN_LIB_Z/bzlib.c randtable.c \
             $$DN_LIB_Z/compress.c $$DN_LIB_Z/crctable.c $$DN_LIB_Z/decompress.c huffman.c
} 

sys_bzlib {
   unix:LIBS += -lbz2
   macx:LIBS += -lbz2     
   #win32:LIBS += $$DN_LIBS_PLTFM/libbz2.lib     
}

