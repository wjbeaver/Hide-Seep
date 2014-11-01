/*****************************************************************************
  Igor binary file format v5 (IBW)
  UCSB/BioITR property
  Copyright (c) 2005 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    10/19/2005 16:03 - First creation
            
  Ver : 1
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

// windows: use secure C libraries with VS2005 or higher
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
  //#define HAVE_SECURE_C
  //#pragma message(">>>>> IBW: using secure c libraries")
#endif 

#include "dim_ibw_format.h"
#include "dim_ibw_format_io.cpp"

//****************************************************************************
// INTERNAL STRUCTURES
//****************************************************************************

void swapBinHeader5(BinHeader5 *bh)
{
  dimSwapShort( (DIM_UINT16*) &bh->checksum );
  dimSwapLong ( (DIM_UINT32*) &bh->wfmSize );  

  dimSwapLong ( (DIM_UINT32*) &bh->formulaSize ); 
  dimSwapLong ( (DIM_UINT32*) &bh->noteSize ); 
  dimSwapLong ( (DIM_UINT32*) &bh->dataEUnitsSize ); 
  dimSwapLong ( (DIM_UINT32*) &bh->sIndicesSize ); 

  dimSwapLong ( (DIM_UINT32*) &bh->wfmSize ); 
  dimSwapLong ( (DIM_UINT32*) &bh->wfmSize ); 

  for (int i=0; i<MAXDIMS; ++i)
  {
    dimSwapLong ( (DIM_UINT32*) &bh->dimEUnitsSize[i] ); 
    dimSwapLong ( (DIM_UINT32*) &bh->dimLabelsSize[i] ); 
  }
}

void swapWaveHeader5(WaveHeader5 *wh)
{
  dimSwapLong ( (DIM_UINT32*) &wh->creationDate );   
  dimSwapLong ( (DIM_UINT32*) &wh->modDate );   
  dimSwapLong ( (DIM_UINT32*) &wh->npnts );  
  dimSwapShort( (DIM_UINT16*) &wh->type );
  dimSwapShort( (DIM_UINT16*) &wh->fsValid );
  dimSwapDouble( (DIM_DOUBLE*) &wh->topFullScale );
  dimSwapDouble( (DIM_DOUBLE*) &wh->botFullScale );

  for (int i=0; i<MAXDIMS; ++i)
  {
    dimSwapLong ( (DIM_UINT32*) &wh->nDim[i] ); 
    dimSwapDouble( (DIM_DOUBLE*) &wh->sfA[i] );
    dimSwapDouble( (DIM_DOUBLE*) &wh->sfB[i] );
  }
}

void ibwReadNote ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
  DIbwParams *par = (DIbwParams *) fmtHndl->internalParams;
  
  long buf_size = par->bh.noteSize;
  par->note.resize(buf_size+1);
  char *buf = &par->note[0];
  buf[buf_size] = '\0';

  if (dimSeek(fmtHndl, par->notes_offset, SEEK_SET) != 0) return;
  if (dimRead( fmtHndl, buf, buf_size, 1 ) != 1) return;

  for (int i=0; i<buf_size; ++i)
    if (buf[i] == 0x0d) buf[i] = 0x0a;
}

void ibwGetImageInfo( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;
  DIbwParams *ibwPar = (DIbwParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &ibwPar->i; 
  *info = initTDimImageInfo();

  if (fmtHndl->stream == NULL) return;
  if (dimSeek(fmtHndl, 0, SEEK_SET) != 0) return;
  if ( dimRead( fmtHndl, &ibwPar->bh, 1, sizeof(BinHeader5) ) != sizeof(BinHeader5)) return;
  
  // test if little-endian
  if (memcmp( &ibwPar->bh.version, ibwMagicWin, DIM_IBW_MAGIC_SIZE ) == 0) 
    ibwPar->little_endian = true;
  else
    ibwPar->little_endian = false;

  // swap structure elements if running on Big endian machine...
  if ( (dimBigendian) && (ibwPar->little_endian == true) ) 
  {
    swapBinHeader5( &ibwPar->bh );
  }
 
  if ( dimRead( fmtHndl, &ibwPar->wh, 1, sizeof(WaveHeader5) ) != sizeof(WaveHeader5)) return;

  // swap structure elements if running on Big endian machine...
  if ( (dimBigendian) && (ibwPar->little_endian == true) ) 
  {
    swapWaveHeader5( &ibwPar->wh );
  }

/*
  info->resUnits = DIM_RES_mim;
  info->xRes = nimg.xR / nimg.width;
  info->yRes = nimg.yR / nimg.height;
  */

  // get correct type size
  switch ( ibwPar->wh.type )
  {
    case NT_CMPLX:
      ibwPar->real_bytespp = 8;
      ibwPar->real_type  = DIM_TAG_SRATIONAL;
      break;
    case NT_FP32:
      ibwPar->real_bytespp = 4;
      ibwPar->real_type  = DIM_TAG_FLOAT;
      break;
    case NT_FP64:
      ibwPar->real_bytespp = 8;
      ibwPar->real_type  = DIM_TAG_DOUBLE;
      break;
    case NT_I8:
      ibwPar->real_bytespp = 1;
      ibwPar->real_type  = DIM_TAG_BYTE;
      break;
    case NT_I16:
      ibwPar->real_bytespp = 2;
      ibwPar->real_type  = DIM_TAG_SHORT;
      break;
    case NT_I32:
      ibwPar->real_bytespp = 4;
      ibwPar->real_type  = DIM_TAG_LONG;
      break;
    default:
      ibwPar->real_bytespp = 1;
      ibwPar->real_type  = DIM_TAG_BYTE;
  }

  // prepare needed vars
  ibwPar->data_offset    = sizeof(BinHeader5) + sizeof(WaveHeader5) - 4; //the last pointer is actually initial data
  ibwPar->data_size      = ibwPar->wh.npnts * ibwPar->real_bytespp;
  ibwPar->formula_offset = ibwPar->data_offset + ibwPar->data_size; 
  ibwPar->notes_offset   = ibwPar->formula_offset + ibwPar->bh.formulaSize;

  // set image parameters
  info->width   = ibwPar->wh.nDim[0];
  info->height  = ibwPar->wh.nDim[1];
  info->samples = 1;
  info->number_pages = ibwPar->wh.nDim[2];
  info->imageMode = DIM_GRAYSCALE;
  //info->number_z = info->number_pages;
  
  // by now we'll normalize all data
  info->depth     = 8;
  info->pixelType = D_FMT_UNSIGNED;


  if (info->samples == 3) info->imageMode = DIM_RGB;
  if (info->samples == 4) info->imageMode = DIM_RGBA;

  //-------------------------------------------------
  // init palette
  //-------------------------------------------------
  info->lut.count = 0;
  for (int i=0; i<256; i++) info->lut.rgba[i] = dimRGB(i, i, i);

  ibwReadNote ( fmtHndl );
}

//****************************************************************************
// FORMAT DEMANDED FUNTIONS
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimIbwValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length)
{
  if (length < DIM_IBW_MAGIC_SIZE) return -1;
  if (memcmp( magic, ibwMagicWin, DIM_IBW_MAGIC_SIZE ) == 0) return 0;
  if (memcmp( magic, ibwMagicMac, DIM_IBW_MAGIC_SIZE ) == 0) return 0;
  return -1;
}

TDimFormatHandle dimIbwAquireFormatProc( void )
{
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimIbwReleaseFormatProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimIbwCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimIbwCloseImageProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  dimClose ( fmtHndl );
  if (fmtHndl->internalParams != NULL) 
  {
    DIbwParams *ibwPar = (DIbwParams *) fmtHndl->internalParams;
    delete ibwPar;
  }
  fmtHndl->internalParams = NULL;
}

DIM_UINT dimIbwOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimIbwCloseImageProc (fmtHndl);  
  fmtHndl->internalParams = (void *) new DIbwParams();

  if (io_mode == DIM_IO_READ) {
    if ( isCustomReading ( fmtHndl ) != TRUE )
      fmtHndl->stream = fopen( fmtHndl->fileName, "rb" );

    if (fmtHndl->stream == NULL) return 1;
    ibwGetImageInfo( fmtHndl );
  }
  else return 1;

  return 0;
}

//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimIbwGetNumPagesProc ( TDimFormatHandle *fmtHndl )
{
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;
  DIbwParams *ibwPar = (DIbwParams *) fmtHndl->internalParams;

  return ibwPar->i.number_pages;
}


TDimImageInfo dimIbwGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num )
{
  TDimImageInfo ii = initTDimImageInfo();

  if (fmtHndl == NULL) return ii;
  DIbwParams *ibwPar = (DIbwParams *) fmtHndl->internalParams;

  return ibwPar->i;
  page_num;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimIbwReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT , int group, int tag, int type) {
  return 0;
}

char* dimIbwReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl ) {
  return NULL;
}

DIM_UINT dimIbwAddMetaDataProc (TDimFormatHandle *) {
  return 1;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimIbwReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->stream == NULL) return 1;
  fmtHndl->pageNumber = page;
  return read_ibw_image( fmtHndl );
}

DIM_UINT dimIbwWriteImageProc ( TDimFormatHandle * ) {
  return 1;
}



//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

TDimFormatItem dimIbwItems[1] = {
  {
    "IBW",            // short name, no spaces
    "Igor binary file format v5", // Long format name
    "ibw",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 1, 0, 0, 0, 1 } 
  }
};

TDimFormatHeader dimIbwHeader = {

  sizeof(TDimFormatHeader),
  "1.0.0",
  "IBW CODEC",
  "IBW CODEC",
  
  12,                      // 0 or more, specify number of bytes needed to identify the file
  {1, 1, dimIbwItems},   //dimJpegSupported,
  
  dimIbwValidateFormatProc,
  // begin
  dimIbwAquireFormatProc, //TDimAquireFormatProc
  // end
  dimIbwReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimIbwOpenImageProc, //TDimOpenImageProc
  dimIbwCloseImageProc, //TDimCloseImageProc 

  // info
  dimIbwGetNumPagesProc, //TDimGetNumPagesProc
  dimIbwGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimIbwReadImageProc, //TDimReadImageProc 
  NULL, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimIbwReadMetaDataProc, //TDimReadMetaDataProc
  dimIbwAddMetaDataProc,  //TDimAddMetaDataProc
  dimIbwReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc
  ibw_append_metadata, //TDimAppendMetaDataProc

  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimIbwGetFormatHeader(void)
{
  return &dimIbwHeader;
}

} // extern C





