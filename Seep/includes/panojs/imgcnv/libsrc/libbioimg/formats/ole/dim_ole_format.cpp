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

#include <cmath>
#include <cstring>

#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <xstring.h>
#include <bim_metatags.h>

#include "dim_ole_format.h"
#include "dim_oib_format_io.cpp"
#include "dim_zvi_format_io.cpp"

//****************************************************************************
// FORMAT REQUIRED FUNCTIONS
//****************************************************************************

DIM_INT oleValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length) {
  if (length < BIM_OLE_MAGIC_SIZE) return -1;
  if (memcmp( magic, ole::magic, BIM_OLE_MAGIC_SIZE ) != 0) return -1;
  // this is a very fast way of siply testing if the file is an OLE directory
  // better testing will happen in the read
  return 0;
}

TDimFormatHandle oleAquireFormatProc( void ) {
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void oleCloseImageProc ( TDimFormatHandle *fmtHndl);

void oleReleaseFormatProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  oleCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void oleCloseImageProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams != NULL)
    delete (ole::Params *) fmtHndl->internalParams;
  fmtHndl->internalParams = NULL;
}

DIM_UINT oleOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) oleCloseImageProc (fmtHndl);  
  fmtHndl->internalParams = (void *) new ole::Params();
  ole::Params *par = (ole::Params *) fmtHndl->internalParams;

  if (io_mode == DIM_IO_READ) {
    par->ole_format = ole::FORMAT_UNKNOWN;

    // try to load OLE storage
    par->storage = new POLE::Storage( fmtHndl->fileName );
    par->storage->open();
    if( par->storage->result() != POLE::Storage::Ok ) return 1;

    // test for incoming OLE format
    if (zvi::Directory::isValid(par->storage))
      if (zviGetImageInfo( fmtHndl )==0) {
        par->ole_format = ole::FORMAT_ZVI;
        fmtHndl->subFormat = par->ole_format-1;
      }
    
    if (par->ole_format == ole::FORMAT_UNKNOWN) 
      if (oibGetImageInfo( fmtHndl )==0) {
        par->ole_format = ole::FORMAT_OIB;
        fmtHndl->subFormat = par->ole_format-1;
      }

    if (par->ole_format == ole::FORMAT_UNKNOWN) 
      oleCloseImageProc(fmtHndl);

    return (par->ole_format == ole::FORMAT_UNKNOWN);
  }
  
  return 1;
}

//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT oleGetNumPagesProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;
  ole::Params *par = (ole::Params *) fmtHndl->internalParams;
  return par->i.number_pages;
}


TDimImageInfo oleGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT /*page_num*/ ) {
  if (!fmtHndl) return initTDimImageInfo();
  ole::Params *par = (ole::Params *) fmtHndl->internalParams;
  return par->i;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT oleReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT /*page*/, int group, int tag, int type) {
  if (!fmtHndl) return 1;
  if (!fmtHndl->internalParams) return 1;
  return 0;
}

char* oleReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl ) {
  return NULL;
}

DIM_UINT oleAddMetaDataProc (TDimFormatHandle * /*fmtHndl*/) {
  return 1;
}

DIM_UINT oleAppendMetadata (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (!fmtHndl) return 1;
  if (!fmtHndl->internalParams) return 1;
  ole::Params *par = (ole::Params *) fmtHndl->internalParams;

  if (par->ole_format == ole::FORMAT_ZVI) return zvi_append_metadata (fmtHndl, hash );
  else
  if (par->ole_format == ole::FORMAT_OIB) return oib_append_metadata (fmtHndl, hash );
  return 1;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT oleReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page ) {
  if (!fmtHndl) return 1;
  if (!fmtHndl->internalParams) return 1;
  ole::Params *par = (ole::Params *) fmtHndl->internalParams;
  fmtHndl->pageNumber = page;

  if (par->ole_format == ole::FORMAT_ZVI) return zvi_read_image( fmtHndl );
  else
  if (par->ole_format == ole::FORMAT_OIB) return read_oib_image( fmtHndl );
  return 1;
}


//****************************************************************************
// EXPORTED FUNCTION
//****************************************************************************

#define BIM_OLE_NUM_FORMTAS 2

TDimFormatItem oleFormatItems[BIM_OLE_NUM_FORMTAS] = {
  {
    "OIB",            // short name, no spaces
    "Olympus Image Binary", // Long format name
    "oib",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 1, 1, 16, 16, 1 } 
  },
  {
    "ZVI",            // short name, no spaces
    "Zeiss ZVI", // Long format name
    "zvi",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 1, 1, 16, 16, 1 } 
  }
};

TDimFormatHeader bimOibHeader = {

  sizeof(TDimFormatHeader),
  "2.0.0",
  "OLE CODEC for OIB and ZVI",
  "OLE CODEC for OIB and ZVI",
  
  BIM_OLE_MAGIC_SIZE,         // 0 or more, specify number of bytes needed to identify the file
  {1, BIM_OLE_NUM_FORMTAS, oleFormatItems},   //dimJpegSupported,
  
  oleValidateFormatProc,
  // begin
  oleAquireFormatProc, //TDimAquireFormatProc
  // end
  oleReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  oleOpenImageProc, //TDimOpenImageProc
  oleCloseImageProc, //TDimCloseImageProc 

  // info
  oleGetNumPagesProc, //TDimGetNumPagesProc
  oleGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  oleReadImageProc, //TDimReadImageProc 
  NULL, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  oleReadMetaDataProc, //TDimReadMetaDataProc
  oleAddMetaDataProc,  //TDimAddMetaDataProc
  oleReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc
  oleAppendMetadata, //TDimAppendMetaDataProc

  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* oleGetFormatHeader(void) {
  return &bimOibHeader;
}

} // extern C





