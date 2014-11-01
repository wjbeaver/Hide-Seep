/*****************************************************************************
  TIFF FLUOVIEW IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation
        
  Ver : 1
*****************************************************************************/

#include <xstring.h>
#include "dim_tiff_format.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <tag_map.h>

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

void change_0_to_n (char *str, long size);

void read_text_tag(TIFF *tif, TDimTiffIFD *ifd, DIM_UINT tag, MemIOBuf *outIOBuf);

//----------------------------------------------------------------------------
// PSIA MISC FUNCTIONS
//----------------------------------------------------------------------------

bool fluoviewIsTiffValid(TIFF *tif) {
  if (tif->tif_flags&TIFF_BIGTIFF) return false;        
  char *b_list = NULL;
  int16   d_list_count;
  int res[3] = {0,0,0};

  if (tif == 0) return FALSE;

  res[0] = TIFFGetField(tif, 34361, &d_list_count, &b_list);
  res[1] = TIFFGetField(tif, 34362, &d_list_count, &b_list);
  res[2] = TIFFGetField(tif, 34386, &d_list_count, &b_list);

  if (res[0] == 1) return TRUE;
  if (res[1] == 1) return TRUE;
  if (res[2] == 1) return TRUE;

  return FALSE;
}


bool fluoviewIsTiffValid(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return FALSE;
  if (tiffParams->dimTiff->tif_flags&TIFF_BIGTIFF) return false;    
  // if tag 33629 exists then the file is valid STAK file
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 34361 ) == TRUE) return TRUE;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 34362 ) == TRUE) return TRUE;
  if (isTagPresentInFirstIFD( &tiffParams->ifds, 34386 ) == TRUE) return TRUE;

  return FALSE;
}

void doSwabMMHEAD(MM_DIM_INFO *mmdi)
{
  TIFFSwabLong   ((uint32*) &mmdi->Size);
  TIFFSwabDouble ((double*) &mmdi->Origin);
  TIFFSwabDouble ((double*) &mmdi->Resolution);
}

void doSwabMMHEAD(MM_HEAD *mmh)
{
  TIFFSwabShort((uint16*) &mmh->HeaderFlag);
  TIFFSwabLong ((uint32*) &mmh->Data);     
  TIFFSwabLong ((uint32*) &mmh->NumberOfColors);
  TIFFSwabLong ((uint32*) &mmh->MM_256_Colors);  
  TIFFSwabLong ((uint32*) &mmh->MM_All_Colors);    
  TIFFSwabLong ((uint32*) &mmh->CommentSize); 
  TIFFSwabLong ((uint32*) &mmh->Comment);     
  TIFFSwabLong ((uint32*) &mmh->SpatialPosition); 
  TIFFSwabShort((uint16*) &mmh->MapType);  
  TIFFSwabLong ((uint32*) &mmh->MapMin); 
  TIFFSwabLong ((uint32*) &mmh->MapMax);  
  TIFFSwabLong ((uint32*) &mmh->MinValue); 
  TIFFSwabLong ((uint32*) &mmh->MaxValue); 
  TIFFSwabLong ((uint32*) &mmh->Map);
  TIFFSwabLong ((uint32*) &mmh->Gamma);
  TIFFSwabLong ((uint32*) &mmh->Offset);
  TIFFSwabLong ((uint32*) &mmh->ThumbNail);
  TIFFSwabLong ((uint32*) &mmh->UserFieldSize);
  TIFFSwabLong ((uint32*) &mmh->UserFieldHandle);

  doSwabMMHEAD(&mmh->Gray);
  for (int i=0; i<SPATIAL_DIMENSION; ++i)
    doSwabMMHEAD(&mmh->DimInfo[i]);
}

void printMM_DIM_INFO(MM_DIM_INFO *mmdi)
{
  printf( "\nMM_DIM_INFO Name: %s\n", mmdi->Name);
  printf( "MM_DIM_INFO size: %d\n", mmdi->Size);
  printf( "MM_DIM_INFO origin: %f\n", mmdi->Origin);
  printf( "MM_DIM_INFO resolution: %f\n", mmdi->Resolution);
  printf( "MM_DIM_INFO Units: %s\n", mmdi->Units);
}

void printMMHEADER(MM_HEAD *mmh)
{
  printf( "\nMMHEAD HeaderFlag: %.4X\n", mmh->HeaderFlag);
  printf( "\nMMHEAD ImageType: %.1X\n", mmh->ImageType);
  printf( "\nMMHEAD Name: %s\n", mmh->Name);
  printf( "\nMMHEAD Status: %.1X\n", mmh->Status);
  printf( "\nMMHEAD Data: %.8X\n", mmh->Data);
  printf( "\nMMHEAD NumberOfColors: %.8X\n", mmh->NumberOfColors);
  printf( "\nMMHEAD MM_256_Colors: %.8X\n", mmh->MM_256_Colors);
  printf( "\nMMHEAD MM_All_Colors: %.8X\n", mmh->MM_All_Colors);
  printf( "\nMMHEAD CommentSize: %.8X\n", mmh->CommentSize);
  printf( "\nMMHEAD Comment: %.8X\n", mmh->Comment);

  for (int i=0; i<SPATIAL_DIMENSION; ++i)
    printMM_DIM_INFO(&mmh->DimInfo[i]);
}

int fluoviewGetInfo (DTiffParams *tiffParams)
{
  if (tiffParams == NULL) return 1;
  if (tiffParams->dimTiff == NULL) return 1;
  if (tiffParams->ifds.count <= 0) return 1;

  TDimImageInfo *info = &tiffParams->info;
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;
  DIM_UCHAR *buf = NULL;
  uint32 size, type;
  MM_HEAD *fvInfo;

  if (!isTagPresentInFirstIFD( &tiffParams->ifds, MMHEADER )) return 1;

  readTiffTag (tiffParams->dimTiff, &tiffParams->ifds.ifds[0], MMHEADER, size, type, &buf);
  if ( (size <= 0) || (buf == NULL) ) return 1;

  if (bigendian) doSwabMMHEAD((MM_HEAD *) buf);

  fvi->head = * (MM_HEAD *) buf;
  freeTiffTagBuf( &buf );
  fvInfo = &fvi->head;

  fvi->ch = 1;
  fvi->pages = 1;
  fvi->t_frames = 1;
  fvi->z_slices = 1;

  //---------------------------------------------------------------
  // retreive dimension parameters
  // Typical dimension names include "X", "Y", "Z", "T", "Ch", "Ani"
  // Fluoview order is: XYZTC -> we obtain XYCZT
  //---------------------------------------------------------------
  int d; 
  for (d=0; d<SPATIAL_DIMENSION; d++)
  {
    if ( strncmp(fvInfo->DimInfo[d].Name, "Ch", 2 ) == 0 )
      fvi->ch = fvInfo->DimInfo[d].Size; 
    else
    if ( strncmp(fvInfo->DimInfo[d].Name, "T", 1 ) == 0 )
      fvi->t_frames = fvInfo->DimInfo[d].Size; 
    else
    if ( strncmp(fvInfo->DimInfo[d].Name, "Z", 1 ) == 0 )
    {
      fvi->z_slices = fvInfo->DimInfo[d].Size; 
      fvi->zR = fvInfo->DimInfo[d].Resolution;
    }
    else
    if ( strncmp(fvInfo->DimInfo[d].Name, "X", 1 ) == 0 )
      fvi->xR = fvInfo->DimInfo[d].Resolution; 
    else
    if ( strncmp(fvInfo->DimInfo[d].Name, "Y", 1 ) == 0 )
      fvi->yR = fvInfo->DimInfo[d].Resolution; 
  }

  fvi->pages_tiff = tiffParams->info.number_pages;
  fvi->pages = tiffParams->info.number_pages / fvi->ch;
  tiffParams->info.number_pages = fvi->pages;

  tiffParams->info.samples = fvi->ch;
  if (tiffParams->info.samples > 1) 
    tiffParams->info.imageMode = DIM_RGB;
  else
    tiffParams->info.imageMode = DIM_GRAYSCALE;    

  uint16 bitspersample = 1;  
  TIFFGetField(tiffParams->dimTiff, TIFFTAG_BITSPERSAMPLE, &bitspersample);  
  tiffParams->info.depth = bitspersample;

  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];
  TIFF *tif = tiffParams->dimTiff;

  //--------------------------------------------------------------------  
  // get channel names
  //--------------------------------------------------------------------
  fvi->sample_names.resize( fvi->ch );
  for (int i=0; i<fvi->ch; i++) 
    fvi->sample_names[i] = "unknown";

  if (isTagPresentInIFD ( ifd, 270 ) == TRUE ) {
    uint32 buf_size, buf_type;
    DIM_UCHAR *buf = NULL;
    readTiffTag (tif, ifd, 270, buf_size, buf_type, &buf);
    change_0_to_n ((char *) buf, buf_size);

    xstring search_str;
    char *line;
    char str[1024];

    // Extract channel names
    for (int i=0; i<fvi->ch; i++) {
      search_str.sprintf( "Channel %d Dye=", i+1 );
      line = strstr( (char *) buf, search_str.c_str() );
      if (line) {
        search_str.sprintf( "Channel %d Dye=%%s\n", i+1 );
        sscanf( line, search_str.c_str(), str );
        fvi->sample_names[i] = str;
      }
    }

    // Get desired channel to display mapping
    fvi->display_lut.resize(3, -1);
    for (int i=0; i< std::min<int>(3,fvi->ch); ++i) 
      fvi->display_lut[i] = i;
   
    int r,g,b, chOut;

    for (int i=0; i<fvi->ch; ++i) {
      chOut = -1;
      search_str.sprintf( "[LUT Ch%d]", i );
      line = strstr( (char *) buf, search_str.c_str() );
      if (line) {
        line = strstr( line, "RGB 255=" );    
        sscanf( line, "RGB 255=%d\t%d\t%d", &r, &g, &b );
        if ( r>g && r>b ) chOut = 0;
        else
        if ( g>r && g>b ) chOut = 1;
        else
        if ( b>g && b>r ) chOut = 2;
      }
      if (chOut>=0) fvi->display_lut[chOut] = i;
    }

    freeTiffTagBuf( (DIM_UCHAR **) &buf );
  }

  //---------------------------------------------------------------
  // define dims
  //---------------------------------------------------------------

  info->number_z = fvi->z_slices;
  info->number_t = fvi->t_frames;

  if (fvi->z_slices > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_Z;
  }

  if (fvi->t_frames > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_T;
  }

  if ((fvi->z_slices > 1) && (fvi->t_frames > 1)) {
    info->number_dims = 5;
    info->dimensions[3].dim = DIM_DIM_Z;        
    info->dimensions[4].dim = DIM_DIM_T;
  }


  return 0;
}

// in fluoview luts are saved as INI text in tag: 270 - Image Description
void fluoviewInitPalette( DTiffParams *tiffParams, TDimImageInfo *info )
{
  uint32 buf_size;
  uint32 buf_type;  
  DIM_UCHAR *buf = NULL;
  TIFF *tif = tiffParams->dimTiff;
  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];

  if (tif == NULL) return;
  if (info == NULL) return;

  if (tiffParams->subType != tstFluoview) return; 
  if (tiffParams->fluoviewInfo.ch > 1) return;
  if (isTagPresentInIFD ( ifd, 270 ) != TRUE ) return;

  readTiffTag (tif, ifd, 270, buf_size, buf_type, &buf);
  change_0_to_n ((char *) buf, buf_size);


  int i=0;
  char *line, str[100];
  int r,g,b;

  info->lut.count = 0;
  for (i=0; i<256; i++) info->lut.rgba[i] = i*256;

  line = strstr( (char *) buf, "[LUT Ch0]" );

  if (line != NULL) 
  for (i=0; i<256; i++)
  {
    sprintf(str, "RGB %d=", i);
    line = strstr( line, str );    
    
    sprintf(str, "RGB %d=%%d\t%%d\t%%d\n", i);
    sscanf( line, str, &r, &g, &b );

    info->lut.rgba[i] = dimRGB( r, g, b );
  }
  info->lut.count = 256;

  freeTiffTagBuf( (DIM_UCHAR **) &buf );
}


void fluoviewGetCurrentPageInfo(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return;
  TDimImageInfo *info = &tiffParams->info;

  if ( tiffParams->subType == tstFluoview ) {
    fluoviewInitPalette( tiffParams, info);
    DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;    
    info->resUnits = DIM_RES_um;
    info->xRes = fvi->xR;
    info->yRes = fvi->yR;
  }
}

//----------------------------------------------------------------------------
// READ/WRITE FUNCTIONS
//----------------------------------------------------------------------------

DIM_UINT fluoviewReadPlane( TDimFormatHandle *fmtHndl, DTiffParams *tiffParams, int plane )
{
  if (tiffParams == 0) return 1;
  if (tiffParams->dimTiff == 0) return 1;
  if (tiffParams->subType != tstFluoview) return 1;  

  uint16 bitspersample = 1;
  
  TIFF *tif = tiffParams->dimTiff;  
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;  
  TDimImageInfo *ii = &tiffParams->info;

  if (tif == NULL) return 1;
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

  ii->samples = fvi->ch;
  //ii->imageMode = DIM_GRAYSCALE;    
  
  ii->depth = bitspersample;
  if (bitspersample == 16) ii->pixelType = D_FMT_UNSIGNED;

  unsigned int sample;
  TDimImageBitmap *img = fmtHndl->image;
  if ( allocImg( fmtHndl, ii, img) != 0 ) return 1;

  //--------------------------------------------------------------------
  // read data
  //--------------------------------------------------------------------
  DIM_UINT lineSize = getLineSizeInBytes( img );

  for (sample=0; sample<(unsigned int)fvi->ch; ++sample) {
    // switch to correct page in original TIFF
    int dirNum = fmtHndl->pageNumber + sample * ( fvi->pages_tiff / fvi->ch );
    TIFFSetDirectory( tif, dirNum );

    if( TIFFIsTiled(tif) ) continue;
    DIM_UCHAR *p = (DIM_UCHAR *) img->bits[ sample ];
    register DIM_UINT y = 0;

    for(y=0; y<img->i.height; y++) {
      dimProgress( fmtHndl, y*(sample+1), img->i.height*fvi->ch, "Reading FLUOVIEW" );
      if ( dimTestAbort( fmtHndl ) == 1) break;  

      TIFFReadScanline(tif, p, y, 0);
      p += lineSize;
    } // for y
  }  // for sample

  TIFFSetDirectory(tif, fmtHndl->pageNumber);
  return 0;
}

//----------------------------------------------------------------------------
// Metadata hash
//----------------------------------------------------------------------------

DIM_UINT append_metadata_fluoview (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;

  DTiffParams *tiffParams = (DTiffParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &tiffParams->info;
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;
  MM_HEAD *fvInfo = &fvi->head;


  hash->append_tag( "image_num_z", fvi->z_slices );
  hash->append_tag( "image_num_t", fvi->t_frames );
  hash->append_tag( "image_num_c", fvi->ch );

  //----------------------------------------------------------------------------
  // DIMENSIONS
  //----------------------------------------------------------------------------

  for (int i=0; i<SPATIAL_DIMENSION; i++) {
    xstring name  = fvInfo->DimInfo[i].Name;
    name = name.removeSpacesBoth(); 
    double res   = fvInfo->DimInfo[i].Resolution;
    xstring units = fvInfo->DimInfo[i].Units;
    units = units.removeSpacesBoth(); 
    if (units ==  "um") units = "microns";
    if (units ==  "µm") units = "microns";
    if (units ==  "s") units = "seconds";

    if (name == "X") {
      hash->append_tag( "pixel_resolution_x", res );
      hash->append_tag( "pixel_resolution_unit_x", units );
    } else
    if (name == "Y") {
      hash->append_tag( "pixel_resolution_y", res );
      hash->append_tag( "pixel_resolution_unit_y", units );
    } else
    if (name == "Z") {
      hash->append_tag( "pixel_resolution_z", res );
      hash->append_tag( "pixel_resolution_unit_z", units );
    } else
    if (name == "T") {
      hash->append_tag( "pixel_resolution_t", res );
      hash->append_tag( "pixel_resolution_unit_t", units );
    }
  }  

  //----------------------------------------------------------------------------
  // Channel names and preferred mapping
  //----------------------------------------------------------------------------
  for (unsigned int i=0; i<fvi->sample_names.size(); ++i) {
    xstring tag_name;
    tag_name.sprintf( "channel_%d_name", i );
    hash->append_tag( tag_name, fvi->sample_names[i] );
  }

  if ( fvi->display_lut.size() == 3 ) {
    hash->append_tag( "display_channel_red",   fvi->display_lut[0] );
    hash->append_tag( "display_channel_green", fvi->display_lut[1] );
    hash->append_tag( "display_channel_blue",  fvi->display_lut[2] );
  }

  //----------------------------------------------------------------------------
  // Additional info from Tag 270
  //----------------------------------------------------------------------------

  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];
  TIFF *tif = tiffParams->dimTiff;
  xstring tag_270 = read_tag_as_string(tif, ifd, 270);


  //----------------------------------------------------------------------------
  // Add all other tags as Custom tags
  //----------------------------------------------------------------------------
  DTagMap fluo_hash;
  fluo_hash.parse_ini( tag_270, "=" );
  fluo_hash.eraseKeysStaringWith("LUT Ch");
  hash->append_tags( fluo_hash, "custom/" );


  //----------------------------------------------------------------------------
  // Parsing some additional specific tags
  //----------------------------------------------------------------------------
  std::map< std::string, std::string >::const_iterator it;

  // objective
  it = hash->find("custom/Acquisition Parameters/Objective Lens");
  if (it != hash->end() ) 
    hash->append_tag( "objective", it->second );

  // magnification
  it = hash->find("custom/Acquisition Parameters/Magnification");
  if (it != hash->end() ) 
    hash->append_tag( "magnification", it->second );


  //---------------------------------------
  //Date=02-17-2004
  //Time=11:54:50
  xstring line;

  it = hash->find("custom/Acquisition Parameters/Date");
  if (it != hash->end() ) line = it->second;
  std::vector<xstring> date = line.split( "-" );

  it = hash->find("custom/Acquisition Parameters/Time");
  if (it != hash->end() ) line = it->second;
  std::vector<xstring> time = line.split( ":" );

  if (date.size()>2 && time.size()>2) {
    xstring imaging_time;
    imaging_time.sprintf("%.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
       date[2].toInt(), date[0].toInt(), date[1].toInt(), 
       time[0].toInt(), time[1].toInt(), time[2].toInt() );
    hash->append_tag( "date_time", imaging_time );
  }

  return 0;
}























