/*****************************************************************************
  PNG support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    07/29/2004 16:31 - First creation
    08/04/2004 22:25 - Update to FMT_IFS 1.2, support for io protorypes
        
  Ver : 2
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "dim_png_format.h"
#include "dim_png_format_io.cpp"

//****************************************************************************
// CALLBACKS
//****************************************************************************

static void dpng_read_fn( png_structp png_ptr, png_bytep data, png_size_t length )
{
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) png_get_io_ptr( png_ptr );

  DIM_ULONG nr = dimRead( fmtHndl, data, 1, length );
	if (nr <= length) {
    png_error( png_ptr, "Read Error" );
	  return;
	}
}

static void dpng_write_fn( png_structp png_ptr, png_bytep data, png_size_t length )
{
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) png_get_io_ptr( png_ptr );

  DIM_ULONG nr = dimWrite( fmtHndl, data, 1, length );
	if (nr < length) {
    png_error( png_ptr, "Write Error" );
	  return;
	}
}

static void dpng_flush_fn( png_structp png_ptr )
{
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) png_get_io_ptr( png_ptr );

  dimFlush( fmtHndl );
}

//****************************************************************************
//
// INTERNAL STRUCTURES
//
//****************************************************************************

bool pngGetImageInfo( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return FALSE;
  if (fmtHndl->internalParams == NULL) return FALSE;
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &pngPar->i;  

  *info = initTDimImageInfo();
  info->number_pages = 1;
  info->samples = 1;


  pngPar->png_ptr = png_create_read_struct ( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
  if (!pngPar->png_ptr) return FALSE;
  
  pngPar->info_ptr = png_create_info_struct( pngPar->png_ptr );
  if (!pngPar->info_ptr) {
    png_destroy_read_struct( &pngPar->png_ptr, (png_infopp)NULL, (png_infopp)NULL );
    return FALSE;
  }
  
  pngPar->end_info = png_create_info_struct( pngPar->png_ptr );
  if (!pngPar->end_info) {
    png_destroy_read_struct( &pngPar->png_ptr, &pngPar->info_ptr, (png_infopp)NULL );
    return FALSE;
  }

  if (setjmp( png_jmpbuf(pngPar->png_ptr) )) {
    png_destroy_read_struct( &pngPar->png_ptr, &pngPar->info_ptr, &pngPar->end_info );
    return FALSE;
  }

  if ( isCustomReading ( fmtHndl ) != TRUE )
    png_init_io( pngPar->png_ptr, (FILE *) fmtHndl->stream );
  else
  {
    png_set_read_fn( pngPar->png_ptr, (void*) fmtHndl, dpng_read_fn );
    png_read_info( pngPar->png_ptr, pngPar->info_ptr );
  } 

  // no gamma info


  //-----------------------------------------------------------------------
  // read image header
  //-----------------------------------------------------------------------
  png_uint_32 width;
  png_uint_32 height;
  int bit_depth;
  int color_type;

  png_read_info( pngPar->png_ptr, pngPar->info_ptr );
  png_get_IHDR( pngPar->png_ptr, pngPar->info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0 );

  info->width  = width;
  info->height = height;
  info->depth = bit_depth;
  info->samples = 1;
  info->imageMode = DIM_GRAYSCALE;

  if ( color_type == PNG_COLOR_TYPE_GRAY ) 
  {
    info->samples = 1;
    info->imageMode = DIM_GRAYSCALE;
  }

  if ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) 
  {
    info->samples = 2;
    info->imageMode = DIM_GRAYSCALE;
  }

  if ( color_type == PNG_COLOR_TYPE_PALETTE ) 
  {
    info->samples = 1;
    info->imageMode = DIM_INDEXED;
  }

  if ( color_type == PNG_COLOR_TYPE_RGB ) 
  {
    info->samples = 3;
    info->imageMode = DIM_RGB;
  }
 
  if ( color_type == PNG_COLOR_TYPE_RGB_ALPHA ) 
  {
    info->samples = 4;
    info->imageMode = DIM_RGBA;
  }
 
  //-------------------------------------------------
  // init palette
  //-------------------------------------------------
  DIM_UINT i;

  if ( ( color_type == PNG_COLOR_TYPE_GRAY ) ||
       ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) ||
       ( color_type == PNG_COLOR_TYPE_PALETTE ) ) 
  {
    info->lut.count = 256;
    for (i=0; i<256; i++) info->lut.rgba[i] = dimRGB( i, i, i );
  }

  
  //-------------------------------------------------
  // read palette
  //-------------------------------------------------
  if ( color_type == PNG_COLOR_TYPE_PALETTE ) 
  {
    int num_colors = pngPar->info_ptr->num_palette;
  
    if ( num_colors > 0 ) // LUT is present
    {
      info->lut.count = num_colors;
      
      if ( png_get_valid( pngPar->png_ptr, pngPar->info_ptr, PNG_INFO_tRNS ) ) 
      { // RGBA palette
        for ( i=0; i<(DIM_UINT)num_colors; i++ ) 
          info->lut.rgba[i] = dimRGBA( pngPar->info_ptr->palette[i].red, 
                                       pngPar->info_ptr->palette[i].green, 
                                       pngPar->info_ptr->palette[i].blue,
                                       #if PNG_LIBPNG_VER >= 10400
                                       pngPar->info_ptr->trans_alpha[i] );
                                       #else
                                       pngPar->info_ptr->trans[i] ); 
                                       #endif
      }
      else
      { // RGB palette
        for ( i=0; i<(DIM_UINT)num_colors; i++ ) 
          info->lut.rgba[i] = dimRGB( pngPar->info_ptr->palette[i].red, 
                                      pngPar->info_ptr->palette[i].green, 
                                      pngPar->info_ptr->palette[i].blue );
      }


    } // if num_col > 0
  
  } // if paletted

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

DIM_INT dimPngValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length)
{
  if (length < 8) return -1;
  if (memcmp(magic, png_magic, 8) == 0) return 0;
  return -1;
}

TDimFormatHandle dimPngAquireFormatProc( void )
{
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimPngReleaseFormatProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimPngCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimPngCloseImageProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;

  if (fmtHndl->internalParams == NULL) return;
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;

  if ( fmtHndl->io_mode == DIM_IO_READ )
    png_destroy_read_struct( &pngPar->png_ptr, &pngPar->info_ptr, &pngPar->end_info );
  else
    png_destroy_write_struct( &pngPar->png_ptr, &pngPar->info_ptr );

  dimFree ( &fmtHndl->internalParams );
  dimClose ( fmtHndl );
}

DIM_UINT dimPngOpenImageProc  ( TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimPngCloseImageProc (fmtHndl);  
  fmtHndl->internalParams = (void *) new TDimPngParams [1];
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;
  fmtHndl->io_mode = io_mode;

  if ( io_mode == DIM_IO_READ )
  {
    if ( isCustomReading ( fmtHndl ) != TRUE )
      fmtHndl->stream = fopen( fmtHndl->fileName, "rb" );
    if (fmtHndl->stream == NULL) { dimPngCloseImageProc (fmtHndl); return 1; };
    if ( !pngGetImageInfo( fmtHndl ) ) { dimPngCloseImageProc (fmtHndl); return 1; };
  }

  if ( io_mode == DIM_IO_WRITE )
  {
    if ( isCustomWriting ( fmtHndl ) != TRUE )
      fmtHndl->stream = fopen( fmtHndl->fileName, "wb" );
    if (fmtHndl->stream == NULL) { dimPngCloseImageProc (fmtHndl); return 1; };

    pngPar->png_ptr = png_create_write_struct ( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    if (!pngPar->png_ptr) return 1;

    pngPar->info_ptr = png_create_info_struct( pngPar->png_ptr );
    if (!pngPar->info_ptr) {
       png_destroy_write_struct( &pngPar->png_ptr, (png_infopp) NULL );
       return 1;
    }

    if ( isCustomWriting ( fmtHndl ) != TRUE )
      png_init_io( pngPar->png_ptr, (FILE *) fmtHndl->stream );
    else
    {
      png_set_write_fn( pngPar->png_ptr, (void*) fmtHndl, dpng_write_fn, dpng_flush_fn);
      //png_read_info( pngPar->png_ptr, pngPar->info_ptr );
    }
  }

  return 0;
}


DIM_UINT dimPngFOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, DIM_ImageIOModes io_mode)
{
  fmtHndl->fileName = fileName;
  return dimPngOpenImageProc(fmtHndl, io_mode);
}

DIM_UINT dimPngIOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, 
                                         DIM_IMAGE_CLASS *image, DIM_ImageIOModes io_mode)
{
  fmtHndl->fileName = fileName;
  fmtHndl->image    = image;
  return dimPngOpenImageProc(fmtHndl, io_mode);
}


//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimPngGetNumPagesProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;

  return 1;
}


TDimImageInfo dimPngGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num )
{
  TDimImageInfo ii = initTDimImageInfo();

  if (fmtHndl == NULL) return ii;
  TDimPngParams *pngPar = (TDimPngParams *) fmtHndl->internalParams;

  return pngPar->i;
  page_num;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimPngAddMetaDataProc (TDimFormatHandle *fmtHndl)
{
  fmtHndl=fmtHndl;
  return 1;
}


DIM_UINT dimPngReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type)
{
  return read_png_metadata ( fmtHndl, group, tag, type );
  page;
}

char* dimPngReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl )
{
  return png_read_meta_as_text( fmtHndl );
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimPngReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;

  fmtHndl->pageNumber = page;
  return read_png_image( fmtHndl );
}

DIM_UINT dimPngWriteImageProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;
  return write_png_image( fmtHndl );
}

//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

TDimFormatItem dimPngItems[1] = {
{
    "PNG",            // short name, no spaces
    "Portable Network Graphics", // Long format name
    "png",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    1, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 4, 1, 16, 0 } 
  }
};

TDimFormatHeader dimPngHeader = {

  sizeof(TDimFormatHeader),
  "1.0.0",
  "DIMIN PNG CODEC",
  "PNG CODEC",
  
  8,                     // 0 or more, specify number of bytes needed to identify the file
  { 1, 1, dimPngItems }, // ( ver, sub formats ) only one sub format
  
  dimPngValidateFormatProc,
  // begin
  dimPngAquireFormatProc, //TDimAquireFormatProc
  // end
  dimPngReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimPngOpenImageProc, //TDimOpenImageProc
  dimPngCloseImageProc, //TDimCloseImageProc 

  // info
  dimPngGetNumPagesProc, //TDimGetNumPagesProc
  dimPngGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimPngReadImageProc, //TDimReadImageProc 
  dimPngWriteImageProc, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimPngReadMetaDataProc, //TDimReadMetaDataProc
  dimPngAddMetaDataProc,  //TDimAddMetaDataProc
  dimPngReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc

  NULL,
  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimPngGetFormatHeader(void)
{
  return &dimPngHeader;
}

} // extern C


