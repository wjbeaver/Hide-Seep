/*****************************************************************************
  FFMPEG support 
  Copyright (c) 2008 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    2008-02-01 14:45 - First creation
        
  Ver : 1
*****************************************************************************/

#include <string>
#include <algorithm>
#include "dim_ffmpeg_format.h"
#include "tag_map.h"

//----------------------------------------------------------------------------
// READ PROC
//----------------------------------------------------------------------------

static int read_ffmpeg_image(TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->i;  
  TDimImageBitmap *img = fmtHndl->image;

  // init the image
  if ( allocImg( fmtHndl, info, img) != 0 ) return 1;
  unsigned int page_size = getImgSizeInBytes(img);

  bool res = false;
  try {
    res = par->ff_in.seek(fmtHndl->pageNumber);
  } catch (...) {
    return 1;
  }

  if ( res ) {
    const std::vector<unsigned char> *frame = par->ff_in.currBGR();
    const unsigned char *fbuf = &(*frame)[0];

    if (frame->size() >= page_size)
    for ( int c=0; c<info->samples; ++c ) {
      const unsigned char *pI = fbuf + (info->samples-c-1);
      unsigned char *pO = (unsigned char *) img->bits[c];

      for ( int y=0; y<info->height; ++y ) {

        dimProgress( fmtHndl, y, info->height, "Reading Video" );
        if ( dimTestAbort( fmtHndl ) == 1) break;  

        for ( unsigned long x=0; x<info->width; ++x ) {
          *pO = *pI;
          ++pO;
          pI+=info->samples;
        } // for x
      } // for y
    }// c
  } // if could seek to the page

  return 0;
}

static int write_ffmpeg_image(TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->i;  
  TDimImageBitmap *img = fmtHndl->image;
  *info = img->i;

  if (!par->frame_sizes_set) {

    std::string filename = fmtHndl->fileName;
    par->ff_out.setWidth( info->width );
    par->ff_out.setHeight( info->height );
    par->ff_out.setFormat( formats_write[ fmtHndl->subFormat ] );

    // use our default codec
    par->ff_out.setCodec( encoder_ids[ fmtHndl->subFormat ] );

    // for some formats set fourcc to non default one, e.g. XVID instead of default FMP4 for better compatibility of AVI
    par->ff_out.setFourCC( formats_fourcc[ fmtHndl->subFormat ] );

    if (par->fps > 0) par->ff_out.setFramesPerSecond(par->fps);
    if (par->bitrate > 0) par->ff_out.setBitRate(par->bitrate);

    VideoIO::KeyValueMap kvm;
    kvm["filename"] = filename;
    kvm["depth"]    = "3";
    
    /*
    assnString(kvm["bitRate"],          getBitRate());
    assnString(kvm["bitRateTolerance"], getBitRateTolerance());
    assnString(kvm["gopSize"],          getGopSize());
    assnString(kvm["maxBFrames"],       getMaxBFrames());
    */
    par->ff_out.setup( kvm );    
    //par->ff_out.open( filename );
    if (!par->ff_out.isOpen()) return 1;
    par->frame_sizes_set = true;
  }

  unsigned char *fbuf = par->ff_out.rawFrameData();
  int channels = std::min<int>( info->samples, 3 );
  for ( int c=0; c<channels; ++c ) {
    unsigned char *pI = fbuf + c;
    unsigned char *pO = (unsigned char *) img->bits[c];

    for ( int y=0; y<info->height; ++y ) {

      dimProgress( fmtHndl, y, info->height, "Writing video" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  

      for ( unsigned long x=0; x<info->width; ++x ) {
        *pI = *pO;
        ++pO;
        pI+=3;
      } // for x
    } // for y
  }// c

  try {
    par->ff_out.addFromRawFrame( info->width, info->height, 3 );
  } catch (...) {
    return 1;
  }

  return 0;
}


