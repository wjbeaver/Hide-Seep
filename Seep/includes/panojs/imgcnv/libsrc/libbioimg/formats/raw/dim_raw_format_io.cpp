/*****************************************************************************
  RAW IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    12/01/2005 15:27 - First creation
    2007-07-12 21:01 - reading raw
        
  Ver : 2
*****************************************************************************/

#include <string>

#include "dim_raw_format.h"

//****************************************************************************
// READ PROC
//****************************************************************************

static int read_raw_image(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  TDimRawParams *rawPar = (TDimRawParams *) fmtHndl->internalParams;
  //TDimImageInfo *info = &rawPar->i;  
  TDimImageBitmap *img = fmtHndl->image;
  
  //-------------------------------------------------
  // init the image
  //-------------------------------------------------
  //if ( allocImg( fmtHndl, info, img) != 0 ) return 1;

  //-------------------------------------------------
  // read image data
  //-------------------------------------------------
  unsigned int plane_size  = getImgSizeInBytes( img ); 
  unsigned int cur_plane   = fmtHndl->pageNumber;
  unsigned int header_size = rawPar->header_offset; 

  // seek past header size plus number of planes
  if ( dimSeek( fmtHndl, header_size + plane_size*cur_plane*img->i.samples, SEEK_SET ) != 0) return 1;
  
  for (unsigned int sample=0; sample<img->i.samples; ++sample) {
    if (dimRead( fmtHndl, img->bits[sample], 1, plane_size ) != plane_size) return 1;

    // now it has to know whether to swap the data and so on...
    if ( dimBigendian != (int)rawPar->big_endian) {
      if (img->i.depth == 16)
        dimSwapArrayOfShort((DIM_UINT16*) img->bits[sample], plane_size/2);

      if (img->i.depth == 32)
        dimSwapArrayOfLong((DIM_UINT32*) img->bits[sample], plane_size/4);
    }
  }


  return 0;
}

//****************************************************************************
// WRITE PROC
//****************************************************************************

static int write_raw_image(TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  TDimRawParams *rawPar = (TDimRawParams *) fmtHndl->internalParams;
  TDimImageBitmap *img = fmtHndl->image;

  //-------------------------------------------------
  // write image data
  //-------------------------------------------------
  unsigned long plane_size = getImgSizeInBytes( img );
  
  for (unsigned int sample=0; sample<img->i.samples; ++sample)
    if (dimWrite( fmtHndl, img->bits[sample], 1, plane_size ) != plane_size) return 1;

  dimFlush( fmtHndl );
  return 0;
}











