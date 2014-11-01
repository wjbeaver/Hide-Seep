######################################################################
# Manually generated !!!
# BioViewRemote v 1.0.0 Project file
# run: qmake wvremote.pro in order to generate Makefile for your platform
# Copyright (c) 2005-06, Bio-Image Informatic Center, UCSB
#
# To generate Makefile on any platform:
#   qmake wvremote.pro
#
# To generate VisualStudio project file:
#   qmake -t vcapp -spec win32-msvc2005 wvremote.pro
#   qmake -t vcapp -spec win32-msvc.net wvremote.pro
#   qmake -t vcapp -spec win32-msvc wvremote.pro
#   qmake -spec win32-icc wvremote.pro # to use pure Intel Compiler
#
# To generate xcode project file:
#   qmake -spec macx-xcode wvremote.pro 
#  
# To generate Makefile on MacOSX with binary install:
#   qmake -spec macx-g++ wvremote.pro
#
######################################################################

#---------------------------------------------------------------------
# configuration: editable
#---------------------------------------------------------------------
VERSION = 0.1.1

CONFIG += release
CONFIG += warn_off

# mac universal binaries, qt MUST to be configured with "universal",
# otherwise comment out two following lines
#macx:QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
#macx:CONFIG+=x86 ppc
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4

win32:CONFIG += embed_manifest_exe # VC2005 manifests

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

DN_UI       = $$DN_SRC/ui
DN_VIEW     = $$DN_SRC/viewer
DN_NETW     = $$DN_SRC/network
DN_WIDG     = $$DN_SRC/widgets

DN_LIB_BIO = $$DN_LSRC/libbioimg
DN_FMTS     = $$DN_LIB_BIO/formats
DN_FMTS_API = $$DN_LIB_BIO/formats_api
DN_CORE     = $$DN_LIB_BIO/core_lib

#---------------------------------------------------------------------
# configuration: automatic
#---------------------------------------------------------------------


#---------------------------------------------------------------------
# generation: fixed
#---------------------------------------------------------------------

TEMPLATE = app

CONFIG += qt
CONFIG += thread 

QT += xml network

macx:RC_FILE = $$DN_ICNS/wvremote.icns


MOC_DIR = $$DN_GENS
DESTDIR = $$DN_BIN
OBJECTS_DIR = $$DN_OBJ
UI_DIR = $$DN_GENS
DN_LSRC = ../libsrc

INCLUDEPATH += $$DN_GENS
DEPENDPATH += $$DN_GENS

INCLUDEPATH += $$DN_VIEW
INCLUDEPATH += $$DN_NETW
INCLUDEPATH += $$DN_WIDG

INCLUDEPATH += $$DN_LIB_BIO
INCLUDEPATH += $$DN_FMTS
INCLUDEPATH += $$DN_FMTS_API
INCLUDEPATH += $$DN_CORE

# Main headers are needed to invoke moc where needed
HEADERS += $$DN_VIEW/remoteWidget.h\
           $$DN_VIEW/subAreaLabel.h

# Main app
SOURCES += $$DN_SRC/main_remote.cpp\
           $$DN_VIEW/remoteWidget.cpp\
           $$DN_VIEW/subAreaLabel.cpp
           
#############################################                      
# Widgets
HEADERS += $$DN_WIDG/notifyWidget.h\
           $$DN_WIDG/progresscircle.h\
           $$DN_WIDG/scalebar.h

# Main app
SOURCES += $$DN_WIDG/notifyWidget.cpp\
           $$DN_WIDG/appconfig.cpp\
           $$DN_WIDG/progresscircle.cpp\
           $$DN_WIDG/scalebar.cpp                  
           
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

FORMS = $$DN_UI/remote.ui

           
	              