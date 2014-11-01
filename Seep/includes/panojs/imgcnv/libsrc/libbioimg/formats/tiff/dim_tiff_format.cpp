/*****************************************************************************
  TIFF support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    03/29/2004 22:23 - First creation
    01/23/2007 20:42 - fixes in warning reporting
        
  Ver : 4
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dim_xtiffio.h"
#include "dim_tiff_format.h"

#include <xstring.h>
#include <tag_map.h>
#include <bim_metatags.h>

void getCurrentPageInfo(DTiffParams *tiffParams);
void getImageInfo(DTiffParams *tiffParams);
TDimImageInfo initTDimImageInfo();
void* DimMalloc(DIM_ULONG size);
void* DimFree(void *p);

std::string read_tag_as_string(TIFF *tif, TDimTiffIFD *ifd, DIM_UINT tag);

#include "dim_tiff_format_io.cpp"

//****************************************************************************
// inits
//****************************************************************************

DTiffParams::DTiffParams() {
  this->info = initTDimImageInfo();
  this->dimTiff = NULL;
  this->subType = tstGeneric;
  this->ifds = initTDimTiffIFDs();
}

//****************************************************************************
// STATIC FUNCTIONS THAT MIGHT BE PROVIDED BY HOST AND CALLING STUBS FOR THEM
//****************************************************************************

static TDimProgressProc  hostProgressProc  = NULL;
static TDimErrorProc     hostErrorProc     = NULL;
static TDimTestAbortProc hostTestAbortProc = NULL;
static TDimMallocProc    hostMallocProc    = NULL;
static TDimFreeProc      hostFreeProc      = NULL;

static void localTiffProgressProc(long done, long total, char *descr) {
  if (hostProgressProc != NULL) hostProgressProc( done, total, descr );
}

static void localTiffErrorProc(int val, char *descr) {
  if (hostErrorProc != NULL) hostErrorProc( val, descr );
}

static bool localTiffTestAbortProc ( void ) {
  if (hostTestAbortProc != NULL) {
    if (hostTestAbortProc() == 1) return true; else return false;
  } else
    return FALSE;
}

static void* localTiffMallocProc (DIM_ULONG size) {
  if (hostMallocProc != NULL) 
    return hostMallocProc( size );
  else {
    //void *p = (void *) new char[size];
    void *p = (void *) _TIFFmalloc(size);  
    return p;
  }
}

static void* localTiffFreeProc (void *p) {
  if (hostFreeProc != NULL) 
    return hostFreeProc( p );
  else {
    //unsigned char *pu = (unsigned char*) p;
    //if (p != NULL) delete pu;
	  
	  if (p != NULL) _TIFFfree( p );
    return NULL;
  }
}

static void setLocalTiffFunctions ( TDimFormatHandle *fmtHndl ) {
  hostProgressProc  = fmtHndl->showProgressProc; 
  hostErrorProc     = fmtHndl->showErrorProc; 
  hostTestAbortProc = fmtHndl->testAbortProc; 
  hostMallocProc    = fmtHndl->mallocProc; 
  hostFreeProc      = fmtHndl->freeProc; 
}

static void resetLocalTiffFunctions ( ) {
  hostProgressProc  = NULL; 
  hostErrorProc     = NULL; 
  hostTestAbortProc = NULL; 
  hostMallocProc    = NULL; 
  hostFreeProc      = NULL; 
}

void* DimMalloc(DIM_ULONG size) {
  return localTiffMallocProc ( size );
}

void* DimFree(void *p) {
  return localTiffFreeProc (p);
}


//****************************************************************************
// CALLBACKS
//****************************************************************************

static D_TIFF_SIZE_TYPE tiff_read(thandle_t handle, D_TIFF_DATA_TYPE data, D_TIFF_SIZE_TYPE size) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  return (D_TIFF_SIZE_TYPE) dimRead( fmtHndl, data, 1, size );
}

static D_TIFF_SIZE_TYPE tiff_write(thandle_t handle, D_TIFF_DATA_TYPE data, D_TIFF_SIZE_TYPE size) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  if ( fmtHndl->io_mode != DIM_IO_WRITE ) return 0;
  return (D_TIFF_SIZE_TYPE) dimWrite( fmtHndl, data, 1, size );
}

static D_TIFF_OFFS_TYPE tiff_seek(thandle_t handle, D_TIFF_OFFS_TYPE offset, int whence) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  if ( dimSeek( fmtHndl, offset, whence ) == 0 )
    return (D_TIFF_OFFS_TYPE) dimTell(fmtHndl);
  else
    return 0;
}

static int tiff_close(thandle_t handle) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  dimFlush( fmtHndl );
  return dimClose( fmtHndl );
}

static D_TIFF_OFFS_TYPE tiff_size(thandle_t handle) {
  TDimFormatHandle *fmtHndl = (TDimFormatHandle *) handle;
  return dimSize( fmtHndl );
}

static int tiff_mmap(thandle_t /*handle*/, D_TIFF_DATA_TYPE* /*data*/, D_TIFF_OFFS_TYPE* /*size*/) {
  return 1;
}

static void tiff_unmap(thandle_t /*handle*/, D_TIFF_DATA_TYPE /*data*/, D_TIFF_OFFS_TYPE /*size*/) {

}

//****************************************************************************
//
// FORMAT DEMANDED FUNTIONS
//
//****************************************************************************

//----------------------------------------------------------------------------
// UTILITARY FUNCTIONS
//----------------------------------------------------------------------------

unsigned int tiffGetNumberOfPages( DTiffParams *tiffpar ) {
  unsigned int i=0;
  TIFF *tif = tiffpar->dimTiff;
  if (tif == NULL) return i;
  
  // if STK then get number of pages in special way
  if (tiffpar->subType == tstStk) {
    return stkGetNumPlanes( tif );
  }

  TIFFSetDirectory(tif, 0);
  while (TIFFLastDirectory(tif) == 0) {
    if ( TIFFReadDirectory(tif) == 0) break;
    i++;
  }
  i++;

  //if (tiffpar->subType == tstPsia) i =* 2;

  return i;
}

void tiffReadResolution( TIFF *tif, DIM_UINT &units, double &xRes, double &yRes) {
  if (tif == NULL) return;
  
  float xresolution = 0;
  float yresolution = 0;
  short resolutionunit = 0;

  units = 0; xRes = 0; yRes = 0;

  if ( ( TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT , &resolutionunit) ) &&
       ( TIFFGetField(tif, TIFFTAG_XRESOLUTION , &xresolution) ) &&
       ( TIFFGetField(tif, TIFFTAG_YRESOLUTION , &yresolution) ) )
  {
    units = resolutionunit;
    xRes = xresolution;
    yRes = yresolution;
  }

  // here we need to read specific info here to define resolution correctly


}

DIM_UINT getTiffMode( TIFF *tif)
{
  if (tif == NULL) return DIM_GRAYSCALE;

  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  uint16 samplesperpixel = 1;
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);

  if (photometric == PHOTOMETRIC_RGB) return DIM_RGB;
  if (photometric == PHOTOMETRIC_PALETTE) return DIM_INDEXED;

  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
  if (samplesperpixel > 1) return DIM_MULTI;
    
  return DIM_GRAYSCALE;
}

void getCurrentPageInfo(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return;
  TIFF *tif = tiffParams->dimTiff;
  TDimImageInfo *info = &tiffParams->info;
  if (!tif) return;

  uint32 height = 0; 
  uint32 width = 0; 
  uint16 bitspersample = 1;
  uint16 samplesperpixel = 1;
  uint16 sampleformat = 1;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  uint16 compression = COMPRESSION_NONE;
  uint16 planarconfig;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
  TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleformat);

  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
  TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarconfig);

  if (photometric==PHOTOMETRIC_YCBCR && planarconfig==PLANARCONFIG_CONTIG && 
      compression==COMPRESSION_JPEG) {
    TIFFSetField(tif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
    bitspersample = 8;
    samplesperpixel = 3;
  }

  if (photometric==PHOTOMETRIC_LOGLUV && planarconfig==PLANARCONFIG_CONTIG && 
      (compression==COMPRESSION_SGILOG ||compression==COMPRESSION_SGILOG24 )) {
    TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_8BIT);
    bitspersample = 8;
    samplesperpixel = 3;
  }

  if (photometric==PHOTOMETRIC_LOGL && compression==COMPRESSION_SGILOG) {
    TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_8BIT);
    bitspersample = 8;
  }

  info->width = width;
  info->height = height;

  info->samples = samplesperpixel;
  info->depth = bitspersample;
  info->pixelType = D_FMT_UNSIGNED;

  if (sampleformat == SAMPLEFORMAT_INT)
    info->pixelType = D_FMT_SIGNED;
  else
  if (sampleformat == SAMPLEFORMAT_IEEEFP)
    info->pixelType = D_FMT_FLOAT;


  if( !TIFFIsTiled(tif) ) {
    info->tileWidth = 0;
    info->tileHeight = 0; 
  } else {
    uint32 columns, rows;
    TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &columns);
    TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);
    info->tileWidth = columns;
    info->tileHeight = rows; 
  }
    
  info->transparentIndex = 0;
  info->transparencyMatting = 0;

  info->imageMode = getTiffMode( tif );
  tiffReadResolution( tif, info->resUnits, info->xRes, info->yRes);

  if (tiffParams->subType!=tstFluoview && tiffParams->subType!=tstPsia && tiffParams->subType!=tstAndor)
    init_image_palette( tif, info );


  if ( tiffParams->subType == tstFluoview || tiffParams->subType == tstAndor )
    fluoviewGetCurrentPageInfo(tiffParams);
  else
  if ( tiffParams->subType == tstPsia )
    psiaGetCurrentPageInfo(tiffParams);
  else
  if ( tiffParams->subType == tstCzLsm ) 
    lsmGetCurrentPageInfo(tiffParams);
  else
  if ( tiffParams->subType == tstOmeTiff ) 
    omeTiffGetCurrentPageInfo(tiffParams);
}

void getImageInfo(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return;
  TIFF *tif = tiffParams->dimTiff;
  TDimImageInfo *info = &tiffParams->info;
  if (!tif) return;

  info->ver = sizeof(TDimImageInfo);
  
  // read to which tiff sub type image pertence
  tiffParams->subType = tstGeneric;
  info->number_pages = tiffGetNumberOfPages( tiffParams );
  if (tif->tif_flags&TIFF_BIGTIFF) tiffParams->subType = tstBigTiff;

  if (stkIsTiffValid( tiffParams )) {
    tiffParams->subType = tstStk;
    stkGetInfo( tiffParams );
  } else
  if (psiaIsTiffValid( tiffParams )) {
    tiffParams->subType = tstPsia;
    psiaGetInfo ( tiffParams );
  } else
  if (isValidTiffFluoview(tiffParams)) {
    tiffParams->subType = tstFluoview;
    fluoviewGetInfo ( tiffParams );
  } else
  if (isValidTiffAndor(tiffParams)) {
    tiffParams->subType = tstAndor;
    fluoviewGetInfo ( tiffParams );
  } else
  if (lsmIsTiffValid( tiffParams )) {
    // lsm has thumbnails for each image, discard those
    tiffParams->subType = tstCzLsm;
    lsmGetInfo ( tiffParams );
  } else
  if (omeTiffIsValid(tiffParams)) {
    tiffParams->subType = tstOmeTiff;
    if (tif->tif_flags&TIFF_BIGTIFF) tiffParams->subType = tstOmeBigTiff;
    omeTiffGetInfo ( tiffParams );
  }

  getCurrentPageInfo( tiffParams );
}

//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimTiffValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length) {
  if (length < 4) return -1;
  if (memcmp(magic,d_magic_tiff_CLLT,4) == 0) return 0;
  if (memcmp(magic,d_magic_tiff_CLBG,4) == 0) return 0;
  if (memcmp(magic,d_magic_tiff_MDLT,4) == 0) return 0;
  if (memcmp(magic,d_magic_tiff_MDBG,4) == 0) return 0;
  if (memcmp(magic,d_magic_tiff_BGLT,4) == 0) return 0;
  if (memcmp(magic,d_magic_tiff_BGBG,4) == 0) return 0;
  return -1;
}

TDimFormatHandle dimTiffAquireFormatProc( void ) {
  return initTDimFormatHandle();
}


void dimTiffReleaseFormatProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  dimTiffCloseImageProc ( fmtHndl );  
  resetLocalTiffFunctions( );
}

//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimTiffSetWriteParameters (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  fmtHndl->compression = COMPRESSION_LZW;

  if (!fmtHndl->options) return;
  xstring str = fmtHndl->options;
  std::vector<xstring> options = str.split( " " );
  if (options.size() < 1) return;
  
  int i = -1;
  while (i<(int)options.size()-1) {
    i++;

    if ( options[i]=="compression" && options.size()-i>0 ) {
      i++;
      if (options[i] == "none") fmtHndl->compression = COMPRESSION_NONE;
      if (options[i] == "fax") fmtHndl->compression = COMPRESSION_CCITTFAX4;
      if (options[i] == "lzw") fmtHndl->compression = COMPRESSION_LZW;
      if (options[i] == "packbits") fmtHndl->compression = COMPRESSION_PACKBITS;
      continue;
    }
  } // while
}

void dimTiffCloseImageProc (TDimFormatHandle *fmtHndl)
{
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;

  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;

  if (fmtHndl->io_mode != DIM_IO_WRITE) {
    if (tiffpar != NULL) clearTiffIFDs( &tiffpar->ifds );
    clearMetaTags( &fmtHndl->metaData );
  }

  if ( (tiffpar != NULL) && (tiffpar->dimTiff != NULL) ) {
    XTIFFClose( tiffpar->dimTiff );
    tiffpar->dimTiff = NULL;
  }

  // close stream handle
  if ( fmtHndl->stream && !isCustomReading(fmtHndl) ) dimClose( fmtHndl );

  if (fmtHndl->internalParams != NULL) {
    delete tiffpar;
    fmtHndl->internalParams = NULL;
  }
}

DIM_UINT dimTiffOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode) {
  if (!fmtHndl) return 1;
  setLocalTiffFunctions( fmtHndl );

  dimTiffCloseImageProc( fmtHndl );
  DTiffParams *tiffpar = new DTiffParams();
  fmtHndl->internalParams = tiffpar;  

  TIFFSetWarningHandler(0);
  TIFFSetErrorHandler(0);

  if (io_mode == DIM_IO_WRITE) {
    std::string mode = "w";
    if (fmtHndl->subFormat==tstBigTiff || fmtHndl->subFormat==tstOmeBigTiff) mode = "w8";

    if ( isCustomWriting ( fmtHndl ) != TRUE )
      tiffpar->dimTiff = XTIFFOpen(fmtHndl->fileName, mode.c_str());
    else 
      tiffpar->dimTiff = XTIFFClientOpen( fmtHndl->fileName, mode.c_str(), // "wm"
        (thandle_t) fmtHndl, tiff_read, tiff_write, tiff_seek, tiff_close, tiff_size, tiff_mmap, tiff_unmap );
    dimTiffSetWriteParameters (fmtHndl); 
    if (fmtHndl->subFormat==tstOmeTiff || fmtHndl->subFormat==tstOmeBigTiff)
      tiffpar->subType = tstOmeTiff;
    else
      tiffpar->subType = tstGeneric;
  } else { // if reading

    
    // Use libtiff internal methods where possible, especially with the upcoming libtiff 4
    //if (!fmtHndl->stream && !isCustomReading(fmtHndl) )
    //  fmtHndl->stream = fopen( fmtHndl->fileName, "rb" );
    //if (!fmtHndl->stream) return 1;

    if ( isCustomReading ( fmtHndl ) != TRUE )
      tiffpar->dimTiff = XTIFFOpen(fmtHndl->fileName, "r");
    else 
      tiffpar->dimTiff = XTIFFClientOpen( fmtHndl->fileName, "r", // "rm"
        (thandle_t) fmtHndl, tiff_read, tiff_write, tiff_seek, tiff_close, tiff_size, tiff_mmap, tiff_unmap );

    if (tiffpar->dimTiff != NULL) {
      tiffpar->ifds = readAllTiffIFDs( tiffpar->dimTiff );
      getImageInfo(tiffpar);
      fmtHndl->subFormat = tiffpar->subType;
    }
  }

  if (tiffpar->dimTiff == NULL) return 1;

  return 0;
}

//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimTiffGetNumPagesProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;
  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;

  if (tiffpar->dimTiff == NULL) return 0;

  return tiffpar->info.number_pages;
}


TDimImageInfo dimTiffGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num ) {
  TDimImageInfo ii = initTDimImageInfo();

  if (fmtHndl == NULL) return ii;
  if (fmtHndl->internalParams == NULL) return ii;
  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;
  TIFF *tif = tiffpar->dimTiff;
  if (tif == NULL) return ii;

  fmtHndl->pageNumber = page_num;
  fmtHndl->subFormat = tiffpar->subType;

  unsigned int currentDir = TIFFCurrentDirectory(tif);

  // now must read correct page and set image parameters
  if (currentDir != fmtHndl->pageNumber) {
    if (tiffpar->subType != tstStk)
      TIFFSetDirectory(tif, fmtHndl->pageNumber);
    getCurrentPageInfo( tiffpar );
  }

  return tiffpar->info;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

// libTIFF CANNOT ADD TAGS INTO ANY GIVEN IMAGE
DIM_UINT dimTiffAddMetaDataProc (TDimFormatHandle *) {
  return 1;
}


DIM_UINT dimTiffReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;
  fmtHndl->pageNumber = page;
  return read_tiff_metadata (fmtHndl, tiffpar, group, tag, type);
}

char* dimTiffReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return NULL;
  if (fmtHndl->internalParams == NULL) return NULL;
  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;
  return read_text_tiff_metadata ( fmtHndl, tiffpar );
}

DIM_UINT tiffAppendMetadataProc (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;
  return tiff_append_metadata(fmtHndl, hash );
}



//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimTiffReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page )
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;

  if (tiffpar->dimTiff == NULL) return 1;
  fmtHndl->pageNumber = page;
  
  return read_tiff_image(fmtHndl, tiffpar);
}

DIM_UINT dimTiffWriteImageProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;
  if (tiffpar->dimTiff == NULL) return 1;
  return write_tiff_image(fmtHndl, tiffpar);
}

// at the moment w and h make no effect,the image retreived is the same size as original
DIM_UINT dimTiffReadImagePreviewProc (TDimFormatHandle *fmtHndl, DIM_UINT /*w*/, DIM_UINT /*h*/) {
  TDimImageBitmap bmp8, *bmp;
  initImagePlanes( &bmp8 );
  
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

//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

#define D_TIFF_NUM_FORMATS 9

TDimFormatItem dimTiffItems[D_TIFF_NUM_FORMATS] = {
  {
    "TIFF",            // short name, no spaces
    "Tagged Image File Format", // Long format name
    "tif|tiff|fax|geotiff",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    1, //canWriteMeta; // 0 - NO, 1 - YES
    1, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "STK",            // short name, no spaces
    "Metamorph Stack", // Long format name
    "stk",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )    
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "PSIA",            // short name, no spaces
    "AFM PSIA TIFF", // Long format name
    "tif|tiff",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )    
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "FLUOVIEW",            // short name, no spaces
    "Fluoview TIFF", // Long format name
    "tif|tiff",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )    
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "LSM",            // short name, no spaces
    "Carl Zeiss LSM 5/7", // Long format name
    "lsm",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )    
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "OME-TIFF",            // short name, no spaces
    "Open Microscopy TIFF", // Long format name
    "ome.tif|ome.tiff",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    1, //canWriteMeta; // 0 - NO, 1 - YES
    1, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "BigTIFF",            // short name, no spaces
    "Tagged Image File Format (64bit)", // Long format name
    "btf|tif|tiff|geotiff",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    1, //canWriteMeta; // 0 - NO, 1 - YES
    1, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "OME-BigTIFF",            // short name, no spaces
    "Open Microscopy BigTIFF", // Long format name
    "ome.btf|ome.tif|ome.tiff",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    1, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    1, //canWriteMeta; // 0 - NO, 1 - YES
    1, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    "ANDOR",            // short name, no spaces
    "Andor TIFF", // Long format name
    "tif|tiff",        // pipe "|" separated supported extension list
    1, //canRead;      // 0 - NO, 1 - YES
    0, //canWrite;     // 0 - NO, 1 - YES
    1, //canReadMeta;  // 0 - NO, 1 - YES
    0, //canWriteMeta; // 0 - NO, 1 - YES
    0, //canWriteMultiPage;   // 0 - NO, 1 - YES
    //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )    
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  }
};

TDimFormatHeader dimTiffHeader = {

  sizeof(TDimFormatHeader),
  "1.1.0",
  "DIMIN TIFF CODEC",
  "Tagged Image File Format variants",
  
  4,                      // 0 or more, specify number of bytes needed to identify the file
  {1, D_TIFF_NUM_FORMATS, dimTiffItems},   //dimTiffSupported,
  
  dimTiffValidateFormatProc,
  // begin
  dimTiffAquireFormatProc, //TDimAquireFormatProc
  // end
  dimTiffReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimTiffOpenImageProc, //TDimOpenImageProc
  dimTiffCloseImageProc, //TDimCloseImageProc 

  // info
  dimTiffGetNumPagesProc, //TDimGetNumPagesProc
  dimTiffGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimTiffReadImageProc, //TDimReadImageProc 
  dimTiffWriteImageProc, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  dimTiffReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimTiffReadMetaDataProc, //TDimReadMetaDataProc
  dimTiffAddMetaDataProc,  //TDimAddMetaDataProc
  dimTiffReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc
  tiffAppendMetadataProc, //TDimAppendMetaDataProc

  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimTiffGetFormatHeader(void)
{
  return &dimTiffHeader;
}

} // extern C





