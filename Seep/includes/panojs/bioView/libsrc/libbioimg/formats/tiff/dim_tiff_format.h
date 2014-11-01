/*****************************************************************************
  TIFF support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  Notes:
    Metadata can be red using readMetaData but can not br written to the file
    using addMetaData, it must be supplied with the formatHandler within
    writeImage function.

  History:
    03/29/2004 22:23 - First creation
        
  Ver : 1
*****************************************************************************/

#ifndef DIM_TIFF_FORMAT_H
#define DIM_TIFF_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <tiffvers.h>

// libtiff 3.8.2
#if (TIFFLIB_VERSION <= 20060323)
#define D_TIFF_SIZE_TYPE tsize_t
#define D_TIFF_DATA_TYPE tdata_t
#define D_TIFF_OFFS_TYPE toff_t
#define D_TIFF_BCNT_TYPE uint32
#define D_TIFF_STRP_TYPE tstrip_t
#endif

// libtiff 3.9.2
#if (TIFFLIB_VERSION == 20091104)
#define D_TIFF_SIZE_TYPE tsize_t
#define D_TIFF_DATA_TYPE tdata_t
#define D_TIFF_OFFS_TYPE toff_t
#define D_TIFF_BCNT_TYPE toff_t
#define D_TIFF_STRP_TYPE tstrip_t
#endif

// libtiff 4.0.0 beta5
#if (TIFFLIB_VERSION >= 20100101)
#define D_TIFF_SIZE_TYPE tmsize_t
#define D_TIFF_DATA_TYPE void*
#define D_TIFF_OFFS_TYPE uint64
#define D_TIFF_BCNT_TYPE uint64
#define D_TIFF_STRP_TYPE uint32
#endif

#include "dim_tiny_tiff.h"
#include "dim_psia_format.h"
#include "dim_stk_format.h"
#include "dim_fluoview_format.h"
#include "dim_cz_lsm_format.h"
#include "dim_ometiff_format.h"

#include <tiffio.h>
#include <tif_dir.h>

const unsigned char d_magic_tiff_CLLT[4] = {0x4d, 0x4d, 0x00, 0x2a};
const unsigned char d_magic_tiff_CLBG[4] = {0x49, 0x49, 0x2a, 0x00};
const unsigned char d_magic_tiff_MDLT[4] = {0x50, 0x45, 0x00, 0x2a};
const unsigned char d_magic_tiff_MDBG[4] = {0x45, 0x50, 0x2a, 0x00};
const unsigned char d_magic_tiff_BGLT[4] = {0x4d, 0x4d, 0x00, 0x2b};
const unsigned char d_magic_tiff_BGBG[4] = {0x49, 0x49, 0x2b, 0x00};

typedef enum {
  tstGeneric  = 0,
  tstStk      = 1,
  tstPsia     = 2,
  tstFluoview = 3,
  tstCzLsm    = 4,
  tstOmeTiff  = 5,
  tstBigTiff  = 6,
  tstOmeBigTiff  = 7
} DIM_TiffSubType;

class DTiffParams {
public:
  DTiffParams();

  TDimImageInfo info;

  TIFF  *dimTiff;
  DIM_TiffSubType subType;
  TDimTiffIFDs ifds;

  DStkInfo stkInfo;
  psiaInfoHeader psiaInfo;
  DFluoviewInfo fluoviewInfo;
  DLsmInfo lsmInfo;
  DOMETiffInfo omeTiffInfo;
};


// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimTiffGetFormatHeader(void);
}

// INTERNAL FUNCTIONS
DIM_INT dimTiffValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length);

TDimFormatHandle dimTiffAquireFormatProc( void );
void dimTiffReleaseFormatProc (TDimFormatHandle *fmtHndl);

DIM_UINT dimTiffOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode);
void dimTiffCloseImageProc     (TDimFormatHandle *fmtHndl);

DIM_UINT dimTiffGetNumPagesProc       ( TDimFormatHandle *fmtHndl );
TDimImageInfo dimTiffGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT page_num );

DIM_UINT dimTiffReadImageProc  (TDimFormatHandle *fmtHndl, DIM_UINT page);
DIM_UINT dimTiffWriteImageProc (TDimFormatHandle *fmtHndl);

DIM_UINT dimTiffReadImagePreviewProc (TDimFormatHandle *fmtHndl, DIM_UINT w, DIM_UINT h);

DIM_UINT dimTiffReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type);
char* dimTiffReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl );

#endif // DIM_TIFF_FORMAT_H
