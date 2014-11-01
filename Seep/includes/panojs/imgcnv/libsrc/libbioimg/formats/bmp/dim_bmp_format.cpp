/*****************************************************************************
  BMP support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    04/22/2004 13:06 - First creation
    05/10/2004 14:55 - Big endian support
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
        
  Ver : 3
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bim_metatags.h>

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

#include "dim_bmp_format.h"
#include "dim_bmp_format_io.cpp"

//****************************************************************************
//
// INTERNAL STRUCTURES
//
//****************************************************************************

bool bmpGetImageInfo( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return FALSE;
  if (fmtHndl->internalParams == NULL) return FALSE;
  TDimBmpParams *bmpPar = (TDimBmpParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &bmpPar->i;  

  *info = initTDimImageInfo();
  info->number_pages = 1;
  info->samples = 1;


  if (fmtHndl->stream == NULL) return FALSE;
  if (dimSeek(fmtHndl, 0, SEEK_SET) != 0) return FALSE;
  if ( dimRead( fmtHndl, &bmpPar->bf, 1, sizeof(TDimBITMAPFILEHEADER) ) != 
       sizeof(TDimBITMAPFILEHEADER)) return FALSE;
  
  // test if is BMP
  unsigned char *mag_num = (unsigned char *) &bmpPar->bf.bfType;
  if ( (mag_num[0] != 0x42) || (mag_num[1] != 0x4d) ) return FALSE;

  // swap structure elements if running on Big endian machine...
  if (dimBigendian) {
    dimSwapLong( (DIM_UINT32*) &bmpPar->bf.bfOffBits );
    dimSwapLong( (DIM_UINT32*) &bmpPar->bf.bfSize );
  }
 
  if ( dimRead( fmtHndl, &bmpPar->bi, 1, sizeof(TDimBITMAPINFOHEADER) ) != 
       sizeof(TDimBITMAPINFOHEADER)) return FALSE;

  // swap structure elements if running on Big endian machine...
  if (dimBigendian) {
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biSize );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biWidth );  
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biHeight );  
    dimSwapShort( (DIM_UINT16*) &bmpPar->bi.biPlanes );
    dimSwapShort( (DIM_UINT16*) &bmpPar->bi.biBitCount );
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biCompression );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biSizeImage );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biXPelsPerMeter );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biYPelsPerMeter );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biClrUsed );    
    dimSwapLong( (DIM_UINT32*) &bmpPar->bi.biClrImportant );    
  }

  // test for image parameters
  if ( !(bmpPar->bi.biBitCount == 1  || bmpPar->bi.biBitCount == 4  || bmpPar->bi.biBitCount == 8   || 
         bmpPar->bi.biBitCount == 16 || bmpPar->bi.biBitCount == 24 || bmpPar->bi.biBitCount == 32) ||
	       bmpPar->bi.biPlanes != 1 || bmpPar->bi.biCompression > DIM_BMP_BITFIELDS )
	return FALSE;	// weird BMP image

  if ( !(bmpPar->bi.biCompression == DIM_BMP_RGB || 
        (bmpPar->bi.biBitCount == 4 && bmpPar->bi.biCompression == DIM_BMP_RLE4) ||
	      (bmpPar->bi.biBitCount == 8 && bmpPar->bi.biCompression == DIM_BMP_RLE8) || 
        ((bmpPar->bi.biBitCount == 16 || bmpPar->bi.biBitCount == 32) && bmpPar->bi.biCompression == DIM_BMP_BITFIELDS)) )
	return FALSE;	// weird compression type

  info->width  = bmpPar->bi.biWidth;
  info->height = bmpPar->bi.biHeight;
  info->depth = 8;
  info->samples = 1;
  info->imageMode = DIM_GRAYSCALE;

  if (bmpPar->bi.biBitCount < 8) info->depth = bmpPar->bi.biBitCount;
  if (bmpPar->bi.biBitCount > 8) { info->samples = 3; info->imageMode = DIM_RGB; }
  if (bmpPar->bi.biBitCount == 32) { info->samples = 4; info->imageMode = DIM_RGBA; }



  //-------------------------------------------------
  // init palette
  //-------------------------------------------------
  int num_colors = 0;
  if ( bmpPar->bi.biBitCount <= 8 )
    num_colors = bmpPar->bi.biClrUsed ? bmpPar->bi.biClrUsed : 1 << bmpPar->bi.biBitCount;  
  if (num_colors > 0) info->imageMode = DIM_INDEXED;

  info->lut.count = 0;
  for (unsigned int i=0; i<256; i++) info->lut.rgba[i] = dimRGB(i, i, i);

  
  //-------------------------------------------------
  // read palette
  //-------------------------------------------------
  if (dimSeek( fmtHndl, BMP_FILEHDR_SIZE + bmpPar->bi.biSize, SEEK_SET ) != 0) return FALSE;

  if ( num_colors > 0 ) { // LUT is present
    DIM_UINT8 rgb[4];
    info->lut.count = num_colors;
    int   rgb_len = bmpPar->bi.biSize == DIM_BMP_OLD ? 3 : 4;
    for ( unsigned int i=0; i<(DIM_UINT)num_colors; i++ ) {
      if ( dimRead( fmtHndl, (char *)rgb, 1, rgb_len ) != (unsigned int) rgb_len ) return FALSE;
      info->lut.rgba[i] = dimRGB( rgb[2], rgb[1], rgb[0] );
    }
  } 
   
  return TRUE;
}

//****************************************************************************
//
// FORMAT DEMANDED FUNTIONS
//
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimBmpValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length)
{
  if (length < 2) return -1;
  unsigned char *mag_num = (unsigned char *) magic;
  if ( (mag_num[0] == 0x42) && (mag_num[1] == 0x4d) ) return 0;
  return -1;
}

TDimFormatHandle dimBmpAquireFormatProc( void )
{
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimBmpReleaseFormatProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimBmpCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimBmpCloseImageProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimClose ( fmtHndl );
  dimFree ( &fmtHndl->internalParams );
}

DIM_UINT dimBmpOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimBmpCloseImageProc (fmtHndl);  
  fmtHndl->internalParams = (void *) new TDimBmpParams [1];
  //TDimBmpParams *bmpPar = (TDimBmpParams *) fmtHndl->internalParams;

  if ( io_mode == DIM_IO_READ )
  {
    if ( isCustomReading ( fmtHndl ) != TRUE )
      fmtHndl->stream = /*(void *)*/ fopen( fmtHndl->fileName, "rb" );
    
    if (fmtHndl->stream == NULL) { dimBmpCloseImageProc (fmtHndl); return 1; };
    if ( !bmpGetImageInfo( fmtHndl ) ) { dimBmpCloseImageProc (fmtHndl); return 1; };
  }

  if ( io_mode == DIM_IO_WRITE )
  {
    if ( isCustomWriting ( fmtHndl ) != TRUE )
      fmtHndl->stream = /*(void *)*/ fopen( fmtHndl->fileName, "wb" );
    if (fmtHndl->stream == NULL) { dimBmpCloseImageProc (fmtHndl); return 1; };
  }

  return 0;
}


DIM_UINT dimBmpFOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, DIM_ImageIOModes io_mode)
{
  fmtHndl->fileName = fileName;
  return dimBmpOpenImageProc(fmtHndl, io_mode);
}

DIM_UINT dimBmpIOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, 
                                         DIM_IMAGE_CLASS *image, DIM_ImageIOModes io_mode)
{
  fmtHndl->fileName = fileName;
  fmtHndl->image    = image;
  return dimBmpOpenImageProc(fmtHndl, io_mode);
}


//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimBmpGetNumPagesProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;

  return 1;
}


TDimImageInfo dimBmpGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num )
{
  TDimImageInfo ii = initTDimImageInfo();
  page_num;

  if (fmtHndl == NULL) return ii;
  TDimBmpParams *bmpPar = (TDimBmpParams *) fmtHndl->internalParams;

  return bmpPar->i;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimBmpAddMetaDataProc (TDimFormatHandle *)
{
  return 1;
}


DIM_UINT dimBmpReadMetaDataProc (TDimFormatHandle *, DIM_UINT , int , int , int )
{
  return 1;
}

char* dimBmpReadMetaDataAsTextProc ( TDimFormatHandle * )
{
  return NULL;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimBmpReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;

  fmtHndl->pageNumber = page;
  return read_bmp_image( fmtHndl );
}

DIM_UINT dimBmpWriteImageProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;
  return write_bmp_image( fmtHndl );
}

//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

TDimFormatItem dimBmpItems[1] = {
{
    "BMP",            // short name, no spaces
    "Windows Bitmap", // Long format name
    "bmp",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    0, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 4, 1, 8, 0 } 
  }  
};

TDimFormatHeader dimBmpHeader = {

  sizeof(TDimFormatHeader),
  "1.0.0",
  "DIMIN BMP CODEC",
  "BMP CODEC",
  
  2,                     // 0 or more, specify number of bytes needed to identify the file
  {1, 1, dimBmpItems},   // dimJpegSupported,
  
  dimBmpValidateFormatProc,
  // begin
  dimBmpAquireFormatProc, //TDimAquireFormatProc
  // end
  dimBmpReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimBmpOpenImageProc, //TDimOpenImageProc
  dimBmpCloseImageProc, //TDimCloseImageProc 

  // info
  dimBmpGetNumPagesProc, //TDimGetNumPagesProc
  dimBmpGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimBmpReadImageProc, //TDimReadImageProc 
  dimBmpWriteImageProc, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimBmpReadMetaDataProc, //TDimReadMetaDataProc
  dimBmpAddMetaDataProc,  //TDimAddMetaDataProc
  dimBmpReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc

  NULL,
  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimBmpGetFormatHeader(void)
{
  return &dimBmpHeader;
}

} // extern C


