/*****************************************************************************
  JPEG support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    04/22/2004 13:06 - First creation
        
  Ver : 1
*****************************************************************************/

#ifndef DIM_JPEG_FORMAT_H
#define DIM_JPEG_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimJpegGetFormatHeader(void);
}

//----------------------------------------------------------------------------
// Internal Format Info Structs
//----------------------------------------------------------------------------

// INTERNAL FUNCTIONS
DIM_INT dimJpegValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length);

//TDimFormatHandle dimJpegAquireFormatProc( void );
//void dimJpegReleaseFormatProc (TDimFormatHandle *fmtHndl);

DIM_UINT dimJpegOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode);
DIM_UINT dimJpegFOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, DIM_ImageIOModes io_mode);
DIM_UINT dimJpegIOpenImageProc (TDimFormatHandle *fmtHndl, char* fileName, 
                                         DIM_IMAGE_CLASS *image, DIM_ImageIOModes io_mode);

void dimJpegCloseImageProc     (TDimFormatHandle *fmtHndl);

//DIM_UINT dimJpegGetNumPagesProc       ( TDimFormatHandle *fmtHndl );
//TDimImageInfo dimJpegGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num );

DIM_UINT dimJpegReadImageProc  (TDimFormatHandle *fmtHndl, DIM_UINT page);
DIM_UINT dimJpegWriteImageProc (TDimFormatHandle *fmtHndl);

//DIM_UINT dimJpegReadImagePreviewProc (TDimFormatHandle *fmtHndl, DIM_UINT w, DIM_UINT h);

DIM_UINT dimJpegAddMetaDataProc (TDimFormatHandle *fmtHndl);
DIM_UINT dimJpegReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type);
char* dimJpegReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl );

#endif // DIM_JPEG_FORMAT_H
