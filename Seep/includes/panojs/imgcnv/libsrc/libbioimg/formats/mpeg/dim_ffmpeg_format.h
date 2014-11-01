/*****************************************************************************
  FFMPEG support 
  Copyright (c) 2008 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    2008-02-01 14:45 - First creation
        
  Ver : 1
*****************************************************************************/

#ifndef DIM_FFMPEG_FORMAT_H
#define DIM_FFMPEG_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <cstdio>
#include <vector>
#include <map>

#include "FfmpegIVideo.h"
#include "FfmpegOVideo.h"

class DTagMap;

static std::vector< std::string > formats_write;
static std::vector< std::string > formats_fourcc;
static std::vector< CodecID > encoder_ids;
static std::map< std::string, int > formats;
static std::map< std::string, int > codecs;

// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimFFMpegGetFormatHeader(void);
}

void dimFFMpegCloseImageProc (TDimFormatHandle *fmtHndl);

DIM_UINT ffmpeg_append_metadata (TDimFormatHandle *fmtHndl, DTagMap *hash );

//----------------------------------------------------------------------------
// Internal Format Info Struct
//----------------------------------------------------------------------------

class DFFMpegParams {
public:
  TDimImageInfo i;
  VideoIO::FfmpegIVideo ff_in;
  VideoIO::FfmpegOVideo ff_out;
  bool frame_sizes_set;
  float fps;
  int bitrate;
};



#endif // DIM_FFMPEG_FORMAT_H
