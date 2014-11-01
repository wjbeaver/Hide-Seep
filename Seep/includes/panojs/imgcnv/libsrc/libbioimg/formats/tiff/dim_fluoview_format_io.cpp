/*****************************************************************************
  TIFF FLUOVIEW IO 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation
        
  Ver : 2
*****************************************************************************/

#include <xstring.h>
#include "dim_tiff_format.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <tag_map.h>
#include <bim_metatags.h>

#include <Eigen/Dense>

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
  #pragma warning(disable:4996)
#endif 

void change_0_to_n (char *str, long size);

void read_text_tag(TIFF *tif, TDimTiffIFD *ifd, DIM_UINT tag, MemIOBuf *outIOBuf);

//----------------------------------------------------------------------------
// PSIA MISC FUNCTIONS
//----------------------------------------------------------------------------

/*
bool fluoviewIsTiffValid(TIFF *tif) {
  if (tif->tif_flags&TIFF_BIGTIFF) return false;        
  char *b_list = NULL;
  int16   d_list_count;
  int res[3] = {0,0,0};
  if (tif == 0) return false;
  res[0] = TIFFGetField(tif, BIM_MMHEADER, &d_list_count, &b_list);
  res[1] = TIFFGetField(tif, BIM_MMSTAMP, &d_list_count, &b_list);
  res[2] = TIFFGetField(tif, BIM_MMUSERBLOCK, &d_list_count, &b_list);
  if (res[0]==1 && res[1]==1 //&& res[2]==1
    ) return true;
  return false;
}
*/

bool isValidTiffFluoview(DTiffParams *tiffParams) {
  if (!tiffParams) return false;
  if (tiffParams->dimTiff->tif_flags&TIFF_BIGTIFF) return false;    
  return (isTagPresentInFirstIFD(&tiffParams->ifds, BIM_MMHEADER) &&
          isTagPresentInFirstIFD(&tiffParams->ifds, BIM_MMSTAMP) &&
          isTagPresentInFirstIFD(&tiffParams->ifds, BIM_MMUSERBLOCK) );
}

bool isValidTiffAndor(DTiffParams *tiffParams) {
  if (!tiffParams) return false;
  if (tiffParams->dimTiff->tif_flags&TIFF_BIGTIFF) return false;    
  return (isTagPresentInFirstIFD(&tiffParams->ifds, BIM_MMHEADER) &&
          isTagPresentInFirstIFD(&tiffParams->ifds, BIM_MMSTAMP) &&
          isTagPresentInFirstIFD(&tiffParams->ifds, BIM_ANDORBLOCK) );
}

void doSwabMMHEAD(MM_DIM_INFO *mmdi) {
  TIFFSwabLong   ((uint32*) &mmdi->Size);
  TIFFSwabDouble ((double*) &mmdi->Origin);
  TIFFSwabDouble ((double*) &mmdi->Resolution);
}

void doSwabMMHEAD(MM_HEAD *mmh) {
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

void printMM_DIM_INFO(MM_DIM_INFO *mmdi) {
  printf( "\nMM_DIM_INFO Name: %s\n", mmdi->Name);
  printf( "MM_DIM_INFO size: %d\n", mmdi->Size);
  printf( "MM_DIM_INFO origin: %f\n", mmdi->Origin);
  printf( "MM_DIM_INFO resolution: %f\n", mmdi->Resolution);
  printf( "MM_DIM_INFO Units: %s\n", mmdi->Units);
}

void printMMHEADER(MM_HEAD *mmh) {
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

int fluoviewGetInfo (DTiffParams *tiffParams) {
  if (tiffParams == NULL) return 1;
  if (tiffParams->dimTiff == NULL) return 1;
  if (tiffParams->ifds.count <= 0) return 1;

  TDimImageInfo *info = &tiffParams->info;
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;
  DIM_UCHAR *buf = NULL;
  uint32 size, type;
  MM_HEAD *fvInfo;

  if (!isTagPresentInFirstIFD( &tiffParams->ifds, BIM_MMHEADER )) return 1;

  readTiffTag (tiffParams->dimTiff, &tiffParams->ifds.ifds[0], BIM_MMHEADER, size, type, &buf);
  if (!buf || size<sizeof(MM_HEAD)) return 1;
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
  for (int d=0; d<SPATIAL_DIMENSION; d++) {
    if ( strncmp(fvInfo->DimInfo[d].Name, "Ch", 2 ) == 0 )
      fvi->ch = fvInfo->DimInfo[d].Size; 
    else
    if ( strncmp(fvInfo->DimInfo[d].Name, "T", 1 ) == 0 )
      fvi->t_frames = fvInfo->DimInfo[d].Size; 
    else
    if ( strncmp(fvInfo->DimInfo[d].Name, "Z", 1 ) == 0 ) {
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
    fvi->sample_names[i] = xstring::xprintf( "Channel %d", i+1 );

  if (isTagPresentInIFD(ifd, 270)) {
    uint32 buf_size, buf_type;
    DIM_UCHAR *buf = NULL;
    readTiffTag (tif, ifd, 270, buf_size, buf_type, &buf);
    change_0_to_n ((char *) buf, buf_size);

    // Extract channel names
    for (int i=0; i<fvi->ch; i++) {
      char *line = strstr( (char *) buf, xstring::xprintf( "Channel %d Dye=", i+1 ).c_str() );
      if (line) {
        char str[1024];
        sscanf( line, xstring::xprintf( "Channel %d Dye=%%s\n", i+1 ).c_str(), str );
        fvi->sample_names[i] = str;
      }
    }

    
    // ----------------------------------------------------
    // Get desired channel to display mapping
    // ----------------------------------------------------
    fvi->display_lut.resize((int) bim::NumberDisplayChannels, -1);
    for (int i=0; i<(int) bim::NumberDisplayChannels; ++i) fvi->display_lut[i] = -1;

    bool display_channel_set=false;
    for (unsigned int sample=0; sample<fvi->ch; ++sample) {
      int r=-1,g=-1,b=-1;
      char *line = strstr( (char*)buf, xstring::xprintf("[LUT Ch%d]", sample).c_str() );
      if (line) {
        line = strstr( line, "RGB 255=" );    
        sscanf( line, "RGB 255=%d\t%d\t%d", &r, &g, &b );
      }

      int display_channel=-1;
      if ( r>=1 && g==0 && b==0 ) display_channel = bim::Red;
      else
      if ( r==0 && g>=1 && b==0 ) display_channel = bim::Green;
      else
      if ( r==0 && g==0 && b>=1 ) display_channel = bim::Blue;
      else
      if ( r>=1 && g>=1 && b==0 ) display_channel = bim::Yellow;
      else
      if ( r>=1 && g==0 && b>=1 ) display_channel = bim::Magenta;
      else
      if ( r==0 && g>=1 && b>=1 ) display_channel = bim::Cyan;
      else
      if ( r>=1 && g>=1 && b>=1 ) display_channel = bim::Gray;

      if (display_channel>=0) { fvi->display_lut[display_channel] = sample; display_channel_set=true; }
    }

    // if the image has no preferred mapping set something
    if (!display_channel_set)
      if (fvi->ch == 1) 
        for (int i=0; i<3; ++i) fvi->display_lut[i] = 0;
      else
        for (int i=0; i<std::min((int) bim::NumberDisplayChannels, (int)fvi->ch); ++i) 
          fvi->display_lut[i] = i;


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
void fluoviewInitPalette( DTiffParams *tiffParams, TDimImageInfo *info ) {
  uint32 buf_size;
  uint32 buf_type;  
  DIM_UCHAR *buf = NULL;
  TIFF *tif = tiffParams->dimTiff;
  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];

  if (!tif) return;
  if (!info) return;
  if (tiffParams->subType!=tstFluoview) return; 
  if (tiffParams->fluoviewInfo.ch > 1) return;
  if (!isTagPresentInIFD(ifd, 270)) return;

  readTiffTag (tif, ifd, 270, buf_size, buf_type, &buf);
  change_0_to_n ((char *) buf, buf_size);

  info->lut.count = 0;
  for (int i=0; i<256; i++) info->lut.rgba[i] = i*256;

  char *line = strstr( (char *) buf, "[LUT Ch0]" );
  if (line) 
  for (int i=0; i<256; i++) {
    line = strstr( line, xstring::xprintf("RGB %d=", i).c_str() );    
    int r,g,b;
    sscanf( line, xstring::xprintf("RGB %d=%%d\t%%d\t%%d\n", i).c_str(), &r, &g, &b );
    info->lut.rgba[i] = dimRGB( r, g, b );
  }
  info->lut.count = 256;

  freeTiffTagBuf( (DIM_UCHAR **) &buf );
}


void fluoviewGetCurrentPageInfo(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return;
  TDimImageInfo *info = &tiffParams->info;

  if ( tiffParams->subType==tstFluoview || tiffParams->subType==tstAndor) {
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

DIM_UINT fluoviewReadPlane( TDimFormatHandle *fmtHndl, DTiffParams *tiffParams, int plane ) {
  if (tiffParams == 0) return 1;
  if (tiffParams->dimTiff == 0) return 1;
  if (!(tiffParams->subType==tstFluoview || tiffParams->subType==tstAndor)) return 1;  

  TIFF *tif = tiffParams->dimTiff;  
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;  
  TDimImageInfo *ii = &tiffParams->info;
  if (!tif) return 1;

  uint16 bitspersample = 1;
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

  ii->samples = fvi->ch;
  //ii->imageMode = DIM_GRAYSCALE;    
  ii->depth = bitspersample;

  TDimImageBitmap *img = fmtHndl->image;
  if ( allocImg( fmtHndl, ii, img) != 0 ) return 1;
  DIM_UINT lineSize = getLineSizeInBytes( img );

  //--------------------------------------------------------------------
  // read data
  //--------------------------------------------------------------------
  for (unsigned int sample=0; sample<(unsigned int)fvi->ch; ++sample) {
    // switch to correct page in original TIFF
    int dirNum = fmtHndl->pageNumber + sample * ( fvi->pages_tiff / fvi->ch );
    TIFFSetDirectory( tif, dirNum );
    if( TIFFIsTiled(tif) ) continue;

    DIM_UCHAR *p = (DIM_UCHAR *) img->bits[ sample ];
    for(unsigned int y=0; y<img->i.height; y++) {
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

void parse_metadata_fluoview (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  DTiffParams *tiffParams = (DTiffParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &tiffParams->info;
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;
  MM_HEAD *fvInfo = &fvi->head;

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
  hash->append_tags( fluo_hash, bim::CUSTOM_TAGS_PREFIX );


  //----------------------------------------------------------------------------
  // Parsing some additional specific tags
  //----------------------------------------------------------------------------
  std::map< std::string, std::string >::const_iterator it;

  // objective
  it = hash->find("custom/Acquisition Parameters/Objective Lens");
  if (it != hash->end() ) 
    hash->append_tag( bim::OBJECTIVE_DESCRIPTION, it->second );

  // magnification
  it = hash->find("custom/Acquisition Parameters/Magnification");
  if (it != hash->end() ) 
    hash->append_tag( bim::OBJECTIVE_MAGNIFICATION, it->second );


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
    hash->append_tag( bim::IMAGE_DATE_TIME, imaging_time );
  }
}

void parse_metadata_andor (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  DTiffParams *tiffParams = (DTiffParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &tiffParams->info;
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;
  MM_HEAD *fvInfo = &fvi->head;

  //----------------------------------------------------------------------------
  // Additional info from Tag 270
  //----------------------------------------------------------------------------
  TDimTiffIFD *ifd = &tiffParams->ifds.ifds[0];
  TIFF *tif = tiffParams->dimTiff;
  xstring tag_270 = read_tag_as_string(tif, ifd, 270);

  //----------------------------------------------------------------------------
  // Add all other tags as Custom tags
  //----------------------------------------------------------------------------
  DTagMap andor_hash;

  // first parse all INI standard tags
  andor_hash.parse_ini( tag_270, "=" );
  
  // now parse out of standard Andor blocks
  std::deque<std::string> ProtocolDescription = DTagMap::iniGetBlock( tag_270, "Protocol Description" );
  // simply add lines, needed parsing
  if (ProtocolDescription.size()>0) {
    std::string pp;
    std::deque<std::string>::const_iterator itt = ProtocolDescription.begin();  
    while (itt != ProtocolDescription.end()) {
      pp += *itt;
      pp += "\n";
      ++itt;
    }
    andor_hash["Protocol Description"] = pp;
  }

  std::deque<std::string> RegionInfo = DTagMap::iniGetBlock( tag_270, "Region Info (Fields)" );
  // simply add lines, needed parsing
  if (RegionInfo.size()>0) {
    std::string pp;
    std::deque<std::string>::const_iterator itt = RegionInfo.begin();  
    while (itt != RegionInfo.end()) {
      pp += *itt;
      pp += "\n";
      ++itt;
    }
    andor_hash["Region Info (Fields)"] = pp;
  }  


  
  hash->append_tags( andor_hash, bim::CUSTOM_TAGS_PREFIX );

  //----------------------------------------------------------------------------
  // Parsing some additional specific tags
  //----------------------------------------------------------------------------
  std::map< std::string, std::string >::const_iterator it;

/*
  // objective
  it = hash->find("custom/Acquisition Parameters/Objective Lens");
  if (it != hash->end() ) 
    hash->append_tag( bim::OBJECTIVE_DESCRIPTION, it->second );

  // magnification
  it = hash->find("custom/Acquisition Parameters/Magnification");
  if (it != hash->end() ) 
    hash->append_tag( bim::OBJECTIVE_MAGNIFICATION, it->second );
*/

  //---------------------------------------
  //Date=06/08/2010
  //Time=10:10:36 AM
  xstring line;

  it = andor_hash.find("Created/Date");
  if (it != andor_hash.end() ) line = it->second;
  std::vector<xstring> date = line.split( "/" );

  it = andor_hash.find("Created/Time");
  if (it != andor_hash.end() ) line = it->second;
  std::vector<xstring> time_ampm = line.split( " " );
  std::vector<xstring> time = time_ampm[0].split( ":" );
  int time_add=0;
  if (time_ampm.size()>=1 && time_ampm[1]=="PM") time_add=12;

  if (date.size()>2 && time.size()>2) {
    xstring imaging_time;
    imaging_time.sprintf("%.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
       date[2].toInt(), date[0].toInt(), date[1].toInt(), 
       time[0].toInt()+time_add, time[1].toInt(), time[2].toInt() );
    hash->append_tag( bim::IMAGE_DATE_TIME, imaging_time );
  }

  //----------------------------------------------------------------------------
  // Parsing stage
  //----------------------------------------------------------------------------

  //parse XYFields
  std::deque< Eigen::Vector3d > xyFields;
  std::vector<xstring> XYFields = xstring(andor_hash["XYZScan/XYFields"]).split( "\t" );
  if ( XYFields.size()>0 && XYFields[0].toInt()<XYFields.size() ) {
    for (int i=1; i<XYFields.size(); ++i) {
      std::vector<xstring> XYZ = XYFields[i].split(",");
      if (XYZ.size()<3) continue;
      xyFields.push_back( Eigen::Vector3d(XYZ[0].toDouble(), XYZ[1].toDouble(), XYZ[2].toDouble()) );
    }
  }

  //parse MontageOffsets
  std::deque< Eigen::Vector3d > montageOffsets;
  std::vector<xstring> MontageOffsets = xstring(andor_hash["XYZScan/MontageOffsets"]).split( "\t" );
  if ( MontageOffsets.size()>0 && MontageOffsets[0].toInt()<MontageOffsets.size() ) {
    for (int i=1; i<MontageOffsets.size(); ++i) {
      std::vector<xstring> XYZ = MontageOffsets[i].split(",");
      if (XYZ.size()<3) continue;
      montageOffsets.push_back( Eigen::Vector3d(XYZ[0].toDouble(), XYZ[1].toDouble(), XYZ[2].toDouble()) );
    }
  }

  // write stage coordinates
  int p=0;
  for (int i=0; i<xyFields.size(); ++i)
    for (int j=0; j<montageOffsets.size(); ++j) {
      Eigen::Vector3d v = xyFields[i] + montageOffsets[j];
      hash->append_tag( xstring::xprintf( bim::STAGE_POSITION_TEMPLATE_X.c_str(), p ), v[0] );
      hash->append_tag( xstring::xprintf( bim::STAGE_POSITION_TEMPLATE_Y.c_str(), p ), v[1] );
      hash->append_tag( xstring::xprintf( bim::STAGE_POSITION_TEMPLATE_Z.c_str(), p ), v[2] );
      ++p;
    }

}

DIM_UINT append_metadata_fluoview (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;

  DTiffParams *tiffParams = (DTiffParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &tiffParams->info;
  DFluoviewInfo *fvi = &tiffParams->fluoviewInfo;
  MM_HEAD *fvInfo = &fvi->head;


  hash->append_tag( bim::IMAGE_NUM_Z, fvi->z_slices );
  hash->append_tag( bim::IMAGE_NUM_T, fvi->t_frames );
  hash->append_tag( bim::IMAGE_NUM_C, fvi->ch );

  //----------------------------------------------------------------------------
  // DIMENSIONS
  //----------------------------------------------------------------------------

  for (int i=0; i<SPATIAL_DIMENSION; i++) {
    xstring name  = fvInfo->DimInfo[i].Name;
    name = name.removeSpacesBoth(); 
    double res   = fvInfo->DimInfo[i].Resolution;
    xstring units = fvInfo->DimInfo[i].Units;
    units = units.removeSpacesBoth(); 
    if (units == "um") units = bim::PIXEL_RESOLUTION_UNIT_MICRONS;
    if (units == "µm") units = bim::PIXEL_RESOLUTION_UNIT_MICRONS;
    if (units == "s") units = bim::PIXEL_RESOLUTION_UNIT_SECONDS;

    if (name == "X") {
      hash->append_tag( bim::PIXEL_RESOLUTION_X, res );
      hash->append_tag( bim::PIXEL_RESOLUTION_UNIT_X, units );
    } else
    if (name == "Y") {
      hash->append_tag( bim::PIXEL_RESOLUTION_Y, res );
      hash->append_tag( bim::PIXEL_RESOLUTION_UNIT_Y, units );
    } else
    if (name == "Z") {
      hash->append_tag( bim::PIXEL_RESOLUTION_Z, res );
      hash->append_tag( bim::PIXEL_RESOLUTION_UNIT_Z, units );
    } else
    if (name == "T") {
      hash->append_tag( bim::PIXEL_RESOLUTION_T, res );
      hash->append_tag( bim::PIXEL_RESOLUTION_UNIT_T, units );
    }
  }  

  //----------------------------------------------------------------------------
  // Channel names and preferred mapping
  //----------------------------------------------------------------------------
  for (unsigned int i=0; i<fvi->sample_names.size(); ++i)
    hash->append_tag( xstring::xprintf(bim::CHANNEL_NAME_TEMPLATE.c_str(), i), fvi->sample_names[i] );

  // preferred lut mapping
  if (fvi->display_lut.size() >= bim::NumberDisplayChannels) {
    hash->set_value( bim::DISPLAY_CHANNEL_RED,     fvi->display_lut[bim::Red] );
    hash->set_value( bim::DISPLAY_CHANNEL_GREEN,   fvi->display_lut[bim::Green] );
    hash->set_value( bim::DISPLAY_CHANNEL_BLUE,    fvi->display_lut[bim::Blue] );
    hash->set_value( bim::DISPLAY_CHANNEL_YELLOW,  fvi->display_lut[bim::Yellow] );
    hash->set_value( bim::DISPLAY_CHANNEL_MAGENTA, fvi->display_lut[bim::Magenta] );
    hash->set_value( bim::DISPLAY_CHANNEL_CYAN,    fvi->display_lut[bim::Cyan] );
    hash->set_value( bim::DISPLAY_CHANNEL_GRAY,    fvi->display_lut[bim::Gray] );
  }

  //----------------------------------------------------------------------------
  // Additional info
  //----------------------------------------------------------------------------

  if (tiffParams->subType==tstFluoview) parse_metadata_fluoview( fmtHndl, hash );
  else
  if (tiffParams->subType==tstAndor) parse_metadata_andor( fmtHndl, hash );

  return 0;
}

