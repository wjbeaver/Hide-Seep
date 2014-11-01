/*****************************************************************************
  TIFF STK IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  TODO:
    At this point it's not supporting more then one sample per pixel...
    Check compression related to stripes, we might change it later

  History:
    03/29/2004 22:23 - First creation
    09/28/2005 23:10 - fixed bugs for bigendian architecture
        
  Ver : 2
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <string>

#include <tag_map.h>

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

#include "memio.h"
#include "dim_tiff_format.h"
#include "dim_stk_format.h"

#define HOURS_PER_DAY 24
#define MINS_PER_HOUR 60
#define SECS_PER_MIN  60
#define MSECS_PER_SEC 1000
#define MINS_PER_DAY  (HOURS_PER_DAY * MINS_PER_HOUR)
#define SECS_PER_DAY  (MINS_PER_DAY * SECS_PER_MIN)
#define MSECS_PER_DAY (SECS_PER_DAY * MSECS_PER_SEC)

//----------------------------------------------------------------------------
// misc
//----------------------------------------------------------------------------
void initRational(TDimStkRational *r, DIM_INT32 N) {
  if (r == NULL) return;
  for (int i=0; i<N; i++) {
    r[i].num = 0;
    r[i].den = 1;
  }
}

void initLong(DIM_INT32 *r, DIM_INT32 N) {
  if (r == NULL) return;
  for (int i=0; i<N; i++) r[i] = 0;
}

//----------------------------------------------------------------------------
// housekeeping
//----------------------------------------------------------------------------
DStkInfo::DStkInfo() {
  init();
}

DStkInfo::~DStkInfo() {
  clear();
}

void DStkInfo::init() {
  this->strips_per_plane = 0;
  this->strip_offsets = NULL;
  this->strip_bytecounts = NULL;

  this->metaData.CalibrationUnits  = NULL;
  this->metaData.Name              = NULL;
  this->metaData.grayUnitName      = NULL;
  this->metaData.StagePositionX    = NULL;
  this->metaData.StagePositionY    = NULL;
  this->metaData.CameraChipOffsetX = NULL;
  this->metaData.CameraChipOffsetY = NULL;
  this->metaData.StageLabel        = NULL;
  this->metaData.AbsoluteZ         = NULL;
  this->metaData.AbsoluteZValid    = NULL;
  this->metaData.wavelength        = NULL;
  this->metaData.zDistance         = NULL;
  this->metaData.creationDate      = NULL;
  this->metaData.creationTime      = NULL;
  this->metaData.modificationDate  = NULL;
  this->metaData.modificationTime  = NULL;

  this->metaData.N                      = 0;
  this->metaData.AutoScale              = 0;
  this->metaData.MinScale               = 0;
  this->metaData.MaxScale               = 0;
  this->metaData.SpatialCalibration     = 0;
  this->metaData.XCalibration[0]        = 0;
  this->metaData.XCalibration[1]        = 1;
  this->metaData.YCalibration[0]        = 0;
  this->metaData.YCalibration[1]        = 1;
  this->metaData.ThreshState            = 0;
  this->metaData.ThreshStateRed         = 0;
  this->metaData.ThreshStateGreen       = 0;
  this->metaData.ThreshStateBlue        = 0;
  this->metaData.ThreshStateLo          = 0;
  this->metaData.ThreshStateHi          = 0;
  this->metaData.Zoom                   = 0;
  this->metaData.CreateTime[0]          = 0;
  this->metaData.CreateTime[1]          = 1;
  this->metaData.LastSavedTime[0]       = 0;
  this->metaData.LastSavedTime[1]       = 1;
  this->metaData.currentBuffer          = 0;
  this->metaData.grayFit                = 0;
  this->metaData.grayPointCount         = 0;
  this->metaData.grayX[0]               = 0;
  this->metaData.grayX[1]               = 1;
  this->metaData.grayY[0]               = 0;
  this->metaData.grayY[1]               = 1;
  this->metaData.grayMin[0]             = 0;
  this->metaData.grayMin[1]             = 1;
  this->metaData.grayMax[0]             = 0;
  this->metaData.grayMax[1]             = 1;
  this->metaData.StandardLUT            = 0;
  this->metaData.OverlayMask            = 0;
  this->metaData.OverlayCompress        = 0;
  this->metaData.Overlay                = 0;
  this->metaData.SpecialOverlayMask     = 0;
  this->metaData.SpecialOverlayCompress = 0;
  this->metaData.SpecialOverlay         = 0;
  this->metaData.ImageProperty          = 0;
  this->metaData.AutoScaleLoInfo[0]     = 0;
  this->metaData.AutoScaleLoInfo[1]     = 1;
  this->metaData.AutoScaleHiInfo[0]     = 0;
  this->metaData.AutoScaleHiInfo[1]     = 1;
  this->metaData.Gamma                  = 0;
  this->metaData.GammaRed               = 0;
  this->metaData.GammaGreen             = 0;
  this->metaData.GammaBlue              = 0;
}

void DStkInfo::clearOffsets() {
  if ( (this->strip_offsets != NULL) && (this->strips_per_plane > 0) )
    _TIFFfree(this->strip_offsets);

  if ( (this->strip_bytecounts != NULL) && (this->strips_per_plane > 0) )
    _TIFFfree(this->strip_bytecounts);

  this->strip_offsets    = NULL;
  this->strip_bytecounts = NULL;
  this->strips_per_plane = 0;
}

void DStkInfo::clear() {
  clearOffsets();
 
  if (  this->metaData.CalibrationUnits != NULL )
    _TIFFfree(this->metaData.CalibrationUnits);

  if (  this->metaData.Name != NULL )
    _TIFFfree(this->metaData.Name);

  if (  this->metaData.grayUnitName != NULL )
    _TIFFfree(this->metaData.grayUnitName);

  if (  this->metaData.StagePositionX != NULL )
    _TIFFfree(this->metaData.StagePositionX);

  if (  this->metaData.StagePositionY != NULL )
    _TIFFfree(this->metaData.StagePositionY);

  if (  this->metaData.CameraChipOffsetX != NULL )
    _TIFFfree(this->metaData.CameraChipOffsetX);

  if (  this->metaData.CameraChipOffsetY != NULL )
    _TIFFfree(this->metaData.CameraChipOffsetY); 

  if (  this->metaData.StageLabel != NULL )  
    _TIFFfree(this->metaData.StageLabel);

  if (  this->metaData.AbsoluteZ != NULL )
    _TIFFfree(this->metaData.AbsoluteZ);

  if (  this->metaData.AbsoluteZValid != NULL )
    _TIFFfree(this->metaData.AbsoluteZValid);

  if (  this->metaData.wavelength != NULL )
    _TIFFfree(this->metaData.wavelength);

  if (  this->metaData.zDistance != NULL )
    _TIFFfree(this->metaData.zDistance);

  if (  this->metaData.creationDate != NULL )
    _TIFFfree(this->metaData.creationDate);

  if (  this->metaData.creationTime != NULL )
    _TIFFfree(this->metaData.creationTime);

  if (  this->metaData.modificationDate != NULL )
    _TIFFfree(this->metaData.modificationDate);

  if (  this->metaData.modificationTime != NULL )
    _TIFFfree(this->metaData.modificationTime);

  init();
}

void DStkInfo::allocMetaInfo( DIM_INT32 N ) {
  if (N <= 0) return;

  this->metaData.N = N;

  this->metaData.StagePositionX = (TDimStkRational *) _TIFFmalloc( N*sizeof(TDimStkRational) );
  initRational(this->metaData.StagePositionX, N );

  this->metaData.StagePositionY = (TDimStkRational *) _TIFFmalloc( N*sizeof(TDimStkRational) );
  initRational(this->metaData.StagePositionY, N );

  this->metaData.CameraChipOffsetX = (TDimStkRational *) _TIFFmalloc( N*sizeof(TDimStkRational) );
  initRational(this->metaData.CameraChipOffsetX, N );

  this->metaData.CameraChipOffsetY = (TDimStkRational *) _TIFFmalloc( N*sizeof(TDimStkRational) );
  initRational(this->metaData.CameraChipOffsetY, N );

  this->metaData.AbsoluteZ = (TDimStkRational *) _TIFFmalloc( N*sizeof(TDimStkRational) );
  initRational(this->metaData.AbsoluteZ, N );

  this->metaData.AbsoluteZValid = (DIM_INT32 *) _TIFFmalloc( N*sizeof(DIM_INT32) );
  initLong(this->metaData.AbsoluteZValid, N );

  this->metaData.wavelength = (TDimStkRational *) _TIFFmalloc( N*sizeof(TDimStkRational) );
  initRational(this->metaData.wavelength, N );

  this->metaData.zDistance = (TDimStkRational *) _TIFFmalloc( N*sizeof(TDimStkRational) );
  initRational(this->metaData.zDistance, N );

  this->metaData.creationDate = (DIM_INT32 *) _TIFFmalloc( N*sizeof(DIM_INT32) );
  initLong(this->metaData.creationDate, N );

  this->metaData.creationTime = (DIM_INT32 *) _TIFFmalloc( N*sizeof(DIM_INT32) );
  initLong(this->metaData.creationTime, N );

  this->metaData.modificationDate = (DIM_INT32 *) _TIFFmalloc( N*sizeof(DIM_INT32) );
  initLong(this->metaData.modificationDate, N );

  this->metaData.modificationTime = (DIM_INT32 *) _TIFFmalloc( N*sizeof(DIM_INT32) );
  initLong(this->metaData.modificationTime, N );

  #if defined(DEBUG) || defined(_DEBUG)
  printf("STK: Allocated data for %d pages\n", N);  
  #endif  
}

//----------------------------------------------------------------------------
// STK MISC FUNCTIONS
//----------------------------------------------------------------------------
void JulianToYMD(unsigned DIM_INT32 julian, unsigned short& year, unsigned char& month, unsigned char& day) {
  year=0; month=0; day=0;
  if (julian == 0) return;

  DIM_INT32  a, b, c, d, e, alpha;
  DIM_INT32  z = julian + 1;

  // dealing with Gregorian calendar reform
  if (z < 2299161L) {
    a = z;
  } else {
    alpha = (DIM_INT32) ((z - 1867216.25) / 36524.25);
    a = z + 1 + alpha - alpha / 4;
  }

  b = ( a > 1721423L ? a + 1524 : a + 1158 );
  c = (DIM_INT32) ((b - 122.1) / 365.25);
  d = (DIM_INT32) (365.25 * c);
  e = (DIM_INT32) ((b - d) / 30.6001);

  day   = (unsigned char) (b - d - (DIM_INT32)(30.6001 * e));
  month = (unsigned char) ((e < 13.5) ? e - 1 : e - 13) ; // -1 dima: sub 1 to match Date/Time TIFF tag, dima: old fix, not needed for new
  year  = (short)         ((month > 2.5 ) ? (c - 4716) : c - 4715);
}

unsigned DIM_INT32 YMDToJulian(unsigned short year, unsigned char month, unsigned char day) {
  short a, b = 0;
  short work_year = year;
  short work_month = month;
  short work_day = day;

  // correct for negative year
  if (work_year < 0) {
    work_year++;
  }

  if (work_month <= 2) {
    work_year--;
    work_month += (short)12;
  }

  // deal with Gregorian calendar
  if (work_year*10000. + work_month*100. + work_day >= 15821015.) {
    a = (short)(work_year/100.);
    b = (short)(2 - a + a/4);
  }

  return  (DIM_INT32) (365.25*work_year) +
          (DIM_INT32) (30.6001 * (work_month+1)) +
          work_day + 1720994L + b;
}

void divMod(int Dividend, int Divisor, int &Result, int &Remainder)
{
  Result    = (int) ( ((double) Dividend) / ((double) Divisor) );
  Remainder = Dividend % Divisor;
}

// this convert miliseconds since midnight into hour/minute/second
void MiliMidnightToHMS(unsigned DIM_INT32 ms, unsigned char& hour, unsigned char& minute, unsigned char& second) {
  hour=0; minute=0; second=0;  
  if (ms == 0) return;
  int h, m, s, fms, mc, msc;
  divMod(ms,  SECS_PER_MIN * MSECS_PER_SEC, mc, msc);
  divMod(mc,  MINS_PER_HOUR, h, m);
  divMod(msc, MSECS_PER_SEC, s, fms);
  hour = h; minute = m; second = s;
}

// String versions
std::string JulianToAnsi(unsigned DIM_INT32 julian) {
  unsigned short y=0;
  unsigned char m=0, d=0;
  JulianToYMD(julian, y, m, d);
  return xstring::xprintf( "%.4d-%.2d-%.2d", y, m, d );
}

std::string MiliMidnightToAnsi(unsigned DIM_INT32 ms) {
  unsigned char h=0, mi=0, s=0;
  MiliMidnightToHMS(ms, h, mi, s);
  return xstring::xprintf( "%.2d:%.2d:%.2d", h, mi, s );
}

std::string StkDateTimeToAnsi(unsigned DIM_INT32 julian, unsigned DIM_INT32 ms) {
  std::string s = JulianToAnsi(julian);
  s += " ";
  s += MiliMidnightToAnsi(ms);
  return s;
}

//----------------------------------------------------------------------------
// STK UTIL FUNCTIONS
//----------------------------------------------------------------------------

bool stkAreValidParams(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return FALSE;
  if (tiffParams->dimTiff == NULL) return FALSE;
  return TRUE;
}

DIM_INT32 stkGetNumPlanes(TIFF *tif) {
  double *d_list = NULL;
  DIM_INT32 *l_list = NULL;
  int16   d_list_count[3];
  int res[3] = {0,0,0};

  if (tif == 0) return 0;

  res[0] = TIFFGetField(tif, 33629, &d_list_count[0], &d_list);
  res[1] = TIFFGetField(tif, 33630, &d_list_count[1], &d_list);
  res[2] = TIFFGetField(tif, 33631, &d_list_count[2], &l_list);

  // if tag 33629 exists then the file is valid STAK file
  if (res[0] == 1) return d_list_count[0];
  if (res[1] == 1) return d_list_count[1];
  if (res[2] == 1) return d_list_count[2];

  return 1;
}

bool stkIsTiffValid(TIFF *tif) {
  if (tif->tif_flags&TIFF_BIGTIFF) return false;    
  double *d_list = NULL;
  DIM_INT32 *l_list = NULL;
  int16   d_list_count;
  int res[3] = {0,0,0};

  if (tif == 0) return FALSE;

  res[0] = TIFFGetField(tif, 33629, &d_list_count, &d_list);
  res[1] = TIFFGetField(tif, 33630, &d_list_count, &d_list);
  res[2] = TIFFGetField(tif, 33631, &d_list_count, &l_list);

  // if tag 33629 exists then the file is valid STAK file
  if (res[0] == 1) return TRUE;
  if (res[1] == 1) return TRUE;
  if (res[2] == 1) return TRUE;

  return FALSE;
}

bool stkIsTiffValid(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return FALSE;
  if (tiffParams->dimTiff->tif_flags&TIFF_BIGTIFF) return FALSE;

  // if tag 33629 exists then the file is valid STAK file
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 33629 ) == TRUE) return TRUE;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 33630 ) == TRUE) return TRUE;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 33631 ) == TRUE) return TRUE;

  return FALSE;
}

template<typename Tb, typename To>
void copy_clean_tag_buffer( Tb **buf, unsigned int size_in_bytes, To **out ) {
  if (!*buf) return;
  unsigned int n = size_in_bytes / sizeof(Tb);
  unsigned int out_size_in_bytes = n * sizeof(To);
  *out = (To *) _TIFFmalloc( out_size_in_bytes );

  if (size_in_bytes == out_size_in_bytes)
    _TIFFmemcpy( *out, *buf, size_in_bytes );
  else
    // slow conversion
    for (unsigned int i=0; i<n; ++i)
      (*out)[i] = (*buf)[i];

  _TIFFfree( *buf );
}

void stkGetOffsets(TIFF *tif, DTiffParams *tiffParams ) {
  /*
  the default libtiff strip reading is not good enough since it only gives us strip offsets for 
  the first plane although all the offsets for all the planes are actually stored
  so for uncompressed images use default and for LZW the custom methods
  */
  
  DStkInfo *stkInfo = &tiffParams->stkInfo;
  stkInfo->strips_per_plane = TIFFNumberOfStrips( tif );

  if (stkInfo->strip_offsets != NULL) _TIFFfree(stkInfo->strip_offsets);
  stkInfo->strip_offsets = NULL;
  if (stkInfo->strip_bytecounts != NULL) _TIFFfree(stkInfo->strip_bytecounts);
  stkInfo->strip_bytecounts = NULL;

  if (tiffParams == NULL) return;
  if (tiffParams->dimTiff == NULL) return;
  if (tiffParams->subType != tstStk) return;

  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];
  int N = tiffParams->stkInfo.metaData.N;
  uint32 tag = 0;

  // -----------------------------
  // read Strip Offsets
  // -----------------------------
  tag = 273;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, tag ) == TRUE) {
    uint32 *buf=NULL, size=0, type=0;
    readTiffTag ( tif, ifd, tag, size, type, (DIM_UCHAR **) &buf );
    copy_clean_tag_buffer<uint32, D_TIFF_OFFS_TYPE>( &buf, size, &stkInfo->strip_offsets );

  } // strip offsets
  
  // -----------------------------
  // read strip_bytecounts also
  // -----------------------------
  tag = 279;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, tag ) == TRUE) {
    uint32 *buf=NULL, size=0, type=0;
    readTiffTag ( tif, ifd, tag, size, type, (DIM_UCHAR **) &buf );
    copy_clean_tag_buffer<uint32, D_TIFF_BCNT_TYPE>( &buf, size, &stkInfo->strip_bytecounts );
  } // strip_bytecounts
}


//----------------------------------------------------------------------------
// PARSE UIC TAGS
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// UIC2Tag - 33629  
//----------------------------------------------------------------------------
//At the indicated offset, there is a table of 6*N LONG values 
//indicating for each plane in order: 
//Z distance (numerator), Z distance (denominator), 
//creation date, creation time, modification date, modification time.
void stkParseUIC2Tag( DTiffParams *tiffParams )
{
  if (tiffParams == NULL) return;
  if (tiffParams->dimTiff == NULL) return;
  if (tiffParams->subType != tstStk) return;

  DStkInfo *stkInfo = &tiffParams->stkInfo;
  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];

  // read UIC2
  int tag = 33629;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, tag ) != TRUE) return;
  else
  {
    DIM_INT32 i;
    DIM_INT32 N = stkInfo->metaData.N;
    DIM_UINT32 *buf = NULL;
    readTiffCustomTag (tiffParams->dimTiff, ifd, tag, 6*N*sizeof(DIM_INT32), DIM_TAG_LONG, (DIM_UCHAR **) &buf);
    if (buf == NULL) return;

    for (i=0; i<N; i++) {
      stkInfo->metaData.zDistance[i].num    = buf[i*6+0];
      stkInfo->metaData.zDistance[i].den    = buf[i*6+1];
      stkInfo->metaData.creationDate[i]     = buf[i*6+2];
      stkInfo->metaData.creationTime[i]     = buf[i*6+3];
      stkInfo->metaData.modificationDate[i] = buf[i*6+4];
      stkInfo->metaData.modificationTime[i] = buf[i*6+5];
    }

    freeTiffTagBuf( (DIM_UCHAR **) &buf );
  }
}

//----------------------------------------------------------------------------
// UIC3Tag - 33630 
//----------------------------------------------------------------------------
//A table of 2*N LONG values indicating for each plane in order: 
//wavelength (numerator), wavelength (denominator).
void stkParseUIC3Tag( DTiffParams *tiffParams )
{
  if (tiffParams == NULL) return;
  if (tiffParams->dimTiff == NULL) return;
  if (tiffParams->subType != tstStk) return;

  DStkInfo *stkInfo = &tiffParams->stkInfo;
  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];

  // read UIC3
  int tag = 33630;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, tag ) != TRUE) return;
  else
  {
    DIM_INT32 i=0;
    DIM_INT32 N = tiffParams->stkInfo.metaData.N;
    DIM_UINT32 *buf = NULL;
 
    readTiffCustomTag (tiffParams->dimTiff, ifd, tag, 2*N*sizeof(DIM_UINT32), DIM_TAG_LONG, (DIM_UCHAR **) &buf);
    if (buf == NULL) return;

    for (i=0; i<N; i++)
    {
      stkInfo->metaData.wavelength[i].num = buf[i*2];
      stkInfo->metaData.wavelength[i].den = buf[i*2+1];
    }

    freeTiffTagBuf( (DIM_UCHAR **) &buf );
  }
}

//----------------------------------------------------------------------------
// UIC4Tag - 33631 (TIFFTAG_STK_UIC4)
//----------------------------------------------------------------------------
/*
UIC4Tag
Tag	= 33631 (835F.H)
Type	= LONG
N	= Number of planes

At the indicated offset, there is a series of pairs consisting of an ID code 
of size SHORT and a variable-sized (ID-dependent) block of data. Pairs should 
be read until an ID of 0 is encountered. The possible tags and their values 
are defined in the “Tag ID Codes” section below. The “AutoScale” tag never 
appears in this table (because its ID is used to terminate the table.)
*/
void stkParseUIC4Tag( DTiffParams *tiffParams ) {
  if (tiffParams == NULL) return;
  if (tiffParams->dimTiff == NULL) return;
  if (tiffParams->subType != tstStk) return;

  DStkInfo *stkInfo = &tiffParams->stkInfo;
  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];

  // read UIC4
  int tag = TIFFTAG_STK_UIC4;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, tag ) == TRUE) {

    DIM_INT32 N = stkInfo->metaData.N;
    short tag_id;
    std::vector<unsigned char> buffer;
    int32 data_offset = getTiffTagOffset(tiffParams->dimTiff, ifd, tag);

    while (data_offset != 0) {
      // first read key for a tuple
      if ( readTiffBufNoAlloc (tiffParams->dimTiff, data_offset, 2, TIFF_SHORT, (uchar *) &tag_id) != 0) break;
      //safeguard, currently i only know of keys <= 45, stop otherwise
      if (tag_id == 0 || tag_id > DIM_STK_CameraChipOffset) break;
      data_offset += 2;

      if (tag_id == DIM_STK_StagePosition) {
        int buf_size = N*4*sizeof(uint32);
        if (buffer.size() < buf_size) buffer.resize(buf_size);
        DIM_INT32 *buf = (DIM_INT32 *) &buffer[0];
        if ( readTiffBufNoAlloc (tiffParams->dimTiff, data_offset, buf_size, TIFF_LONG, (unsigned char *) buf ) != 0) break;
        data_offset += buf_size;
        for (int i=0; i<N; ++i) {
          stkInfo->metaData.StagePositionX[i].num = buf[i*4+0];
          stkInfo->metaData.StagePositionX[i].den = buf[i*4+1];
          stkInfo->metaData.StagePositionY[i].num = buf[i*4+2];
          stkInfo->metaData.StagePositionY[i].den = buf[i*4+3];
        }
        continue;
      } // tag DIM_STK_StagePosition

      if (tag_id == DIM_STK_CameraChipOffset) {
        int buf_size = N*4*sizeof(uint32);
        if (buffer.size() < buf_size) buffer.resize(buf_size);
        DIM_INT32 *buf = (DIM_INT32 *) &buffer[0];
        if ( readTiffBufNoAlloc (tiffParams->dimTiff, data_offset, buf_size, TIFF_LONG, (unsigned char *) buf ) != 0) break;
        data_offset += buf_size;
        for (int i=0; i<N; ++i) {
          stkInfo->metaData.CameraChipOffsetX[i].num = buf[i*4+0];
          stkInfo->metaData.CameraChipOffsetX[i].den = buf[i*4+1];
          stkInfo->metaData.CameraChipOffsetY[i].num = buf[i*4+2];
          stkInfo->metaData.CameraChipOffsetY[i].den = buf[i*4+3];
        }
        continue;
      } // tag DIM_STK_CameraChipOffset
    } // while
  }
}

//----------------------------------------------------------------------------
// UIC1Tag - 33628
//----------------------------------------------------------------------------

//If this tag exists and is of type LONG, then at the indicated offset 
//there is a series of N pairs consisting of an ID code of size LONG and a 
//variable-sized (ID-dependent) block of data. The possible tags and their 
//values are defined in the "Tag ID Codes" section below. These values are 
//used internally by the Meta Imaging Series applications, and may not be 
//useful to other applications. To replicate the behavior of MetaMorph, 
//this table should be read and its values stored after the table 
//indicated by UIC4Tag is read.
// NOTE: difference form tag UIC4 is that this one contains exactly N pairs
// and long organization of data... We're not reading tag UIC4 at all at the moment

void stkReadStringEntry(DIM_INT32 offset, DTiffParams *tiffParams, char **string) {
  //DStkInfo *stkInfo = &tiffParams->stkInfo;
  TIFF *tif = tiffParams->dimTiff;
  if (offset <= 0) return;

  DIM_INT32 size;
  readTiffBufNoAlloc (tif, offset, sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) &size);
  *string = (char *) _TIFFmalloc( size+1 );
  (*string)[size] = '\0';
  readTiffBufNoAlloc (tif, offset+sizeof( DIM_INT32 ), size, DIM_TAG_LONG, (DIM_UCHAR *) (*string));
}

void stkParseIDEntry(DIM_INT32 *pair, DIM_INT32 offset, DTiffParams *tiffParams) {
  DStkInfo *stkInfo = &tiffParams->stkInfo;
  //TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];
  TIFF *tif = tiffParams->dimTiff;

  if (pair[0] == DIM_STK_AutoScale)
    stkInfo->metaData.AutoScale = pair[1];

  if (pair[0] == DIM_STK_MinScale)
    stkInfo->metaData.MinScale = pair[1];

  if (pair[0] == DIM_STK_MaxScale)
    stkInfo->metaData.MaxScale = pair[1];

  if (pair[0] == DIM_STK_SpatialCalibration)
    stkInfo->metaData.SpatialCalibration = pair[1];

  if (pair[0] == DIM_STK_XCalibration)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.XCalibration);

  if (pair[0] == DIM_STK_YCalibration)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.YCalibration);

  if (pair[0] == DIM_STK_CalibrationUnits)
    stkReadStringEntry(pair[1], tiffParams, (char **) &stkInfo->metaData.CalibrationUnits);

  if (pair[0] == DIM_STK_Name)
    stkReadStringEntry(pair[1], tiffParams, (char **) &stkInfo->metaData.Name);

  if (pair[0] == DIM_STK_ThreshState)
    stkInfo->metaData.ThreshState = pair[1];

  if (pair[0] == DIM_STK_ThreshStateRed)
    stkInfo->metaData.ThreshStateRed = pair[1];

  if (pair[0] == DIM_STK_ThreshStateGreen)
    stkInfo->metaData.ThreshStateGreen = pair[1];

  if (pair[0] == DIM_STK_ThreshStateBlue)
    stkInfo->metaData.ThreshStateBlue = pair[1];

  if (pair[0] == DIM_STK_ThreshStateLo)
    stkInfo->metaData.ThreshStateLo = pair[1];

  if (pair[0] == DIM_STK_ThreshStateHi)
    stkInfo->metaData.ThreshStateHi = pair[1];

  if (pair[0] == DIM_STK_Zoom)
    stkInfo->metaData.Zoom = pair[1];

  if (pair[0] == DIM_STK_CreateTime)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.CreateTime);

  if (pair[0] == DIM_STK_LastSavedTime)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.LastSavedTime);

  if (pair[0] == DIM_STK_currentBuffer)
    stkInfo->metaData.currentBuffer = pair[1];

  if (pair[0] == DIM_STK_grayFit)
    stkInfo->metaData.grayFit = pair[1];

  if (pair[0] == DIM_STK_grayPointCount)
    stkInfo->metaData.grayPointCount = pair[1];

  if (pair[0] == DIM_STK_grayX)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.grayX);

  if (pair[0] == DIM_STK_grayY)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.grayY);

  if (pair[0] == DIM_STK_grayMin)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.grayMin);

  if (pair[0] == DIM_STK_grayMax)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.grayMax);

  if (pair[0] == DIM_STK_grayUnitName)
    stkReadStringEntry(pair[1], tiffParams, (char **) &stkInfo->metaData.grayUnitName);

  if (pair[0] == DIM_STK_StandardLUT)
    stkInfo->metaData.StandardLUT = pair[1];

  if (pair[0] == DIM_STK_AutoScaleLoInfo)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.AutoScaleLoInfo);

  if (pair[0] == DIM_STK_AutoScaleHiInfo)
    readTiffBufNoAlloc (tif, pair[1], 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.AutoScaleHiInfo);

  if (pair[0] == DIM_STK_Gamma)
    readTiffBufNoAlloc (tif, pair[1], sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) &stkInfo->metaData.Gamma);

  if (pair[0] == DIM_STK_GammaRed)
    readTiffBufNoAlloc (tif, pair[1], sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) &stkInfo->metaData.GammaRed);

  if (pair[0] == DIM_STK_GammaGreen)
    readTiffBufNoAlloc (tif, pair[1], sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) &stkInfo->metaData.GammaGreen);

  if (pair[0] == DIM_STK_GammaBlue)
    readTiffBufNoAlloc (tif, pair[1], sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) &stkInfo->metaData.GammaBlue);

  DIM_INT32 N = stkInfo->metaData.N;

  /*
  std::vector<unsigned char> buffer;

  // only teh first page value can be red here
  if (pair[0] == DIM_STK_StagePosition) {
    int buf_size = N*4*sizeof(uint32);
    if (buffer.size() < buf_size) buffer.resize(buf_size);
    DIM_INT32 *buf = (DIM_INT32 *) &buffer[0];
    if ( readTiffBufNoAlloc (tif, pair[1], buf_size, DIM_TAG_LONG, (unsigned char *) buf ) == 0) {
      for (int i=0; i<N; ++i) {
        stkInfo->metaData.StagePositionX[i].num = buf[i*4+0];
        stkInfo->metaData.StagePositionX[i].den = buf[i*4+1];
        stkInfo->metaData.StagePositionY[i].num = buf[i*4+2];
        stkInfo->metaData.StagePositionY[i].den = buf[i*4+3];
      } // for i
    } // if read tiff buf
  } // DIM_STK_StagePosition

  if (pair[0] == DIM_STK_CameraChipOffset) {
    int buf_size = N*4*sizeof(uint32);
    if (buffer.size() < buf_size) buffer.resize(buf_size);
    DIM_INT32 *buf = (DIM_INT32 *) &buffer[0];
    if ( readTiffBufNoAlloc (tif, pair[1], buf_size, DIM_TAG_LONG, (unsigned char *) buf ) == 0) {
      for (int i=0; i<N; ++i) {
        stkInfo->metaData.CameraChipOffsetX[i].num = buf[i*4+0];
        stkInfo->metaData.CameraChipOffsetX[i].den = buf[i*4+1];
        stkInfo->metaData.CameraChipOffsetY[i].num = buf[i*4+2];
        stkInfo->metaData.CameraChipOffsetY[i].den = buf[i*4+3];
      } // for i
    } // if read tiff buf
  } // DIM_STK_CameraChipOffset
  */
  
  if (pair[0] == DIM_STK_AbsoluteZ)
    readTiffBufNoAlloc (tif, pair[1], N*2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.AbsoluteZ);

  if (pair[0] == DIM_STK_AbsoluteZValid)
    readTiffBufNoAlloc (tif, pair[1], N*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) stkInfo->metaData.AbsoluteZValid);
}

void stkParseUIC1Tag( DTiffParams *tiffParams )
{
  if (tiffParams == NULL) return;
  if (tiffParams->dimTiff == NULL) return;
  if (tiffParams->subType != tstStk) return;

  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];
  TIFF *tif = tiffParams->dimTiff;

  int tag = 33628;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, tag ) != TRUE) return;
  
  DIM_INT32 i, offset;
  DIM_INT32 N = tiffParams->stkInfo.metaData.N; 
  DIM_INT32 pair[2];

  offset = getTiffTagOffset(tif, ifd, tag);
  if (offset == -1) return;
  DIM_INT32 num_ids = getTiffTagCount(tif, ifd, tag);
  DIM_INT32 id_offset = offset;

  // now read and parce ID table
  for (i=0; i<num_ids; i++) {
    readTiffBufNoAlloc( tif, id_offset, 2*sizeof( DIM_INT32 ), DIM_TAG_LONG, (DIM_UCHAR *) pair);
    stkParseIDEntry(pair, id_offset, tiffParams);
    id_offset += 2*sizeof( DIM_INT32 );
  }

}

void stkParseUICTags( DTiffParams *tiffParams ) {
  if (tiffParams == NULL) return;
  if (tiffParams->dimTiff == NULL) return;
  if (tiffParams->subType != tstStk) return;
  
  stkParseUIC2Tag( tiffParams );
  stkParseUIC3Tag( tiffParams );
  stkParseUIC4Tag( tiffParams );
  stkParseUIC1Tag( tiffParams );
}


//----------------------------------------------------------------------------
// STK INFO
//----------------------------------------------------------------------------

int stkGetInfo ( DTiffParams *tiffParams )
{
  if (tiffParams == NULL) return 1;
  if (tiffParams->dimTiff == NULL) return 1;
  if (tiffParams->ifds.count <= 0) return 1;

  tiffParams->stkInfo.allocMetaInfo( stkGetNumPlanes( tiffParams->dimTiff ) );
  stkGetOffsets( tiffParams->dimTiff, tiffParams );
  stkParseUICTags( tiffParams );

  //---------------------------------------------------------------
  // define dims
  //---------------------------------------------------------------
  DStkInfo *stk = &tiffParams->stkInfo;
  TDimImageInfo *info = &tiffParams->info;
  TDimStkMetaData *meta = &stk->metaData;
  
  //info->samples = 1;
  info->number_pages = meta->N;
  info->number_z = 1;
  info->number_t = info->number_pages;

  if (meta->zDistance[0].den!=0 && meta->zDistance[0].num!=0) {
    double v = meta->zDistance[0].num / (double) meta->zDistance[0].den;
    if (v>0) {
      info->number_z = info->number_pages;
      info->number_t = 1;
    }
  }

  if (info->number_z > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_Z;
  }

  if (info->number_t > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_T;
  }

  if ((info->number_z > 1) && (info->number_t > 1)) {
    info->number_dims = 5;
    info->dimensions[3].dim = DIM_DIM_Z;        
    info->dimensions[4].dim = DIM_DIM_T;
  }

  return 0;
}

//----------------------------------------------------------------------------
// READ/WRITE FUNCTIONS
//----------------------------------------------------------------------------


DIM_UINT  stkReadPlane(DTiffParams *tiffParams, int plane, TDimImageBitmap *img, TDimFormatHandle *fmtHndl) {
  if (tiffParams == 0) return 1;
  if (img        == 0) return 1;
  if (tiffParams->dimTiff == 0) return 1;
  DIM_UINT sample = 0;

  TIFF *tif = tiffParams->dimTiff;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  uint16 compress_tag;
  uint16 bitspersample = 8;
  uint16 samplesperpixel = 1;
  uint32 height = 0; 
  uint32 width = 0;
  toff_t old_pos = tif->tif_curoff;
  uint32 rowsperstrip   = 1;

  (void) TIFFGetFieldDefaulted(tif, TIFFTAG_COMPRESSION, &compress_tag); 
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);  
  TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip); 


  DIM_UINT Bpp = (unsigned int) ceil( ((double) img->i.depth) / 8.0 );
  DIM_INT32 size = img->i.width * img->i.height * Bpp;

  D_TIFF_OFFS_TYPE file_plane_offset = tiffParams->stkInfo.strip_offsets[0] + (size * plane);

  if (compress_tag != COMPRESSION_NONE) 
    file_plane_offset = tiffParams->stkInfo.strip_offsets[plane * tiffParams->stkInfo.strips_per_plane ]; 



  // position file position into begining of the plane
  if (tif->tif_seekproc((thandle_t) tif->tif_fd, file_plane_offset, SEEK_SET) != 0) {
    
    //---------------------------------------------------
    // load the buffer with decompressed plane data
    //---------------------------------------------------
  
    if (compress_tag == COMPRESSION_NONE) {
      dimProgress( fmtHndl, 0, 10, "Reading STK" );
      
      int read_size = tif->tif_readproc((thandle_t) tif->tif_fd, img->bits[sample], size);
      if (read_size != size) return 1;
      // now swap bytes if needed
      tif->tif_postdecode(tif, (tidata_t) img->bits[sample], size); 
    } else {
      int strip_size = width * Bpp * samplesperpixel * rowsperstrip;
      DIM_UCHAR *buf = (DIM_UCHAR *) _TIFFmalloc( strip_size ); 

      // ---------------------------------------------------
      // let's tweak libtiff and change offsets and bytecounts for correct values for this plane
	    TIFFDirectory *td = &tif->tif_dir;

      D_TIFF_OFFS_TYPE *plane_strip_offsets = tiffParams->stkInfo.strip_offsets + (plane * tiffParams->stkInfo.strips_per_plane);
      _TIFFmemcpy( td->td_stripoffset, plane_strip_offsets, tiffParams->stkInfo.strips_per_plane * sizeof(D_TIFF_OFFS_TYPE) );

      D_TIFF_BCNT_TYPE *plane_strip_bytecounts = tiffParams->stkInfo.strip_bytecounts + (plane * tiffParams->stkInfo.strips_per_plane);
      _TIFFmemcpy( td->td_stripbytecount, plane_strip_bytecounts, tiffParams->stkInfo.strips_per_plane * sizeof(D_TIFF_BCNT_TYPE) );
      // ---------------------------------------------------

      int strip_num = plane * tiffParams->stkInfo.strips_per_plane;
      D_TIFF_OFFS_TYPE strip_offset = 0;
      while (strip_offset < tiffParams->stkInfo.strips_per_plane) {
        dimProgress( fmtHndl, strip_offset, tiffParams->stkInfo.strips_per_plane, "Reading STK" );
        if ( dimTestAbort( fmtHndl ) == 1) break;  

        int read_size = TIFFReadEncodedStrip (tif, strip_offset, (D_TIFF_DATA_TYPE) buf, (D_TIFF_SIZE_TYPE) strip_size);
        if (read_size == -1) return 1;
        _TIFFmemcpy( ((unsigned char *) img->bits[sample])+strip_offset*strip_size, buf, read_size );
        ++strip_offset;
      }

      _TIFFfree( buf );  
    }

    // invert it if we got negative
    TIFFGetField(tiffParams->dimTiff, TIFFTAG_PHOTOMETRIC, &photometric);
    if (photometric == PHOTOMETRIC_MINISWHITE)
      invertSample(img, sample);
  }

  tif->tif_seekproc((thandle_t) tif->tif_fd, old_pos, SEEK_SET);

  return 0;
}


//----------------------------------------------------------------------------
// METADATA TEXT FUNCTIONS
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// META DATA PROC
//----------------------------------------------------------------------------

DIM_UINT stkAddOneTag (TDimFormatHandle *fmtHndl, int tag, const char* str)
{
  DIM_UCHAR *buf = NULL;
  DIM_UINT32 buf_size = strlen(str);
  DIM_UINT32 buf_type = DIM_TAG_ASCII;

  if ( (buf_size == 0) || (str == NULL) ) return 1;
  else
  {
    // now add tag into structure
    TDimTagItem item;

    buf = (unsigned char *) DimMalloc(buf_size + 1);
    strncpy((char *) buf, str, buf_size);
    buf[buf_size] = '\0';

    item.tagGroup  = DIM_META_STK;
    item.tagId     = tag;
    item.tagType   = buf_type;
    item.tagLength = buf_size;
    item.tagData   = buf;

    addMetaTag( &fmtHndl->metaData, item);
  }

  return 0;
}


DIM_UINT stkReadMetaMeta (TDimFormatHandle *fmtHndl, int group, int tag, int type)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DTiffParams *tiffParams = (DTiffParams *) fmtHndl->internalParams;
  TDimStkMetaData *meta = &tiffParams->stkInfo.metaData;
  
  if ( (group != DIM_META_STK) && (group != -1) ) return 1;

  // add tag UIC2Tag 33629, some info
  std::string str_uic2 = "";
  char text[1024];

  for (int i=0; i<meta->N; i++) {
    unsigned short y;
    unsigned char m, d;
    unsigned char h, mi, s;

    JulianToYMD(meta->creationDate[i], y, m, d);
    MiliMidnightToHMS(meta->creationTime[i], h, mi, s);

    sprintf(text, "Page %.3d: %.4d-%.2d-%.2d %.2d:%.2d:%.2d\n", i, y, m, d, h, mi, s );
    str_uic2 += text;
  }
 
  if (str_uic2.size() > 0) 
    stkAddOneTag ( fmtHndl, 33629, str_uic2.c_str() );

  return 0;
}

//***********************************************************************************************
// new metadata support
//***********************************************************************************************

std::string stk_readline( const std::string &str, int &pos ) {
  std::string line;
  std::string::const_iterator it = str.begin() + pos;
  while (it<str.end() && *it != 0xA ) {
    if (*it != 0xD) 
      line += *it;
    else
      ++pos;
    ++it;
  }
  pos += line.size();
  if (it<str.end() && *it == 0xA) ++pos;
  return line;
}

template<typename T>
bool isValueRepeated( T *v, unsigned int N ) {
  if (!v) return true;
  if (N<=0) return true;
  T first = v[0];
  for (unsigned int i=1; i<N; ++i)
    if (v[i] != first) return false;
  return true;
}

template<>
bool isValueRepeated( TDimStkRational *v, unsigned int N ) {
  if (!v) return true;
  if (N<=0) return true;
  TDimStkRational first = v[0];
  for (unsigned int i=1; i<N; ++i)
    if (v[i].num != first.num || v[i].den != first.den) return false;
  return true;
}

void appendValues( TDimStkRational *v, unsigned int N, const std::string &tag_name, DTagMap *hash ) {
  xstring name, val;
  if ( isValueRepeated(v, N) ) N=1;
  for (int i=0; i<N; ++i)
    if (v[i].den!=0 && v[i].num!=0) {
      double vv = v[i].num / (double) v[i].den;
      if (N>1)
        name.sprintf( "%s_planes/%d", tag_name.c_str(), i );
      else
        name = tag_name;
      val.sprintf( "%f", vv );
      hash->append_tag( name, val );
      if (i==0) hash->append_tag( tag_name, val ); 
    }
}


DIM_UINT append_metadata_stk (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;
  DTiffParams *tiffpar = (DTiffParams *) fmtHndl->internalParams;
  TDimTiffIFD *ifd = &tiffpar->ifds.ifds[0];
  TDimStkMetaData *stk = &tiffpar->stkInfo.metaData;
  TDimImageInfo *info = &tiffpar->info;

  // is this STK?
  if (!isTagPresentInIFD(ifd, 33628)) return 0;

  xstring name, val;

  hash->append_tag( "image_num_z", (const int) info->number_z );
  hash->append_tag( "image_num_t", (const int) info->number_t );
  //hash->append_tag( "image_num_c", lsm->ch );

  hash->append_tag( "label", stk->Name );

  //---------------------------------------------------------------------------
  // Date/Time
  //---------------------------------------------------------------------------
  {
    unsigned short y;
    unsigned char m, d, h, mi, s;
    unsigned int NN = stk->N;
    if ( isValueRepeated<DIM_INT32>(stk->creationDate, stk->N) && isValueRepeated<DIM_INT32>(stk->creationTime, stk->N) ) NN=1;
    for (int i=0; i<stk->N; i++) {
      JulianToYMD(stk->creationDate[i], y, m, d);
      MiliMidnightToHMS(stk->creationTime[i], h, mi, s);
      if (NN>1) 
        name.sprintf( "date_time_planes/%d", i );
      else
        name = "date_time_plane";
      val.sprintf( "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", y, m, d, h, mi, s );
      hash->append_tag( name, val );
    }
  }

  //---------------------------------------------------------------------------
  // get the pixel size in microns
  //---------------------------------------------------------------------------
  double pixel_size[4] = {0,0,0,0};

  name = stk->CalibrationUnits;
  if (stk->SpatialCalibration>0 && (name == "um" || name == "microns") ) {
    if (stk->XCalibration[1]!=0 && stk->XCalibration[0]!=0) 
      pixel_size[0] = stk->XCalibration[0] / (double) stk->XCalibration[1];
    if (stk->YCalibration[1]!=0 && stk->YCalibration[0]!=0) 
      pixel_size[1] = stk->YCalibration[0] / (double) stk->YCalibration[1];

    hash->append_tag( "pixel_resolution_x", pixel_size[0] );
    hash->append_tag( "pixel_resolution_y", pixel_size[1] );
    hash->append_tag( "pixel_resolution_unit_x", "microns" );
    hash->append_tag( "pixel_resolution_unit_y", "microns" );
  }

  //---------------------------------------------------------------------------
  // add 3'd dimension
  //---------------------------------------------------------------------------
  if (info->number_z > 1) {
    if (stk->zDistance[0].den!=0 && stk->zDistance[0].num!=0)
      pixel_size[2] = stk->zDistance[0].num / (double) stk->zDistance[0].den;
    hash->append_tag( "pixel_resolution_z", pixel_size[2] );
    hash->append_tag( "pixel_resolution_unit_z", "microns" );
  } else {
    hash->append_tag( "pixel_resolution_unit_t", "seconds" );    
    if (stk->N>=2) {
      unsigned char h1, m1, s1, h2, m2, s2;
      MiliMidnightToHMS(stk->creationTime[0], h1, m1, s1);
      MiliMidnightToHMS(stk->creationTime[1], h2, m2, s2);
      double delta_s = ( fabs((float)h2-h1)*60.0 + fabs((float)m2-m1))*60.0 + fabs((float)s2-s1);
      hash->append_tag( "pixel_resolution_t", delta_s );
    }
  }

  //------------------------------------------------------------
  // Long arrays
  //------------------------------------------------------------
  appendValues( stk->StagePositionX, stk->N, "stage_position_x", hash );
  appendValues( stk->StagePositionY, stk->N, "stage_position_y", hash );
  appendValues( stk->zDistance, stk->N, "stage_distance_z", hash );
  appendValues( stk->CameraChipOffsetX, stk->N, "camera_sensor_x", hash );
  appendValues( stk->CameraChipOffsetY, stk->N, "camera_sensor_y", hash );

  if (info->number_z>1 && stk->AbsoluteZValid && stk->AbsoluteZValid[0]) 
    appendValues( stk->AbsoluteZ, stk->N, "stage_position_z", hash );

  //------------------------------------------------------------
  // Add tags from structure
  //------------------------------------------------------------

  hash->append_tag( "custom/Name", stk->Name );
  hash->append_tag( "custom/AutoScale", (const int) stk->AutoScale );
  hash->append_tag( "custom/MinScale", (const int) stk->MinScale );
  hash->append_tag( "custom/MaxScale", (const int) stk->MaxScale );

  hash->append_tag( "custom/SpatialCalibration", (const int) stk->SpatialCalibration );
  hash->append_tag( "custom/XCalibration", xstring::xprintf("%d/%d", stk->XCalibration[0], stk->XCalibration[1]) );
  hash->append_tag( "custom/YCalibration", xstring::xprintf("%d/%d", stk->YCalibration[0], stk->YCalibration[1]) );
  if (stk->CalibrationUnits) hash->append_tag( "custom/CalibrationUnits", stk->CalibrationUnits );


  hash->append_tag( "custom/ThreshState", (const int) stk->ThreshState );
  hash->append_tag( "custom/ThreshStateRed", (const unsigned int) stk->ThreshStateRed );
  hash->append_tag( "custom/ThreshStateGreen", (const unsigned int) stk->ThreshStateGreen );
  hash->append_tag( "custom/ThreshStateBlue", (const unsigned int) stk->ThreshStateBlue );
  hash->append_tag( "custom/ThreshStateLo", (const unsigned int) stk->ThreshStateLo );
  hash->append_tag( "custom/ThreshStateHi", (const unsigned int) stk->ThreshStateHi );
  hash->append_tag( "custom/Zoom", (const int) stk->Zoom );

  hash->append_tag( "custom/CreateTime", StkDateTimeToAnsi(stk->CreateTime[0], stk->CreateTime[1]) );
  hash->append_tag( "custom/LastSavedTime", StkDateTimeToAnsi(stk->LastSavedTime[0], stk->LastSavedTime[1]) );
  
  hash->append_tag( "custom/currentBuffer", (const int) stk->currentBuffer );
  hash->append_tag( "custom/grayFit", (const int) stk->grayFit );
  hash->append_tag( "custom/grayPointCount", (const int) stk->grayPointCount );
  hash->append_tag( "custom/grayX", xstring::xprintf("%d/%d", stk->grayX[0], stk->grayX[1]) );
  hash->append_tag( "custom/grayY", xstring::xprintf("%d/%d", stk->grayY[0], stk->grayY[1]) );
  hash->append_tag( "custom/grayMin", xstring::xprintf("%d/%d", stk->grayMin[0], stk->grayMin[1]) );
  hash->append_tag( "custom/grayMax", xstring::xprintf("%d/%d", stk->grayMax[0], stk->grayMax[1]) );
  if (stk->grayUnitName) hash->append_tag( "custom/grayUnitName", stk->grayUnitName );
  hash->append_tag( "custom/StandardLUT", (const int) stk->StandardLUT );

  hash->append_tag( "custom/AutoScaleLoInfo", xstring::xprintf("%d/%d", stk->AutoScaleLoInfo[0], stk->AutoScaleLoInfo[1]) );
  hash->append_tag( "custom/AutoScaleHiInfo", xstring::xprintf("%d/%d", stk->AutoScaleHiInfo[0], stk->AutoScaleHiInfo[1]) );
  hash->append_tag( "custom/Gamma", (const int) stk->Gamma );
  hash->append_tag( "custom/GammaRed", (const int) stk->GammaRed );
  hash->append_tag( "custom/GammaGreen", (const int) stk->GammaGreen );
  hash->append_tag( "custom/GammaBlue", (const int) stk->GammaBlue );
  
  /*
  // begin: Used internally by MetaMorph
  DIM_INT32 OverlayMask;
  DIM_INT32 OverlayCompress;
  DIM_INT32 Overlay;
  DIM_INT32 SpecialOverlayMask;
  DIM_INT32 SpecialOverlayCompress;
  DIM_INT32 SpecialOverlay;
  DIM_INT32 ImageProperty;
  // end: Used internally by MetaMorph
*/

  //------------------------------------------------------------
  // Add tags from tiff
  //------------------------------------------------------------
  TIFF *tif = tiffpar->dimTiff;
  xstring tag_305 = read_tag_as_string(tif, ifd, TIFFTAG_SOFTWARE );
  hash->append_tag( "custom/Software", tag_305 );
  //xstring tag_306 = read_tag_as_string(tif, ifd, TIFFTAG_DATETIME );

  xstring tag_270 = read_tag_as_string(tif, ifd, TIFFTAG_IMAGEDESCRIPTION );
  hash->parse_ini( tag_270, ":", "custom" );

  int p=0;
  xstring line = stk_readline( tag_270, p );
  if (line.startsWith("Acquired from ")) {
    hash->append_tag( "custom/AcquiredFrom", line.right("Acquired from ") );
  }


  return 0;
}



