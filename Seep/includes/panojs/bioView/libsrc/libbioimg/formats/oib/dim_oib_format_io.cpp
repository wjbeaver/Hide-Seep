/*****************************************************************************
  Olympus Image Binary (OIB) format support
  Copyright (c) 2008, Center for Bio Image Informatics, UCSB
  
  Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    2008-06-04 14:26:14 - First creation
    2008-09-15 19:04:47 - Fix for older files with unordered streams    
    2008-11-06 13:36:43 - Parse preferred channel mapping

  Ver : 4
*****************************************************************************/

#include <cstring>

#include <string>

#include <tag_map.h>
#include "dim_oib_format.h"

#ifndef XMD_H
#define XMD_H // Shut JPEGlib up.
#endif  
  
#include <tiffio.h>

//****************************************************************************
// CALLBACKS
//****************************************************************************

// libtiff 3.8.2
#if (TIFFLIB_VERSION <= 20060323)
#define D_TIFF_SIZE_TYPE tsize_t
#define D_TIFF_DATA_TYPE tdata_t
#define D_TIFF_OFFS_TYPE toff_t
#endif

// libtiff 3.9.2
#if (TIFFLIB_VERSION == 20091104)
#define D_TIFF_SIZE_TYPE tsize_t
#define D_TIFF_DATA_TYPE tdata_t
#define D_TIFF_OFFS_TYPE toff_t
#endif

// libtiff 4.0.0 beta5
#if (TIFFLIB_VERSION >= 20100101)
#define D_TIFF_SIZE_TYPE tmsize_t
#define D_TIFF_DATA_TYPE void*
#define D_TIFF_OFFS_TYPE uint64
#endif

static D_TIFF_SIZE_TYPE stream_tiff_read(thandle_t handle, D_TIFF_DATA_TYPE data, D_TIFF_SIZE_TYPE size) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  POLE::Stream *stream = (POLE::Stream *) fmtHndl->stream;
  return (D_TIFF_SIZE_TYPE) stream->read( (unsigned char *) data, size );
}

static D_TIFF_SIZE_TYPE stream_tiff_write(thandle_t /*handle*/, D_TIFF_DATA_TYPE /*data*/, D_TIFF_SIZE_TYPE /*size*/) {
  return 0;
}

static D_TIFF_OFFS_TYPE stream_tiff_seek(thandle_t handle, D_TIFF_OFFS_TYPE offset, int whence) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  POLE::Stream *stream = (POLE::Stream *) fmtHndl->stream;
  D_TIFF_SIZE_TYPE off = offset;
  if (whence == SEEK_CUR) off = stream->tell() + offset;
  if (whence == SEEK_END) off = stream->size() + offset - 1;
  stream->seek( off );
  return stream->tell();
}

static int stream_tiff_close(thandle_t handle) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  POLE::Stream *stream = (POLE::Stream *) fmtHndl->stream;
  return 0;
}

static D_TIFF_OFFS_TYPE stream_tiff_size(thandle_t handle) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  POLE::Stream *stream = (POLE::Stream *) fmtHndl->stream;
  return stream->size();
}

static int stream_tiff_mmap(thandle_t /*handle*/, D_TIFF_DATA_TYPE* /*data*/, D_TIFF_OFFS_TYPE* /*size*/) {
  return 1;
}

static void stream_tiff_unmap(thandle_t /*handle*/, D_TIFF_DATA_TYPE /*data*/, D_TIFF_OFFS_TYPE /*size*/) {
}

//----------------------------------------------------------------------------
// READ PROC
//----------------------------------------------------------------------------

// find stream name independent of the OIB version
std::string get_stream_name( DOibParams *par, int page, int sample ) {
  
  std::string position_name = get_channel_stream_name( par, page, sample );

  //v 1.0.0.0 - Stream00007=Storage00001/probe_C001.tif (use filename_)
  std::string file_name = par->oifFolderName + "/" + par->oifFileName + "_" + position_name; 
  xstring stream_name = par->oib_info_hash.get_key( file_name );

  //v 2.0.0.0 - Stream00006=Storage00001/s_C001Z001.tif (use s_)
  if (stream_name.size()<=0) {
    file_name = par->oifFolderName + "/s_" + position_name; 
    stream_name = par->oib_info_hash.get_key( file_name );
  }

  //v X.X.X.X slow gessing if we did not find the stream, hope that _C001Z001 construct will be found
  if (stream_name.size()<=0) {
    stream_name = par->oib_info_hash.get_key_where_value_endsWith( position_name );
  }

  //strip initial "OibSaveInfo/" from the key
  std::string info_header("OibSaveInfo/");
  stream_name.erase( 0, info_header.size() );
  stream_name = "/" + par->oifFolderName + "/" + stream_name;
  return stream_name;
}

static int read_oib_image(TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DOibParams *par = (DOibParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->i; 
  if (par->storage == NULL) return 1;
  if (fmtHndl->pageNumber >= par->i.number_pages) fmtHndl->pageNumber = par->i.number_pages-1;

  TDimImageBitmap *img = fmtHndl->image;
  if ( allocImg( fmtHndl, &par->i, img) != 0 ) return 1;

  TIFFSetWarningHandler(0);
  TIFFSetErrorHandler(0);
  for (unsigned int sample=0; sample<img->i.samples; ++sample) {
    std::string stream_name = get_stream_name( par, fmtHndl->pageNumber, sample );
    POLE::Stream *stream = new POLE::Stream( par->storage, stream_name );
    if (!stream || stream->size() <= 0) return 1;
    if( stream->fail() ) return 1;
    fmtHndl->stream = stream;

    TIFF *tiff = TIFFClientOpen( fmtHndl->fileName, "rm", (thandle_t) fmtHndl, 
      stream_tiff_read, stream_tiff_write, stream_tiff_seek, stream_tiff_close, stream_tiff_size, stream_tiff_mmap, stream_tiff_unmap );

    if (!tiff) return 1;

    // since i'm not sure if metadata for depth is correct, we'll read actual tiff settings first
    uint16 bitspersample = 1;
    uint32 height = 0; 
    uint32 width = 0; 
    uint16 samplesperpixel = 1;

    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
    TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bitspersample);
    
    if (par->i.depth != bitspersample) return 1;
    if (par->i.width != width) return 1;
    if (par->i.height != height) return 1;
    if (samplesperpixel != 1) return 1;

    DIM_UCHAR *p = (DIM_UCHAR *) img->bits[sample];
    DIM_UINT lineSize = getLineSizeInBytes( img );

    for(unsigned int y=0; y<img->i.height; y++) {
      dimProgress( fmtHndl, y*(sample+1), img->i.height*img->i.samples, "Reading OIB" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  
      TIFFReadScanline(tiff, p, y, 0);
      p += lineSize;
    } // for y

    fmtHndl->stream = NULL;
    delete stream;
    TIFFClose( tiff );
  }  // for sample

  return 0;
}


//----------------------------------------------------------------------------
// META DATA PROC
//----------------------------------------------------------------------------

DIM_UINT read_oib_metadata (TDimFormatHandle *fmtHndl, int /*group*/, int /*tag*/, int /*type*/) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  return 0;
}

DIM_UINT oib_append_metadata (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;
  DOibParams *par = (DOibParams *) fmtHndl->internalParams;
  if (!par->storage) return 1;

  hash->set_value( "pixel_resolution_x", par->pixel_resolution[0] );
  hash->set_value( "pixel_resolution_y", par->pixel_resolution[1] );
  hash->set_value( "pixel_resolution_z", par->pixel_resolution[2] );
  hash->set_value( "pixel_resolution_t", par->pixel_resolution[3] );

  if (par->pixel_resolution[0]>0) hash->set_value( "pixel_resolution_unit_x", "microns" );
  if (par->pixel_resolution[1]>0) hash->set_value( "pixel_resolution_unit_y", "microns" );
  if (par->pixel_resolution[2]>0) hash->set_value( "pixel_resolution_unit_z", "microns" );
  if (par->pixel_resolution[3]>0) hash->set_value( "pixel_resolution_unit_t", "seconds" );

  //date
  std::string date = par->oif_metadata_hash.get_value("Acquisition Parameters Common/ImageCaputreDate", "" );
  if (date.size()>0) {
    if ( *date.begin() == '\'' && *(date.end()-1) == '\'' ) date = date.substr( 1, date.size()-2);
    hash->set_value( "date_time", date );
  }

  // channel names
  xstring tag;
  for (int i=0; i<par->channel_names.size(); ++i) {
    tag.sprintf("channel_%d_name", i);
    hash->set_value( tag.c_str(), par->channel_names[i] );
  }

  // preferred lut mapping
  if (par->display_lut.size() >= 3) {
    hash->set_value( "display_channel_red",   par->display_lut[0] );
    hash->set_value( "display_channel_green", par->display_lut[1] );
    hash->set_value( "display_channel_blue",  par->display_lut[2] );
  }

  // include all other tags from the hash into custom tag location
  std::map< std::string, std::string >::const_iterator it = par->oif_metadata_hash.begin();
  while (it != par->oif_metadata_hash.end() ) {
    xstring key = it->first;
    if ( !key.startsWith("Sequential Group") && 
         !key.startsWith("ProfileSaveInfo") )
      hash->set_value( xstring("custom/")+it->first, it->second );
    ++it;
  }

  return 0;
}





