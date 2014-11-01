/*****************************************************************************
  TIFF PSIA IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation
    10/10/2005 16:23 - image allocation fixed
        
  Ver : 2
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "memio.h"
#include "dim_tiff_format.h"

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

//----------------------------------------------------------------------------
// PSIA MISC FUNCTIONS
//----------------------------------------------------------------------------

bool psiaIsTiffValid(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return FALSE;
  if (tiffParams->dimTiff->tif_flags&TIFF_BIGTIFF) return false;   
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 50434 ) == TRUE) return TRUE;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 50435 ) == TRUE) return TRUE;
  return FALSE;
}

void wstr2charcpy (char *trg, char *src, unsigned int n) {
  unsigned int i2=0;
  for (unsigned int i=0; i<n; i++) {
    trg[i] = src[i2];
    i2+=2;
  }
}

int psiaGetInfo (DTiffParams *tiffParams) {
  if (tiffParams == NULL) return 1;
  if (tiffParams->dimTiff == NULL) return 1;
  if (tiffParams->ifds.count <= 0) return 1;

  DIM_UCHAR *buf = NULL;
  uint32 size, type;
  psiaInfoHeader *psiaInfo = &tiffParams->psiaInfo;

  if (!isTagPresentInFirstIFD( &tiffParams->ifds, 50435 )) return 1;

  readTiffTag (tiffParams->dimTiff, &tiffParams->ifds.ifds[0], 50435, size, type, &buf);
  if ( (size <= 0) || (buf == NULL) ) return 1;

  psiaInfo->dfLPFStrength = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_LPFSSTRENGTH); 
  psiaInfo->bAutoFlatten  = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_AUTOFLATTEN);      
  psiaInfo->bACTrack      = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_ACTRACK);          
  psiaInfo->nWidth        = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_WIDTH);            
  psiaInfo->nHeight       = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_HEIGHT);           
  psiaInfo->dfAngle       = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_ANGLE);
  psiaInfo->bSineScan     = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_SINESCAN);         
  psiaInfo->dfOverScan    = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_OVERSCAN);        
  psiaInfo->bFastScanDir  = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_FASTSCANDIR);      
  psiaInfo->bSlowScanDir  = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_SLOWSCANDIR);      
  psiaInfo->bXYSwap       = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_XYSWAP);           
  psiaInfo->dfXScanSize   = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_XSCANSIZE);       
  psiaInfo->dfYScanSize   = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_YSCANSIZE);
  psiaInfo->dfXOffset     = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_XOFFSET);         
  psiaInfo->dfYOffset     = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_YOFFSET);
  psiaInfo->dfScanRate    = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_SCANRATE);        
  psiaInfo->dfSetPoint    = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_SETPOINT);        
  psiaInfo->dtTipBias     = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_TIPBIAS);         
  psiaInfo->dfSampleBias  = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_SAMPLEBIAS);      
  psiaInfo->dfDataGain    = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_DATAGAIN);        
  psiaInfo->dfZScale      = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_ZSCALE);          
  psiaInfo->dfZOffset     = * (DIM_FLOAT64 *) (buf + DIM_PSIA_OFFSET_ZOFFSET);         
  psiaInfo->nDataMin      = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_DATAMIN);
  psiaInfo->nDataMax      = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_DATAMAX);
  psiaInfo->nDataAvg      = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_DATAAVG);
  psiaInfo->ncompression  = * (DIM_UINT32 *)  (buf + DIM_PSIA_OFFSET_NCOMPRESSION);

  // if running the MSB machine (motorola, power pc) then swap
  if (bigendian) {
    TIFFSwabDouble ( &psiaInfo->dfLPFStrength );  
    TIFFSwabLong   ( (uint32 *) &psiaInfo->bAutoFlatten );
    TIFFSwabLong   ( (uint32 *) &psiaInfo->bACTrack );
    TIFFSwabLong   ( (uint32 *) &psiaInfo->nWidth );
    TIFFSwabLong   ( (uint32 *) &psiaInfo->nHeight );
    TIFFSwabDouble ( &psiaInfo->dfAngle );  
    TIFFSwabLong   ( (uint32 *) &psiaInfo->bSineScan );
    TIFFSwabDouble ( &psiaInfo->dfOverScan );  
    TIFFSwabLong   ( (uint32 *) &psiaInfo->bFastScanDir ); 
    TIFFSwabLong   ( (uint32 *) &psiaInfo->bSlowScanDir );
    TIFFSwabLong   ( (uint32 *) &psiaInfo->bXYSwap );  
    TIFFSwabDouble ( &psiaInfo->dfXScanSize ); 
    TIFFSwabDouble ( &psiaInfo->dfYScanSize ); 
    TIFFSwabDouble ( &psiaInfo->dfXOffset ); 
    TIFFSwabDouble ( &psiaInfo->dfYOffset );   
    TIFFSwabDouble ( &psiaInfo->dfScanRate );
    TIFFSwabDouble ( &psiaInfo->dfSetPoint ); 
    TIFFSwabDouble ( &psiaInfo->dtTipBias ); 
    TIFFSwabDouble ( &psiaInfo->dfSampleBias );   
    TIFFSwabDouble ( &psiaInfo->dfZScale ); 
    TIFFSwabDouble ( &psiaInfo->dfZOffset ); 
    TIFFSwabLong   ( (uint32 *) &psiaInfo->nDataMin );
    TIFFSwabLong   ( (uint32 *) &psiaInfo->nDataMax );
    TIFFSwabLong   ( (uint32 *) &psiaInfo->nDataAvg );
    TIFFSwabLong   ( (uint32 *) &psiaInfo->ncompression );
  }

  wstr2charcpy (psiaInfo->szSourceNameW,   (char *) (buf + DIM_PSIA_OFFSET_SOURCENAME), 32);
  wstr2charcpy (psiaInfo->szImageModeW,    (char *) (buf + DIM_PSIA_OFFSET_IMAGEMODE), 8);
  wstr2charcpy (psiaInfo->szSetPointUnitW, (char *) (buf + DIM_PSIA_OFFSET_SETPOINTUNIT), 8);
  wstr2charcpy (psiaInfo->szUnitW,         (char *) (buf + DIM_PSIA_OFFSET_UNIT), 8);

  freeTiffTagBuf( &buf );

  return 0;
}

void psiaGetCurrentPageInfo(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return;
  TDimImageInfo *info = &tiffParams->info;
  if ( tiffParams->subType != tstPsia ) return;

  psiaInfoHeader *meta = &tiffParams->psiaInfo;
  info->resUnits = DIM_RES_um;
  info->xRes = meta->dfXScanSize / meta->nWidth;
  info->yRes = meta->dfYScanSize / meta->nHeight;

  info->depth     = 16;
  info->pixelType = D_FMT_UNSIGNED;
  info->samples   = 1;
  info->width     = tiffParams->psiaInfo.nWidth;
  info->height    = tiffParams->psiaInfo.nHeight;
}

//----------------------------------------------------------------------------
// READ/WRITE FUNCTIONS
//----------------------------------------------------------------------------

DIM_UINT psiaReadPlane(TDimFormatHandle *fmtHndl, DTiffParams *tiffParams, int plane, TDimImageBitmap *img) {
  if (tiffParams == 0) return 1;
  if (img        == 0) return 1;
  if (tiffParams->dimTiff == 0) return 1;

  DIM_UINT sample = 0;
  DIM_UCHAR *buf = NULL;
  uint32 size, type;
  register unsigned int y;
  DIM_UCHAR *p, *p2;

  img->i.depth     = 16;
  img->i.pixelType = D_FMT_UNSIGNED;
  img->i.samples   = 1;
  img->i.width     = tiffParams->psiaInfo.nWidth;
  img->i.height    = tiffParams->psiaInfo.nHeight;
  if ( allocImg( fmtHndl, &img->i, img) != 0 ) return 1;

  //--------------------------------------------------------------------
  // read actual image 
  //--------------------------------------------------------------------
  size = getImgSizeInBytes( img );
  readTiffTag (tiffParams->dimTiff, &tiffParams->ifds.ifds[0], 50434, size, type, &buf);
  if ( (size <= 0) || (buf == NULL) ) return 1;

  DIM_UINT32 line_size = img->i.width*2;
  p = ((DIM_UCHAR *) img->bits[0]) + (line_size * (img->i.height-1) );
  p2 = buf;
  for (y=0; y<img->i.height; y++ ) {
    dimProgress( fmtHndl, y, img->i.height, "Reading PSIA" );
    if ( dimTestAbort( fmtHndl ) == 1) break;  

    _TIFFmemcpy(p, p2, line_size);
    p  -= line_size;
    p2 += line_size;
  }

  if (bigendian) //if (swabflag) 
    TIFFSwabArrayOfShort( (DIM_UINT16 *) img->bits[0], size/2 );
  freeTiffTagBuf( &buf );

  // psia data is stored inverted
  invertImg( img );

  return 0;
}

//----------------------------------------------------------------------------
// METADATA FUNCTIONS
//----------------------------------------------------------------------------

DIM_UINT append_metadata_psia (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;
  DTiffParams *tiffParams = (DTiffParams *) fmtHndl->internalParams;
  if (tiffParams->subType != tstPsia) return 1; 
  psiaInfoHeader *meta = &tiffParams->psiaInfo;

  std::map< int, std::string > psia_vals;
  psia_vals[0] = "Off";
  psia_vals[1] = "On";

  hash->append_tag( "custom/Source", meta->szSourceNameW );
  hash->append_tag( "custom/Head Mode", meta->szImageModeW );
  hash->append_tag( "custom/Low Pass Filter", meta->dfLPFStrength );
  hash->append_tag( "custom/Auto Flatten", psia_vals[meta->bAutoFlatten] );
  hash->append_tag( "custom/AC Track", psia_vals[meta->bACTrack] );
  hash->append_tag( "custom/Data Width", xstring::xprintf("%d (pixels)", meta->nWidth) );
  hash->append_tag( "custom/Data Height", xstring::xprintf("%d (pixels)", meta->nHeight) );
  hash->append_tag( "custom/Rotation", xstring::xprintf("%.2f (deg)", meta->dfAngle) );
  hash->append_tag( "custom/Sine Scan", psia_vals[meta->bSineScan] );
  hash->append_tag( "custom/Over Scan", xstring::xprintf("%.2f (%%)", meta->dfOverScan) );

  if (meta->bFastScanDir == 0)    
    hash->append_tag( "custom/Fast Scan Dir", "Right to Left" );
  else
    hash->append_tag( "custom/Fast Scan Dir", "Left to Right" );

  if (meta->bSlowScanDir == 0)    
    hash->append_tag( "custom/Slow Scan Dir", "Top to Bototm" );
  else
    hash->append_tag( "custom/Slow Scan Dir", "Bottom to Top" );

  if (meta->bXYSwap == 0)    
    hash->append_tag( "custom/Fast Scan Axis", "X" );
  else
    hash->append_tag( "custom/Fast Scan Axis", "Y" );

  hash->append_tag( "custom/X Scan Size", xstring::xprintf("%.2f (%s)", meta->dfXScanSize, meta->szUnitW) );
  hash->append_tag( "custom/Y Scan Size", xstring::xprintf("%.2f (%s)", meta->dfYScanSize, meta->szUnitW) );
  hash->append_tag( "custom/X Scan Offset", xstring::xprintf("%.2f (%s)", meta->dfXOffset, meta->szUnitW) );
  hash->append_tag( "custom/Y Scan Offset", xstring::xprintf("%.2f (%s)", meta->dfYOffset, meta->szUnitW) );
  hash->append_tag( "custom/Scan Rate", xstring::xprintf("%.2f (Hz)", meta->dfScanRate) );
  hash->append_tag( "custom/Set Point", xstring::xprintf("%.4f (%s)", meta->dfSetPoint, meta->szSetPointUnitW) );
  hash->append_tag( "custom/Tip Bias", xstring::xprintf("%.2f (V)", meta->dtTipBias) );
  hash->append_tag( "custom/Sample Bias", xstring::xprintf("%.2f (V)", meta->dfSampleBias) );
  hash->append_tag( "custom/Data Gain", xstring::xprintf("%.4E (%s/step)", meta->dfDataGain, meta->szUnitW) );

  //  double dval = meta->nDataMin * meta->dfDataGain;
  hash->append_tag( "custom/Data Min", xstring::xprintf("%.4G (%s)", meta->nDataMin * meta->dfDataGain, meta->szUnitW) );
  hash->append_tag( "custom/Data Max", xstring::xprintf("%.4G (%s)", meta->nDataMax * meta->dfDataGain, meta->szUnitW) );
  hash->append_tag( "custom/Data Avg", (const int) meta->nDataAvg );
  hash->append_tag( "custom/Z Scale", meta->dfZScale );
  hash->append_tag( "custom/Z Offset", meta->dfZOffset );
  hash->append_tag( "custom/NCompression", (const int) meta->ncompression );


  TIFF *tif = tiffParams->dimTiff;
  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];
  xstring psia_comments = read_tag_as_string(tif, ifd, 50436);
  if (psia_comments.size()>0)
    hash->append_tag( "custom/Comments", psia_comments );

  return 0;
}


















