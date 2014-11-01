#!/bin/bash

echo ----------------------------------------------------------------------
echo Building bioView
echo ----------------------------------------------------------------------
qmake wv.pro
make

echo ----------------------------------------------------------------------
echo Creating distributable binaries for bioView
echo ----------------------------------------------------------------------
echo

echo Updating binary
sh macx_update_bin_wv

echo Updating libs
sh macx_update_lib_wv

echo Updating plugs
sh macx_update_plugs

echo ----------------------------------------------------------------------
echo Building bioView Remote
echo ----------------------------------------------------------------------
qmake wvremote.pro
make

echo ----------------------------------------------------------------------
echo Creating distributable binaries for bioView Remote
echo ----------------------------------------------------------------------
echo

echo Updating binary
sh macx_update_bin_wvremote

echo Updating libs
sh macx_update_lib_wvremote


