/*****************************************************************************
  Olympus Image Binary (OIB) format support
  Copyright (c) 2008, Center for Bio Image Informatics, UCSB
  
  Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    2008-06-04 14:26:14 - First creation
    2008-09-15 19:04:47 - Fix for older files with unordered streams
    2008-11-06 13:36:43 - Parse preferred channel mapping
            
  Ver : 3
*****************************************************************************/

#ifndef D_OIB_FORMAT_H
#define D_OIB_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <cstdio>
#include <vector>
#include <string>
#include <map>

#include <tag_map.h>

#include <pole.h>

//class DTagMap;
class DOibParams;

// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimOibGetFormatHeader(void);
}

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------
void             dimOibCloseImageProc     ( TDimFormatHandle *fmtHndl);

DIM_UINT oib_append_metadata (TDimFormatHandle *fmtHndl, DTagMap *hash );
std::string get_channel_stream_name( DOibParams *par, int page, int sample );

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------

#define DIM_OIB_MAGIC_SIZE 8

const unsigned char oibMagic[DIM_OIB_MAGIC_SIZE]  = { 0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1 };

class DOibAxis {
public:
  DOibAxis(): MaxSize(0), StartPosition(0.0), EndPosition(0.0) { }
  
  int MaxSize;

  double StartPosition;
  double EndPosition;

  std::string AxisCode;
  std::string AxisName;
  std::string PixUnit;
  std::string UnitName;

  //CalibrateValueA=100.0
  //CalibrateValueB=0.0
  //ClipPosition=0
};

class DOibParams {
public:
  DOibParams(): storage(NULL), num_z(0), num_t(0), num_l(0) { i = initTDimImageInfo(); }
  ~DOibParams() { if (storage) delete storage; }
  
  TDimImageInfo i;
  POLE::Storage *storage;

  int num_z, num_t, num_l;

  std::string oif_metadata;
  std::string oifFolderName;
  std::string oifFileName;

  std::vector< double > pixel_resolution;
  std::vector<std::string> channel_names;
  std::vector<int> display_lut; // rgb -> chan numbers

  DTagMap oib_info_hash;
  DTagMap oif_metadata_hash;

  std::vector< DOibAxis > axis;
  //std::vector< std::string > streams_for_pages;  
};
 

#endif // D_OIB_FORMAT_H
