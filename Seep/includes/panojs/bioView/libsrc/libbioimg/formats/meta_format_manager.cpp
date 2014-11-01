/*******************************************************************************

  Manager for Image Formats with MetaData parsing

  Uses DimFiSDK version: 1
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  Notes:
    Session: during session any session wide operation will be performed
    with current session!!!
    If you want to start simultaneous sessions then create new manager
    using = operator. It will copy all necessary initialization data
    to the new manager.

    Non-session wide operation might be performed simultaneously with 
    an open session.

  History:
    03/23/2004 18:03 - First creation
    01/25/2007 21:00 - added QImaging TIFF
      
  ver: 3
        

*******************************************************************************/

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

#include <cstring>

#include <xstring.h>

#include "meta_format_manager.h"


TMetaFormatManager::TMetaFormatManager() 
: TDimFormatManager(){

  memset( &writeTagItem, 0, sizeof(writeTagItem) );
  writeTagItem.tagGroup  = DIM_META_GENERIC;
  writeTagItem.tagLength = 0;
  writeTagItem.tagType   = DIM_TAG_NOTYPE;
}

TMetaFormatManager::~TMetaFormatManager()
{

}

int TMetaFormatManager::sessionStartRead(const char *fileName) {
  got_meta_for_session = -1;
  imaging_time = "0000-00-00 00:00:00";
  pixel_size[0] = 0;
  pixel_size[1] = 0;
  pixel_size[2] = 0;
  pixel_size[3] = 0;
  channel_names.clear();
  display_lut.clear();
  metadata.clear();
  tagList = 0;
  info = initTDimImageInfo();
  return TDimFormatManager::sessionStartRead(fileName);
}

void TMetaFormatManager::sessionEnd() {
  got_meta_for_session = -1;
  tagList = 0;
  TDimFormatManager::sessionEnd();
}

int TMetaFormatManager::sessionWriteImage ( TDimImageBitmap *bmp, DIM_UINT page ) {
  if (session_active != TRUE) return 1;

  sessionHandle.metaData.count = 0;
  sessionHandle.metaData.tags  = 0;

  //meta_data_text metadata
  if (page==0 && metadata.size()>0) {
    writeTagItem.tagId     = METADATA_TAGS;
    writeTagItem.tagData   = &metadata;
    sessionHandle.metaData.count = 1;
    sessionHandle.metaData.tags  = &writeTagItem;
  }

  if (page==0 && meta_data_text.size()>0) {
    writeTagItem.tagId     = METADATA_OMEXML;
    writeTagItem.tagData   = &meta_data_text;
    sessionHandle.metaData.count = 1;
    sessionHandle.metaData.tags  = &writeTagItem;
  }

  return TDimFormatManager::sessionWriteImage( bmp, page );
}

int TMetaFormatManager::sessionReadImage ( TDimImageBitmap *bmp, DIM_UINT page ) {
  if (session_active != TRUE) return 1;
  int res = TDimFormatManager::sessionReadImage( bmp, page );
  info = bmp->i;

  channel_names.clear();
  display_lut.clear();
  metadata.clear();

  display_lut.resize(3);
  if (info.samples == 1)
    for (int i=0; i<3; ++i) display_lut[i] = 0;
  else
    if (info.samples == 2) {
      for (int i=0; i<2; ++i) display_lut[i] = i;
      display_lut[2] = -1;
    } 
  else
  if (info.samples >= 3)
    for (int i=0; i<3; ++i) display_lut[i] = i;

  switch ( info.resUnits ) {
    case DIM_RES_um:
      pixel_size[0] = info.xRes;
      pixel_size[1] = info.xRes;
      pixel_size[2] = 0;
      pixel_size[3] = 0;
      break;

    case DIM_RES_nm:
      pixel_size[0] = info.xRes / 1000.0;
      pixel_size[1] = info.xRes / 1000.0;
      pixel_size[2] = 0;
      pixel_size[3] = 0;
      break;

    case DIM_RES_mm:
      pixel_size[0] = info.xRes * 1000.0;
      pixel_size[1] = info.xRes * 1000.0;
      pixel_size[2] = 0;
      pixel_size[3] = 0;
      break;

    default:
      pixel_size[0] = 0;
      pixel_size[1] = 0;
      pixel_size[2] = 0;
      pixel_size[3] = 0;
  }

  return res;
}


void TMetaFormatManager::sessionWriteSetMetadata( const DTagMap &hash ) {
  metadata = hash;
}

void TMetaFormatManager::sessionWriteSetOMEXML( const std::string &omexml ) {
  meta_data_text = omexml;
}

void TMetaFormatManager::sessionParseMetaData ( DIM_UINT page ) {
  if (got_meta_for_session == page) return;

  tagList = sessionReadMetaData(page, -1, -1, -1);
  got_meta_for_session = page;

  char *meta_char = sessionGetTextMetaData();
  if (meta_char != NULL) {
    meta_data_text = meta_char;
    delete meta_char;
  }

  // API v1.7: first check if format has appand metadata function, if yes, run
  if (session_active && sessionFormatIndex>=0 && sessionFormatIndex<formatList.size()) {
    TDimFormatHeader *selectedFmt = formatList.at( sessionFormatIndex );
    if (selectedFmt->appendMetaDataProc)
      selectedFmt->appendMetaDataProc ( &sessionHandle, &metadata );
  }

  // All following will only be inserted if tags do not exist

  appendMetadata( "format", this->sessionGetFormatName() );
  if ( this->imaging_time != "0000-00-00 00:00:00" )   
    appendMetadata( "date_time", this->imaging_time );
  
  if ( this->getPixelSizeX() > 0 ) {
    appendMetadata( "pixel_resolution_x", this->getPixelSizeX() );
    appendMetadata( "pixel_resolution_unit_x", "microns" );
  }

  if ( this->getPixelSizeY() > 0 ) {
    appendMetadata( "pixel_resolution_y", this->getPixelSizeY() );
    appendMetadata( "pixel_resolution_unit_y", "microns" );
  }

  if ( this->getPixelSizeZ() > 0 ) {
    appendMetadata( "pixel_resolution_z", this->getPixelSizeZ() );
    appendMetadata( "pixel_resolution_unit_z", "microns" );
  }

  if ( this->getPixelSizeT() > 0 ) {
    appendMetadata( "pixel_resolution_t", this->getPixelSizeT() );
    appendMetadata( "pixel_resolution_unit_t", "seconds" );
  }

  // -------------------------------------------------------
  // Image info

  appendMetadata( "image_num_x", (int) info.width );
  appendMetadata( "image_num_y", (int) info.height );
  appendMetadata( "image_num_z", (int) info.number_z );
  appendMetadata( "image_num_t", (int) info.number_t );
  appendMetadata( "image_num_c", (int) info.samples );
  appendMetadata( "image_num_p", (int) info.number_pages );
  appendMetadata( "image_pixel_depth", (int) info.depth );

  for (unsigned int i=0; i<channel_names.size(); ++i) {
    xstring tag_name;
    tag_name.sprintf( "channel_%d_name", i );
    appendMetadata( tag_name.c_str(), channel_names[i].c_str() );
  }

  if (display_lut.size() >= 3) {
    appendMetadata( "display_channel_red",   display_lut[0] );
    appendMetadata( "display_channel_green", display_lut[1] );
    appendMetadata( "display_channel_blue",  display_lut[2] );
  }

  // hack towards new system: in case the format loaded all data directly into the map, refresh static values
  fill_static_metadata_from_map();
}

double TMetaFormatManager::getPixelSizeX()
{
  if (got_meta_for_session != sessionGetCurrentPage() ) 
    sessionParseMetaData ( sessionGetCurrentPage() );
  return pixel_size[0];
}

double TMetaFormatManager::getPixelSizeY()
{
  if (got_meta_for_session != sessionGetCurrentPage() ) 
    sessionParseMetaData ( sessionGetCurrentPage() );
  return pixel_size[1];
}

double TMetaFormatManager::getPixelSizeZ()
{
  if (got_meta_for_session != sessionGetCurrentPage() ) 
    sessionParseMetaData ( sessionGetCurrentPage() );
  return pixel_size[2];
}

double TMetaFormatManager::getPixelSizeT()
{
  if (got_meta_for_session != sessionGetCurrentPage() ) 
    sessionParseMetaData ( sessionGetCurrentPage() );
  return pixel_size[3];
}

const char* TMetaFormatManager::getImagingTime() {
  if (got_meta_for_session != sessionGetCurrentPage() ) 
    sessionParseMetaData ( sessionGetCurrentPage() );
  return imaging_time.c_str();
}


TDimImageBitmap *TMetaFormatManager::sessionImage() {
  return NULL;
}

//---------------------------------------------------------------------------------------
// MISC
//---------------------------------------------------------------------------------------

void TMetaFormatManager::fill_static_metadata_from_map() {
  std::string s;
  double d;
  int v;

  // Date time
  s = get_metadata_tag( "date_time", "" );
  if (s.size()>0) imaging_time = s;
  
  // Resolution
  d = get_metadata_tag_double( "pixel_resolution_x", 0 );
  if (d>0) pixel_size[0] = d;

  d = get_metadata_tag_double( "pixel_resolution_y", 0 );
  if (d>0) pixel_size[1] = d;

  d = get_metadata_tag_double( "pixel_resolution_z", 0 );
  if (d>0) pixel_size[2] = d;

  d = get_metadata_tag_double( "pixel_resolution_t", 0 );
  if (d>0) pixel_size[3] = d;


  // LUT
  v = get_metadata_tag_int( "display_channel_red", -2 );
  if (v>-2) display_lut[0] = v;  

  v = get_metadata_tag_int( "display_channel_green", -2 );
  if (v>-2) display_lut[1] = v;  

  v = get_metadata_tag_int( "display_channel_blue", -2 );
  if (v>-2) display_lut[2] = v;  

  // ------------------------------------------
  // Channel Names
 
  if (channel_names.size()<info.samples) channel_names.resize( info.samples );
  for (unsigned int i=0; i<channel_names.size(); ++i) {
    xstring tag_name;
    tag_name.sprintf( "channel_%d_name", i );

    s = get_metadata_tag( tag_name, "" );
    if (s.size()>0) channel_names[i] = s;
  }
}



