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
      
  ver: 2
        
*******************************************************************************/

#ifndef META_FORMAT_MANAGER_H
#define META_FORMAT_MANAGER_H

#include "dim_format_manager.h"

#include <vector>
#include <map>

#include <xstring.h>
#include <tag_map.h>

class TMetaFormatManager : public TDimFormatManager {
public:
  TMetaFormatManager();
  ~TMetaFormatManager();
  int  sessionStartRead  (const char *fileName);
  int  sessionReadImage  ( TDimImageBitmap *bmp, DIM_UINT page );
  int  sessionWriteImage ( TDimImageBitmap *bmp, DIM_UINT page );

  void sessionParseMetaData ( DIM_UINT page );
  TDimImageBitmap *sessionImage();
  void sessionEnd();

  //void writeImage (const char *fileName, TDimImageBitmap *bmp, const char *formatName, const char *options = NULL);

  void sessionWriteSetMetadata( const DTagMap &hash );
  void sessionWriteSetOMEXML( const std::string &omexml );

  // META
  double       getPixelSizeX();
  double       getPixelSizeY();
  double       getPixelSizeZ();
  double       getPixelSizeT();
  const char*  getImagingTime();

  inline const std::string                 &get_text_metadata() const { return meta_data_text; }  
  inline const DTagMap                     &get_metadata() const      { return metadata; }
  inline const std::vector< int >          &get_display_lut() const   { return display_lut; }
  inline const std::vector< std::string >  &get_channel_names() const { return channel_names; }

  inline std::string get_metadata_tag( const std::string &key, const std::string &def ) const { return metadata.get_value( key, def ); }
  inline int         get_metadata_tag_int( const std::string &key, const int &def ) const { return metadata.get_value_int( key, def ); }
  inline double      get_metadata_tag_double( const std::string &key, const double &def ) const { return metadata.get_value_double( key, def ); }

  

private:
  int           got_meta_for_session;
  TDimTagList  *tagList;
  std::string   meta_data_text;
  TDimImageInfo info;

  TDimTagItem   writeTagItem;

  // key-value pairs for metadata
  DTagMap metadata;
  inline void appendMetadata( const std::string &key, const std::string &value ) { metadata.append_tag( key, value ); }
  inline void appendMetadata( const std::string &key, const int &value ) { metadata.append_tag( key, value ); }
  inline void appendMetadata( const std::string &key, const double &value ) { metadata.append_tag( key, value ); }

  // META
  double pixel_size[4]; //XYZ in microns, T in seconds
  xstring imaging_time; // "YYYY-MM-DD HH:MM:SS" ANSI date and 24h time
  std::vector< std::string > channel_names;
  std::vector< int > display_lut;

  void fill_static_metadata_from_map();
};

#endif // META_FORMAT_MANAGER_H

