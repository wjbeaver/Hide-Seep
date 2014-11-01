/*****************************************************************************
  JPEG support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    04/22/2004 13:06 - First creation
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
        
  Ver : 2
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <xstring.h>

#include "dim_jpeg_format.h"


// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

//#include <stdio.h> // jpeglib needs this to be pre-included
#include <setjmp.h>

#ifdef FAR
#undef FAR
#endif

#if defined(__RPCNDR_H__)
#define HAVE_BOOLEAN
#define boolean unsigned int
#endif

extern "C" {
#define XMD_H // Shut JPEGlib up.

#include <jpeglib.h>
#include <jerror.h>

#ifdef const
#undef const // Remove crazy C hackery in jconfig.h
#endif
}

#ifndef uchar
#define uchar unsigned char
#endif

#include "dim_jpeg_format_io.cpp"


//****************************************************************************
//
// FORMAT DEMANDED FUNTIONS
//
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimJpegValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length) {
  if (length < 3) return -1;
  if (memcmp(magic,"\377\330\377",3) == 0) return 0;
  return -1;
}

TDimFormatHandle dimJpegAquireFormatProc( void ) {
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimJpegReleaseFormatProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  dimJpegCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------

void dimJpegSetWriteParameters  (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  if (!fmtHndl->options) return;
  xstring str = fmtHndl->options;
  std::vector<xstring> options = str.split( " " );
  if (options.size() < 1) return;
  
  int i = -1;
  while (i<(int)options.size()-1) {
    i++;

    if ( options[i]=="quality" && options.size()-i>0 ) {
      i++;
      fmtHndl->quality = options[i].toInt( 100 );
      continue;
    }
  } // while
}

void dimJpegCloseImageProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
}

DIM_UINT dimJpegOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode) {
  if (fmtHndl == NULL) return 1;
  if ( io_mode == DIM_IO_WRITE ) dimJpegSetWriteParameters(fmtHndl);
  return 0;
}


DIM_UINT dimJpegFOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, DIM_ImageIOModes io_mode) {
  fmtHndl->fileName = fileName;
  return dimJpegOpenImageProc(fmtHndl, io_mode);
}

DIM_UINT dimJpegIOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, 
                                         DIM_IMAGE_CLASS *image, DIM_ImageIOModes io_mode) {
  fmtHndl->fileName = fileName;
  fmtHndl->image    = image;
  return dimJpegOpenImageProc(fmtHndl, io_mode);
}


//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimJpegGetNumPagesProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 0;
  return 1;
}


TDimImageInfo dimJpegGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num ) {
  TDimImageInfo ii = initTDimImageInfo();
  if (!fmtHndl) return ii;
  fmtHndl->pageNumber = page_num;
  fmtHndl->subFormat = 0;
  return ii;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimJpegAddMetaDataProc (TDimFormatHandle * /*fmtHndl*/) {
  return 1;
}


DIM_UINT dimJpegReadMetaDataProc (TDimFormatHandle * /*fmtHndl*/, DIM_UINT /*page*/, int /*group*/, int /*tag*/, int /*type*/) {
  return 1;
}

char* dimJpegReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return NULL;
  return NULL;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimJpegReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page ) {
  if (fmtHndl == NULL) return 1;

  if ( !isCustomReading ( fmtHndl ) )
    fmtHndl->stream = fopen( fmtHndl->fileName, "rb" );

  if (!fmtHndl->stream) return 1;
  fmtHndl->pageNumber = page;
  DIM_UINT res = read_jpeg_image( fmtHndl );
  dimClose( fmtHndl );
  return res;
}

DIM_UINT dimJpegWriteImageProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 1;
  
  if ( !isCustomWriting ( fmtHndl ) )  
    fmtHndl->stream = fopen( fmtHndl->fileName, "wb" );
  
  if (!fmtHndl->stream) return 1;
  DIM_UINT res = write_jpeg_image( fmtHndl );
  dimFlush( fmtHndl );
  dimClose( fmtHndl );
  return res;
}

/*
// at the moment w and h make no effect,the image retreived is the same size as original
DIM_UINT dimJpegReadImagePreviewProc (TDimFormatHandle *fmtHndl, DIM_UINT w, DIM_UINT h)
{
  TDimImageBitmap bmp8, *bmp;
  initImagePlanes( &bmp8 );

  w=w; h=h;
  
  if ( dimTiffReadImageProc( fmtHndl, 0 ) != 0) return 1;
  bmp = fmtHndl->image;
  
  if (bmp->i.samples > 3) bmp->i.samples = 3;

  if (bmp->i.depth == 16) {
    allocImg(&bmp8, bmp->i.width, bmp->i.height, bmp->i.samples, 8);
    normalizeImg(bmp, &bmp8);
    deleteImg( bmp );

    bmp->bits[0] = bmp8.bits[0]; 
    bmp->bits[1] = bmp8.bits[1];    
    bmp->bits[2] = bmp8.bits[2]; 
  }

  bmp->i.depth = 8;
  
  if (bmp->i.samples == 1) {
    bmp->bits[1] = bmp->bits[0];    
    bmp->bits[2] = bmp->bits[0]; 
  }

  if (bmp->i.samples == 2) {
    bmp->bits[2] = bmp->bits[0]; 
  }
  bmp->i.samples = 3;

  return 0;
}

*/

//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

TDimFormatItem dimJpegItems[1] = {
  {
    "JPEG",            // short name, no spaces
    "JPEG File Interchange Format", // Long format name
    "jpg|jpeg|jpe|jif|jfif",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 4, 8, 8, 1 } 
  }
};

TDimFormatHeader dimJpegHeader = {

  sizeof(TDimFormatHeader),
  "1.0.0",
  "DIMIN JPEG CODEC",
  "JPEG-JFIF Compliant CODEC",
  
  3,                      // 0 or more, specify number of bytes needed to identify the file
  {1, 1, dimJpegItems},   // dimJpegSupported,
  
  dimJpegValidateFormatProc,
  // begin
  dimJpegAquireFormatProc, //TDimAquireFormatProc
  // end
  dimJpegReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimJpegOpenImageProc, //TDimOpenImageProc
  dimJpegCloseImageProc, //TDimCloseImageProc 

  // info
  dimJpegGetNumPagesProc, //TDimGetNumPagesProc
  dimJpegGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimJpegReadImageProc, //TDimReadImageProc 
  dimJpegWriteImageProc, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimJpegReadMetaDataProc, //TDimReadMetaDataProc
  dimJpegAddMetaDataProc,  //TDimAddMetaDataProc
  dimJpegReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc

  NULL,
  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimJpegGetFormatHeader(void)
{
  return &dimJpegHeader;
}

} // extern C


