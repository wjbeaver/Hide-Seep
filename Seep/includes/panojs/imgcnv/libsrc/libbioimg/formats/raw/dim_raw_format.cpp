/*****************************************************************************
  RAW support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    12/01/2005 15:27 - First creation
    2007-07-12 21:01 - reading raw
        
  Ver : 2

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dim_raw_format.h"
#include "dim_raw_format_io.cpp"

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

//****************************************************************************
//
// INTERNAL STRUCTURES
//
//****************************************************************************

bool rawGetImageInfo( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return false;
  if (fmtHndl->internalParams == NULL) return false;
  TDimRawParams *rawPar = (TDimRawParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &rawPar->i;  

  *info = initTDimImageInfo();
  info->number_pages = 1;
  info->samples = 1;

  //fmtHndl->compression - offset
  //fmtHndl->quality - endian (0/1)  
  rawPar->header_offset = 0;
  rawPar->big_endian = false;

  if (fmtHndl->stream == NULL) return false;

  return true;
}

bool rawWriteImageInfo( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return false;
  if (fmtHndl->internalParams == NULL) return false;
  TDimRawParams *rawPar = (TDimRawParams *) fmtHndl->internalParams;
  
  TDimImageBitmap *img = fmtHndl->image;
  if (img == NULL) return false;

  std::string infname = fmtHndl->fileName;
  if (infname.size() <= 1) return false; 
  infname += ".info";

  std::string inftext = getImageInfoText(img);


  FILE *stream;
  if( (stream = fopen( infname.c_str(), "wb" )) != NULL ) {
    fwrite( inftext.c_str(), sizeof( char ), inftext.size(), stream );
    fclose( stream );
  }

  return true;
}

//****************************************************************************
//
// FORMAT DEMANDED FUNTIONS
//
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimRawValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length)
{
  return -1;
}

TDimFormatHandle dimRawAquireFormatProc( void )
{
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimRawReleaseFormatProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimRawCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------

static TDimRawParams dim_raw_params;

void dimRawCloseImageProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimClose ( fmtHndl );
  //dimFree ( &fmtHndl->internalParams );
}

DIM_UINT dimRawOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimRawCloseImageProc (fmtHndl);  
  //fmtHndl->internalParams = (void *) new TDimRawParams [1];
  fmtHndl->internalParams = (void *) &dim_raw_params;

  if ( io_mode == DIM_IO_READ ) {
    if ( isCustomReading ( fmtHndl ) != TRUE )
      fmtHndl->stream = /*(void *)*/ fopen( fmtHndl->fileName, "rb" );
    if (!fmtHndl->stream) { dimRawCloseImageProc (fmtHndl); return 1; };
    if ( !rawGetImageInfo( fmtHndl ) ) { dimRawCloseImageProc (fmtHndl); return 1; };
  }

  if ( io_mode == DIM_IO_WRITE ) {
    if ( isCustomWriting ( fmtHndl ) != TRUE )
      fmtHndl->stream = fopen( fmtHndl->fileName, "wb" );

    if (fmtHndl->stream == NULL) { dimRawCloseImageProc (fmtHndl); return 1; };
  }

  return 0;
}


DIM_UINT dimRawFOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, DIM_ImageIOModes io_mode)
{
  fmtHndl->fileName = fileName;
  return dimRawOpenImageProc(fmtHndl, io_mode);
}

DIM_UINT dimRawIOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, 
                                         DIM_IMAGE_CLASS *image, DIM_ImageIOModes io_mode)
{
  fmtHndl->fileName = fileName;
  fmtHndl->image    = image;
  return dimRawOpenImageProc(fmtHndl, io_mode);
}


//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimRawGetNumPagesProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;

  return 1;
}


TDimImageInfo dimRawGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num )
{
  TDimImageInfo ii = initTDimImageInfo();
  page_num;

  if (fmtHndl == NULL) return ii;
  TDimRawParams *rawPar = (TDimRawParams *) fmtHndl->internalParams;

  return rawPar->i;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimRawAddMetaDataProc (TDimFormatHandle *fmtHndl)
{
  fmtHndl=fmtHndl;
  return 1;
}


DIM_UINT dimRawReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type)
{
  fmtHndl; page; group; tag; type;
  return 1;
}

char* dimRawReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl )
{
  fmtHndl;
  return NULL;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimRawReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;

  fmtHndl->pageNumber = page;
  return read_raw_image( fmtHndl );
}

DIM_UINT dimRawWriteImageProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;

  //if (fmtHndl->pageNumber <= 1) rawWriteImageInfo( fmtHndl );

  return write_raw_image( fmtHndl );
}

//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

TDimFormatItem dimRawItems[1] = {
{
    "RAW",              // short name, no spaces
    "RAW image pixels", // Long format name
    "raw",              // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    0, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    1, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 0, 0, 0, 0, 0 } 
  }  
};

TDimFormatHeader dimRawHeader = {

  sizeof(TDimFormatHeader),
  "1.0.0",
  "DIMIN RAW CODEC",
  "RAW CODEC",
  
  2,                     // 0 or more, specify number of bytes needed to identify the file
  {1, 1, dimRawItems},   // dimJpegSupported,
  
  dimRawValidateFormatProc,
  // begin
  dimRawAquireFormatProc, //TDimAquireFormatProc
  // end
  dimRawReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimRawOpenImageProc, //TDimOpenImageProc
  dimRawCloseImageProc, //TDimCloseImageProc 

  // info
  dimRawGetNumPagesProc, //TDimGetNumPagesProc
  dimRawGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimRawReadImageProc, //TDimReadImageProc 
  dimRawWriteImageProc, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimRawReadMetaDataProc, //TDimReadMetaDataProc
  dimRawAddMetaDataProc,  //TDimAddMetaDataProc
  dimRawReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc

  NULL,
  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimRawGetFormatHeader(void)
{
  return &dimRawHeader;
}

} // extern C


