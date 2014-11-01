/*****************************************************************************
  Olympus Image Binary (OIB) format support
  Copyright (c) 2008, Center for Bio Image Informatics, UCSB
  
  Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    2008-06-04 14:26:14 - First creation
    2008-09-15 19:04:47 - Fix for older files with unordered streams
    2008-11-06 13:36:43 - Parse preferred channel mapping
            
  Ver : 3
*****************************************************************************/

#include <cmath>
#include <cstring>

#include <map>
#include <iostream>
#include <fstream>
#include <xstring.h>

#include "dim_oib_format.h"
#include "dim_oib_format_io.cpp"

//****************************************************************************
// MISC
//****************************************************************************

int read_stream_as_wstring( POLE::Storage *storage, const char *name, std::wstring &str ) {
 
  POLE::Stream *stream = new POLE::Stream( storage, name );
  if (!stream || stream->size() <= 0) return 1;
  if( stream->fail() ) return 1;
  str.resize( stream->size() );
  unsigned red = stream->read( (unsigned char *) &str[0], stream->size() );
  delete stream;
  if (red != str.size()) return 1;
  return 0;
}

int read_stream_as_string( POLE::Storage *storage, const char *name, std::string &str ) {
  std::wstring wstr;
  int res = read_stream_as_wstring( storage, name, wstr );
  if (res!=0) return res;

  unsigned size = wstr.size()-1;
  str.resize( size );
  char *p = (char *) &wstr[0];
  p+=2;

  for( unsigned i=0; i<size; ++i ) {
    str[i] = *p;
    p+=2;
  }
  
  return 0;
}


//****************************************************************************
// LUTs
//****************************************************************************
/*
LUT file will be named as “xxx_LUTX.lut”
Where, X is the channel number
*/
std::string get_lut_stream_name( DOibParams *par, int sample ) {
  xstring stream_name, part;

  // create filename
  stream_name += par->oifFolderName;
  stream_name += "/";
  stream_name += par->oifFileName;
  
  part.sprintf("_LUT%d.lut", sample+1);
  stream_name += part;

  return stream_name;
}

DTagMap parse_lut_stream( DOibParams *par, int sample ) {
  std::string info_header("OibSaveInfo/");

  std::string file_name = get_lut_stream_name( par, sample );
  xstring stream_name = par->oib_info_hash.get_key( file_name );
  //strip initial "OibSaveInfo/" from the key
  stream_name.erase( 0, info_header.size() );
  stream_name = "/" + par->oifFolderName + "/" + stream_name;

  std::string lut_data;
  DTagMap lut_info;
  if (read_stream_as_string( par->storage, stream_name.c_str(), lut_data )!=0) return lut_info;
  lut_info.parse_ini( lut_data, "=", "", "ColorLUTData" );

  return lut_info;
}

//****************************************************************************
// CHANNELS
//****************************************************************************
/*
XYZ file will be named as “xxx_C00mZ00n.tif”
XYZT file will be named as “xxx_C00mZ00nT00p.tif”
XYLZT file will be named as “xxx_C00mL00qZ00nT00p.tif”
Where, C00m shows Channel No.m, Z00n shows Z Slice No. n, L00q shows Lambda Slice No.q, T00p shows Time Slice No.p.
*/
std::string get_channel_stream_name( DOibParams *par, int page, int sample ) {
  int z=0, t=0, l=0;
  int nz = std::max(par->num_z, 1);
  int nt = std::max(par->num_t, 1);
  int nl = std::max(par->num_l, 1);
  xstring stream_name, part;

  // get proper z, t and l from page number that's in order ZT
  l = (page/nz)/nt;
  t = (page - l*nt*nz) / nz;
  z = page - nt*l - nz*t;

  // create filename
  //stream_name += par->oifFolderName;
  //stream_name += "/";
  //stream_name += par->oifFileName;
  //stream_name += "_";
  
  // Order here DOES MATTER!!!!
  // channel
  part.sprintf("C%.3d", sample+1);
  stream_name += part;
  // L
  if (par->num_l>0) {
    part.sprintf("L%.3d", l+1);
    stream_name += part;
  }
  // Z
  if (par->num_z>0) {
    part.sprintf("Z%.3d", z+1);
    stream_name += part;
  }
  // T
  if (par->num_t>0) {
    part.sprintf("T%.3d", t+1);
    stream_name += part;
  }
  stream_name += ".tif";

  return stream_name;
}


//****************************************************************************
// INTERNAL STRUCTURES
//****************************************************************************

int oibGetImageInfo( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  DOibParams *par = (DOibParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->i; 
  
  info->ver = sizeof(TDimImageInfo);
  info->imageMode = DIM_GRAYSCALE;
  info->tileWidth = 0;
  info->tileHeight = 0; 
  info->transparentIndex = 0;
  info->transparencyMatting = 0;
  info->lut.count = 0;

  // now load the structure
  par->storage = new POLE::Storage( fmtHndl->fileName );
  par->storage->open();
  if( par->storage->result() != POLE::Storage::Ok ) return 1;
  
  // load first OibInfo.txt
  std::string oib_info;  
  if (read_stream_as_string( par->storage, "/OibInfo.txt", oib_info )!=0) return 1;
  DTagMap oib_info_hash;
  oib_info_hash.parse_ini( oib_info );

  std::string MainFileName = std::string() + "/" + oib_info_hash["OibSaveInfo/MainFileName"];
  par->oifFolderName = oib_info_hash["OibSaveInfo/ThumbFolderName"];
  par->oifFileName = oib_info_hash[ "OibSaveInfo/" + oib_info_hash["OibSaveInfo/MainFileName"] ];
  if (par->oifFileName.size()>4) par->oifFileName.resize( par->oifFileName.size()-4 );
  par->oib_info_hash = oib_info_hash;

  // load MainFileName and parse all metadata
  if (read_stream_as_string( par->storage, MainFileName.c_str(), par->oif_metadata )!=0) return 1;
  par->oif_metadata_hash.parse_ini( par->oif_metadata );

  xstring axis_name;
  int axis_count = par->oif_metadata_hash.get_value_int( "Axis Parameter Common/AxisCount", 0 );
  for (int i=0; i<8; ++i) {
    par->axis.push_back( DOibAxis() );
    axis_name.sprintf("Axis %d Parameters Common", i);
    par->axis[i].MaxSize  = par->oif_metadata_hash.get_value_int( axis_name+"/MaxSize", 0 );

    par->axis[i].StartPosition  = par->oif_metadata_hash.get_value_double( axis_name+"/StartPosition", 0 );
    par->axis[i].EndPosition  = par->oif_metadata_hash.get_value_double( axis_name+"/EndPosition", 0 );

    par->axis[i].AxisCode = par->oif_metadata_hash.get_value( axis_name+"/AxisCode", "" );
    par->axis[i].AxisName = par->oif_metadata_hash.get_value( axis_name+"/AxisName", "" );
    par->axis[i].PixUnit  = par->oif_metadata_hash.get_value( axis_name+"/PixUnit", "" );
    par->axis[i].UnitName = par->oif_metadata_hash.get_value( axis_name+"/UnitName", "" );
  }

  info->depth  = par->oif_metadata_hash.get_value_int( "Reference Image Parameter/ImageDepth", 0 )*8;
  info->width  = par->axis[0].MaxSize;
  info->height = par->axis[1].MaxSize;
  info->samples = par->axis[2].MaxSize;

  // pixel resolution
  par->pixel_resolution.resize(4);
  for (int i=0; i<4; ++i) par->pixel_resolution[i] = 0;
  if (par->axis[0].MaxSize != 0)
    par->pixel_resolution[0] = fabs(par->axis[0].EndPosition-par->axis[0].StartPosition) / (double) par->axis[0].MaxSize;
  if (par->axis[1].MaxSize != 0)
    par->pixel_resolution[1] = fabs(par->axis[1].EndPosition-par->axis[1].StartPosition) / (double) par->axis[1].MaxSize;
  if (par->axis[3].MaxSize != 0)
    par->pixel_resolution[2] = fabs(par->axis[3].EndPosition-par->axis[3].StartPosition) / (double) par->axis[3].MaxSize;
  if (par->axis[4].MaxSize != 0)
    par->pixel_resolution[3] = fabs(par->axis[4].EndPosition-par->axis[4].StartPosition) / (double) par->axis[4].MaxSize;

  // read axis units and scale pixel size accordingly
  if (par->axis[0].PixUnit == "nm") par->pixel_resolution[0] /= 1000.0;
  if (par->axis[1].PixUnit == "nm") par->pixel_resolution[1] /= 1000.0;
  if (par->axis[3].PixUnit == "nm") par->pixel_resolution[2] /= 1000.0;
  if (par->axis[4].PixUnit == "ms") par->pixel_resolution[3] /= 1000.0;

  // add Z and T pages
  info->number_pages = 1;
  if (axis_count > 3) info->number_pages *= par->axis[3].MaxSize;
  if (axis_count > 4) info->number_pages *= par->axis[4].MaxSize;

  info->number_z = std::max(par->axis[3].MaxSize, 1);
  info->number_t = std::max(par->axis[4].MaxSize, 1);

  par->num_z = par->axis[3].MaxSize;
  par->num_t = par->axis[4].MaxSize;
  par->num_l = par->axis[6].MaxSize;

  if (info->depth != 16) 
    info->pixelType = D_FMT_UNSIGNED;
  else
    info->pixelType = D_FMT_UNSIGNED;

  info->resUnits = DIM_RES_um;
  info->xRes = par->pixel_resolution[0];
  info->yRes = par->pixel_resolution[1];

  //---------------------------------------------------------------
  // Channel names
  //---------------------------------------------------------------
  //"Reference Image Parameter/ImageDepth"
  xstring channel_dir;
  par->channel_names.clear();
  int c=1;
  while (c < std::max( (int)info->samples, (int)16) ) {
    channel_dir.sprintf("Channel %d Parameters", c);
    int phys_num = par->oif_metadata_hash.get_value_int( channel_dir+"/Physical CH Number", -1 );
    if (phys_num >= 0)
      par->channel_names.push_back( par->oif_metadata_hash.get_value( channel_dir+"/DyeName", "" ) );
    else break;
    ++c;
  }

  //---------------------------------------------------------------
  // Channel mapping
  //---------------------------------------------------------------
  // Here we basically read Contrast and guess where each channel should be mapped
  // we'll use Red/Contrast Green/Contrast Blue/Contrast, when contrast is 100 
  // that channel will be the preferred mapping

  // first initialize the lut
  par->display_lut.resize(3, -1);
  for (int i=0; i< std::min<int>(3,info->samples); ++i) 
    par->display_lut[i] = i;

  for (unsigned int sample=0; sample<info->samples; ++sample) {
    DTagMap lut_info = parse_lut_stream( par, sample );

    int r = lut_info.get_value_int( "Red/Contrast", 0 );
    int g = lut_info.get_value_int( "Green/Contrast", 0 );
    int b = lut_info.get_value_int( "Blue/Contrast", 0 );

    int chOut=-1;
    if ( r>g && r>b ) chOut = 0;
    else
    if ( g>r && g>b ) chOut = 1;
    else
    if ( b>g && b>r ) chOut = 2;

    if (chOut>=0) par->display_lut[chOut] = sample;
  }


  //---------------------------------------------------------------
  // define dims
  //---------------------------------------------------------------
  if (info->number_z>1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_Z;
  }

  if (info->number_t>1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_T;
  }

  if (info->number_z>1 && info->number_t>1) {
    info->number_dims = 5;
    info->dimensions[3].dim = DIM_DIM_Z;        
    info->dimensions[4].dim = DIM_DIM_T;
  }


  return 0;
}

//****************************************************************************
// FORMAT REQUIRED FUNCTIONS
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimOibValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length) {
  if (length < DIM_OIB_MAGIC_SIZE) return -1;
  if (memcmp( magic, oibMagic, DIM_OIB_MAGIC_SIZE ) != 0) return -1;

  // now try to parse and check if it's really OIB

  return 0;
}

TDimFormatHandle dimOibAquireFormatProc( void ) {
  TDimFormatHandle fp = initTDimFormatHandle();
  return fp;
}

void dimOibReleaseFormatProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  dimOibCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimOibCloseImageProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams != NULL)
    delete (DOibParams *) fmtHndl->internalParams;
  fmtHndl->internalParams = NULL;
}

DIM_UINT dimOibOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode)
{
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimOibCloseImageProc (fmtHndl);  
  fmtHndl->internalParams = (void *) new DOibParams();
  DOibParams *par = (DOibParams *) fmtHndl->internalParams;

  if (io_mode == DIM_IO_READ) {
    int res = oibGetImageInfo( fmtHndl );
    if (res != 0) dimOibCloseImageProc(fmtHndl);
    return res;
  }
  
  return 1;
}

//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimOibGetNumPagesProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;
  DOibParams *par = (DOibParams *) fmtHndl->internalParams;
  return par->i.number_pages;
}


TDimImageInfo dimOibGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT /*page_num*/ ) {
  if (!fmtHndl) return initTDimImageInfo();
  DOibParams *par = (DOibParams *) fmtHndl->internalParams;
  return par->i;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimOibReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT /*page*/, int group, int tag, int type) {
  if (!fmtHndl) return 1;
  return read_oib_metadata (fmtHndl, group, tag, type);
}

char* dimOibReadMetaDataAsTextProc ( TDimFormatHandle *fmtHndl ) {
  return NULL;
}

DIM_UINT dimOibAddMetaDataProc (TDimFormatHandle * /*fmtHndl*/) {
  return 1;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimOibReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  fmtHndl->pageNumber = page;
  return read_oib_image( fmtHndl );
}

DIM_UINT dimOibWriteImageProc ( TDimFormatHandle * /*fmtHndl*/ ) {
  return 1;
}


//****************************************************************************
// EXPORTED FUNCTION
//****************************************************************************

#define DIM_OIB_NUM_FORMTAS 1

TDimFormatItem dimOibItems[DIM_OIB_NUM_FORMTAS] = {
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
  }
};

TDimFormatHeader dimOibHeader = {

  sizeof(TDimFormatHeader),
  "1.0.5",
  "Olympus Image Binary CODEC",
  "Olympus Image Binary CODEC",
  
  DIM_OIB_MAGIC_SIZE,         // 0 or more, specify number of bytes needed to identify the file
  {1, DIM_OIB_NUM_FORMTAS, dimOibItems},   //dimJpegSupported,
  
  dimOibValidateFormatProc,
  // begin
  dimOibAquireFormatProc, //TDimAquireFormatProc
  // end
  dimOibReleaseFormatProc, //TDimReleaseFormatProc
  
  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimOibOpenImageProc, //TDimOpenImageProc
  dimOibCloseImageProc, //TDimCloseImageProc 

  // info
  dimOibGetNumPagesProc, //TDimGetNumPagesProc
  dimOibGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimOibReadImageProc, //TDimReadImageProc 
  NULL, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc
  
  // meta data
  dimOibReadMetaDataProc, //TDimReadMetaDataProc
  dimOibAddMetaDataProc,  //TDimAddMetaDataProc
  dimOibReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc
  oib_append_metadata, //TDimAppendMetaDataProc

  NULL,
  NULL,
  ""

};

extern "C" {

TDimFormatHeader* dimOibGetFormatHeader(void) {
  return &dimOibHeader;
}

} // extern C





