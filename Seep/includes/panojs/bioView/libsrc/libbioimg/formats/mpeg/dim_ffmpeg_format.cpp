/*****************************************************************************
FFMPEG support 
Copyright (c) 2008 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

IMPLEMENTATION

Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

History:
2008-02-01 14:45 - First creation

Ver : 2
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <cstring>

#include <string>
#include <set>
#include <list>
#include <utility>

#include <xstring.h>

// Disables Visual Studio 2005 warnings for deprecated code
#if ( defined(_MSC_VER) && (_MSC_VER >= 1400) )
#pragma warning(disable:4996)
#endif 

#include "dim_ffmpeg_format.h"
#include "dim_ffmpeg_format_io.cpp"

//****************************************************************************
//
// INTERNAL STRUCTURES
//
//****************************************************************************

bool ffmpegGetImageInfo( TDimFormatHandle *fmtHndl ) {

  if (fmtHndl == NULL) return FALSE;
  if (fmtHndl->internalParams == NULL) return FALSE;
  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->i;  
  *info = initTDimImageInfo();

  if (par->ff_in.numFrames() <= 0) return false;

  info->width        = par->ff_in.width();
  info->height       = par->ff_in.height();
  info->depth        = 8;
  info->samples      = par->ff_in.depth();
  info->number_pages = par->ff_in.numFrames();
  info->number_t     = info->number_pages;
  info->number_z     = 1;
  info->imageMode    = DIM_RGB;
  info->pixelType    = D_FMT_UNSIGNED;

  const char *fmt_name = par->ff_in.formatName();
  const char *cdc_name = par->ff_in.codecName();  

  std::map<std::string, int>::const_iterator it = formats.find( fmt_name );
  if (it != formats.end()) {
    fmtHndl->subFormat = (*it).second;
  }

  return true;
}

//****************************************************************************
//
// FORMAT DEMANDED FUNTIONS
//
//****************************************************************************


//----------------------------------------------------------------------------
// PARAMETERS, INITS
//----------------------------------------------------------------------------

DIM_INT dimFFMpegValidateFormatProc (DIM_MAGIC_STREAM *magic, DIM_UINT length) {
  if (length < 16) return -1;
  unsigned char *mag_num = (unsigned char *) magic;

  //Hex: 00 00 01 Bx : VOB
  if ( (mag_num[0] == 0x00) && (mag_num[1] == 0x00) && (mag_num[2] == 0x01) && ( (mag_num[3]>>4) == 0xB) ) return 0; // mpeg 1,2,4

  //00 00 00 XX 66 74 79 70 : MPEG4, M4V
  if ( (mag_num[0] == 0x00) && (mag_num[1] == 0x00) && (mag_num[2] == 0x00) && //(mag_num[3] == 0x01) &&
       (mag_num[4] == 0x66) && (mag_num[5] == 0x74) && (mag_num[6] == 0x79) && (mag_num[7] == 0x70) ) return 0; // mpeg 4

  // mpeg2 stream M2T
  if ( (mag_num[0] == 0x47) && (mag_num[1] == 0x08) && (mag_num[2] == 0x10) ) return 0; // RAW MPEG2 M2T

  //45 4E 54 52 59 56 43 44 VCD
  if ( (mag_num[0] == 0x45) && (mag_num[1] == 0x4e) && (mag_num[2] == 0x54) && (mag_num[3] == 0x52) &&
       (mag_num[4] == 0x59) && (mag_num[5] == 0x56) && (mag_num[6] == 0x43) && (mag_num[7] == 0x44) ) return 0; // VCD


  if ( (mag_num[0] == 0x52) && (mag_num[1] == 0x49) && (mag_num[2] == 0x46) && (mag_num[3] == 0x46) &&
    (mag_num[8] == 0x41) && (mag_num[9] == 0x56) && (mag_num[10] == 0x49) ) return 5; // avi

  if ( (mag_num[4] == 0x6D) && (mag_num[5] == 0x6F) && (mag_num[6] == 0x6F) && (mag_num[7] == 0x76) ) return 7; // qt mov (moov)
  if ( (mag_num[4] == 0x66) && (mag_num[5] == 0x72) && (mag_num[6] == 0x65) && (mag_num[7] == 0x65) ) return 7; // qt mov (free)
  if ( (mag_num[4] == 0x6D) && (mag_num[5] == 0x64) && (mag_num[6] == 0x61) && (mag_num[7] == 0x74) ) return 7; // qt mov (mdat)
  if ( (mag_num[4] == 0x77) && (mag_num[5] == 0x69) && (mag_num[6] == 0x64) && (mag_num[7] == 0x65) ) return 7; // qt mov (wide)
  if ( (mag_num[4] == 0x70) && (mag_num[5] == 0x6E) && (mag_num[6] == 0x6F) && (mag_num[7] == 0x74) ) return 7; // qt mov (pnot)
  if ( (mag_num[4] == 0x73) && (mag_num[5] == 0x6B) && (mag_num[6] == 0x69) && (mag_num[7] == 0x70) ) return 7; // qt mov (skip)
  if ( (mag_num[4] == 0x66) && (mag_num[5] == 0x74) && (mag_num[6] == 0x79) && (mag_num[7] == 0x70) && 
       (mag_num[8] == 0x71) && (mag_num[9] == 0x74) ) return 7; // qt mov (ftypqt)
  if ( (mag_num[24] == 0x77) && (mag_num[25] == 0x69) && (mag_num[26] == 0x64) && (mag_num[27] == 0x65) ) return 7; // qt mov (wide) futher

  if ( (mag_num[0] == 0x46) && (mag_num[1] == 0x4C) && (mag_num[2] == 0x56) ) return 8; // flash flv
  if ( (mag_num[0] == 0x46) && (mag_num[1] == 0x57) && (mag_num[2] == 0x53) ) return 8; // flash swf
  if ( (mag_num[0] == 0x43) && (mag_num[1] == 0x57) && (mag_num[2] == 0x53) ) return 8; // flash swf
  if ( (mag_num[0] == 0xD0) && (mag_num[1] == 0xCF) && (mag_num[2] == 0x11) && (mag_num[3] == 0xE0) &&
    (mag_num[4] == 0xA1) && (mag_num[5] == 0xB1) && (mag_num[6] == 0x1A) && (mag_num[7] == 0xE1) ) return 8; // fla

  // 30 26 b2 75 8e 66 cf 11 a6 d9 00 aa 00 62 ce 6c
  if ( (mag_num[0] == 0x30) && (mag_num[1] == 0x26) && (mag_num[2] == 0xB2) && (mag_num[3] == 0x75) &&
    (mag_num[4] == 0x8E) && (mag_num[5] == 0x66) && (mag_num[6] == 0xCF) && (mag_num[7] == 0x11) &&
    (mag_num[8] == 0xA6) && (mag_num[9] == 0xD9) && (mag_num[10] == 0x00) && (mag_num[11] == 0xAA) &&
    (mag_num[12] == 0x00) && (mag_num[13] == 0x62) && (mag_num[14] == 0xCE) && (mag_num[15] == 0x6C) ) return 6; // wmv

  if ( (mag_num[0] == 0x00) && (mag_num[1] == 0x00) && (mag_num[2] == 0x00) && (mag_num[3] == 0x0C) &&
    (mag_num[4] == 0x6A) && (mag_num[5] == 0x50) && (mag_num[6] == 0x20) && (mag_num[7] == 0x20) &&
    (mag_num[8] == 0x0D) && (mag_num[9] == 0x0A) && (mag_num[10] == 0x87) && (mag_num[11] == 0x0A) ) return 4; // mj2

  // OGG Hex: 4F 67 67 53 ASCII: OggS
  if ( (mag_num[0] == 0x4F) && (mag_num[1] == 0x67) && (mag_num[2] == 0x67) && (mag_num[3] == 0x53) ) return 9; // OGG

  // Matroska  1A 45 DF A3
  if ( (mag_num[0] == 0x1A) && (mag_num[1] == 0x45) && (mag_num[2] == 0xDF) && (mag_num[3] == 0xA3) ) return 10; // Matroska

  // DV ?
  //if ( (mag_num[0] == 0x1A) && (mag_num[1] == 0x45) && (mag_num[2] == 0xDF) && (mag_num[3] == 0xA3) ) return 11; // DV

  //2E 52 4D 46	 	.RMF
  //RM, RMVB	 	RealMedia streaming media file


  return -1;
}

TDimFormatHandle dimFFMpegAquireFormatProc( void ) {
  TDimFormatHandle fp = initTDimFormatHandle();

  // CODEC_ID_NONE indicates use of ffmpeg default codec
  // for some fomrats we would like to force the codec
  
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_MPEG1VIDEO ); // 0 "MPEG",  - CODEC_ID_MPEG1VIDEO
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_MPEG2VIDEO ); // 1 "MPEG2", - MPEG2VIDEO
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_MPEG4 );      // 2 "MPEG4", - MPEG4
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_MJPEG );      // 3 "MJEPG", - MJPEG
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_NONE );       // 4 "MJEPG2000",
  encoder_ids.push_back( CODEC_ID_MPEG4 );      // 5 "AVI",   - XVID
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_MSMPEG4V3 );  // 6 "WMV",   - WMV3
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_MPEG4 );      // 7 "QT",
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_FLV1 );       // 8 "FLASH",
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_THEORA );     // 9 "Ogg format",
  encoder_ids.push_back( CODEC_ID_MPEG4 );      // 10 "Matroska File Format",
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_DVVIDEO );    // 11 "DV video format",
  encoder_ids.push_back( CODEC_ID_NONE ); //encoder_ids.push_back( CODEC_ID_FLV1 );       // 12 "Flash Video",

  formats_write.push_back( "mpeg" );       // 0 "MPEG",  - MPEG1VIDEO
  formats_write.push_back( "mpeg2video" ); // 1 "MPEG2", - MPEG2VIDEO m2v
  formats_write.push_back( "mp4" );        // 2 "MPEG4", - MPEG4
  formats_write.push_back( "mjpeg" );      // 3 "MJEPG", - MJPEG
  formats_write.push_back( "mjpeg2000" );  // 4 "MJEPG2000",
  formats_write.push_back( "avi" );        // 5 "AVI",   - XVID
  formats_write.push_back( "asf" );        // 6 "WMV",   - WMV3
  formats_write.push_back( "mov" );        // 7 "QT", // "mov,mp4,m4a,3gp,3g2,mj2"
  formats_write.push_back( "swf" );        // 8 "FLASH",
  formats_write.push_back( "ogg" );        // 9 "Ogg format",
  formats_write.push_back( "matroska" );   // 10 "Matroska File Format",
  formats_write.push_back( "dv" );         // 11 "DV video format",
  formats_write.push_back( "flv" );        // 12 "FLV",

  formats_fourcc.push_back( "" );     // 0 "MPEG",  - MPEG1VIDEO
  formats_fourcc.push_back( "" );     // 1 "MPEG2", - MPEG2VIDEO m2v
  formats_fourcc.push_back( "" );     // 2 "MPEG4", - MPEG4
  formats_fourcc.push_back( "" );     // 3 "MJEPG", - MJPEG
  formats_fourcc.push_back( "" );     // 4 "MJEPG2000",
  formats_fourcc.push_back( "XVID" ); // 5 "AVI",   - XVID
  formats_fourcc.push_back( "" );     // 6 "WMV",   - WMV3
  formats_fourcc.push_back( "" );     // 7 "QT", // "mov,mp4,m4a,3gp,3g2,mj2"
  formats_fourcc.push_back( "" );     // 8 "FLASH",
  formats_fourcc.push_back( "" );     // 9 "Ogg format",
  formats_fourcc.push_back( "XVID" ); // 10 "Matroska File Format",
  formats_fourcc.push_back( "" );     // 11 "DV video format",
  formats_fourcc.push_back( "" );     // 12 "Flash video",

  formats.insert( std::make_pair( "mpeg", 0 ) );
  formats.insert( std::make_pair( "mpegvideo", 0 ) );
  formats.insert( std::make_pair( "mpeg1video", 0 ) );
  formats.insert( std::make_pair( "mpegts", 1 ) );
  formats.insert( std::make_pair( "mpegtsraw", 1 ) );
  formats.insert( std::make_pair( "mpeg2video", 1 ) );
  formats.insert( std::make_pair( "m4v", 2 ) );
  formats.insert( std::make_pair( "h264", 2 ) );
  formats.insert( std::make_pair( "mjpeg", 3 ) );
  formats.insert( std::make_pair( "ingenient", 3 ) );
  formats.insert( std::make_pair( "avi", 5 ) );
  formats.insert( std::make_pair( "asf", 6 ) );
  formats.insert( std::make_pair( "mov,mp4,m4a,3gp,3g2,mj2", 7 ) );
  formats.insert( std::make_pair( "swf", 8 ) );
  formats.insert( std::make_pair( "ogg", 9 ) );
  formats.insert( std::make_pair( "matroska", 10 ) );
  formats.insert( std::make_pair( "dv", 11 ) );
  formats.insert( std::make_pair( "flv", 12 ) );

  // if format case is ambiguous, use codecs
  codecs.insert( std::make_pair( "mpeg1video", 0 ) );
  codecs.insert( std::make_pair( "mpeg2video", 1 ) );
  codecs.insert( std::make_pair( "mpeg4", 2 ) );

  return fp;
}

void dimFFMpegReleaseFormatProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  dimFFMpegCloseImageProc ( fmtHndl );  
}


//----------------------------------------------------------------------------
// OPEN/CLOSE
//----------------------------------------------------------------------------
void dimFFMpegSetWriteParameters  (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;  
  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;

  par->frame_sizes_set = false;
  par->fps = 0.0f;
  par->bitrate = 0;

  if (!fmtHndl->options) return;
  xstring str = fmtHndl->options;
  std::vector<xstring> options = str.split( " " );
  if (options.size() < 1) return;
  
  int i = -1;
  while (i<(int)options.size()-1) {
    i++;

    if ( options[i]=="fps" && options.size()-i>0 ) {
      i++;
      par->fps = options[i].toDouble( 0.0 );
      continue;
    }

    if ( options[i]=="bitrate" && options.size()-i>0 ) {
      i++;
      par->bitrate = options[i].toInt( 0 );
      continue;
    }
  } // while

}

void dimFFMpegCloseImageProc (TDimFormatHandle *fmtHndl) {
  if (fmtHndl == NULL) return;
  if (fmtHndl->internalParams == NULL) return;  
  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;
  fmtHndl->internalParams = NULL;
  par->ff_in.close();
  par->ff_out.close();
  delete par;
}

DIM_UINT dimFFMpegOpenImageProc  (TDimFormatHandle *fmtHndl, DIM_ImageIOModes io_mode) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams != NULL) dimFFMpegCloseImageProc (fmtHndl);  
  
  DFFMpegParams *par = new DFFMpegParams();
  fmtHndl->internalParams = (void *) par;

  if ( io_mode == DIM_IO_READ ) {
    //if ( isCustomReading ( fmtHndl ) != TRUE )
    //  fmtHndl->stream = /*(void *)*/ fopen( fmtHndl->fileName, "rb" );

    VideoIO::KeyValueMap kvm;
    std::string filename = fmtHndl->fileName;
    kvm["filename"] = filename;
    par->ff_in.setConvertToMatlab(false);
    par->ff_in.open( kvm );
    if ( !ffmpegGetImageInfo( fmtHndl ) ) { dimFFMpegCloseImageProc (fmtHndl); return 1; };
  }

  if ( io_mode == DIM_IO_WRITE ) {
    //if ( isCustomWriting ( fmtHndl ) != TRUE )
    //  fmtHndl->stream = /*(void *)*/ fopen( fmtHndl->fileName, "wb" );
    dimFFMpegSetWriteParameters(fmtHndl);
  }

  return 0;
}


//----------------------------------------------------------------------------
// INFO for OPEN image
//----------------------------------------------------------------------------

DIM_UINT dimFFMpegGetNumPagesProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 0;
  if (fmtHndl->internalParams == NULL) return 0;
  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;
  TDimImageInfo *info = &par->i;    
  return info->number_pages;
}


TDimImageInfo dimFFMpegGetImageInfoProc ( TDimFormatHandle *fmtHndl, DIM_UINT /*page_num*/ ) {
  TDimImageInfo ii = initTDimImageInfo();
  if (fmtHndl == NULL) return ii;
  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;
  return par->i;
}

//----------------------------------------------------------------------------
// METADATA
//----------------------------------------------------------------------------

DIM_UINT dimFFMpegAddMetaDataProc (TDimFormatHandle *fmtHndl) {
  fmtHndl=fmtHndl;
  return 1;
}


DIM_UINT dimFFMpegReadMetaDataProc (TDimFormatHandle *fmtHndl, DIM_UINT page, int group, int tag, int type) {
  fmtHndl; page; group; tag; type;
  return 1;
}

char* dimFFMpegReadMetaDataAsTextProc ( TDimFormatHandle * /*fmtHndl*/ ) {
  return NULL;
}


//----------------------------------------------------------------------------
// READ/WRITE
//----------------------------------------------------------------------------

DIM_UINT dimFFMpegReadImageProc  ( TDimFormatHandle *fmtHndl, DIM_UINT page ) {
  if (fmtHndl == NULL) return 1;
  fmtHndl->pageNumber = page;
  return read_ffmpeg_image( fmtHndl );
}

DIM_UINT dimFFMpegWriteImageProc ( TDimFormatHandle *fmtHndl ) {
  if (fmtHndl == NULL) return 1;
  try {
    return write_ffmpeg_image( fmtHndl );
  } catch (...) {
    return 1;
  }
}

//----------------------------------------------------------------------------
// META DATA PROC
//----------------------------------------------------------------------------

DIM_UINT ffmpeg_append_metadata (TDimFormatHandle *fmtHndl, DTagMap *hash ) {

  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;

  DFFMpegParams *par = (DFFMpegParams *) fmtHndl->internalParams;
  hash->append_tag( "format_name", par->ff_in.formatName() );
  hash->append_tag( "codec_name",  par->ff_in.codecName() );
  hash->append_tag( "frames_per_second",  par->ff_in.fps() );
  hash->append_tag( "pixel_resolution_t",  (1.0/(double)par->ff_in.fps()) );
  hash->append_tag( "pixel_resolution_unit_t",  "seconds" );

  return 0;
}


//****************************************************************************
//
// EXPORTED FUNCTION
//
//****************************************************************************

#define DIM_FFMPEG_NUM_FORMATS 13

TDimFormatItem dimFFMpegItems[DIM_FFMPEG_NUM_FORMATS] = {
  {
    "MPEG",      // short name, no spaces
      "MPEG I", // Long format name
      "mpg|mpeg|m1v",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "MPEG2",            // short name, no spaces
      "MPEG 2", // Long format name
      "mpg|mpeg|m2v|m2t",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "MPEG4",            // short name, no spaces
      "MPEG 4", // Long format name
      "m4v",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "MJPEG",            // short name, no spaces
      "Motion JPEG", // Long format name
      "mjpeg|mjp|mjpg",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "MJPEG2000",            // short name, no spaces
      "Motion JPEG-2000", // Long format name
      "mj2|mjp2",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      0, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      0, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "AVI",            // short name, no spaces
      "Microsoft Windows AVI", // Long format name
      "avi",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "WMV",            // short name, no spaces
      "Microsoft Windows Media Video", // Long format name
      "wmv|asf",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "QuickTime",            // short name, no spaces
      "Apple QuickTime", // Long format name
      "mov",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "Flash",            // short name, no spaces
      "Adobe Flash", // Long format name
      "swf",        // pipe "|" separated supported extension list
      0, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "OGG",            // short name, no spaces
      "OGG Theora Video Format", // Long format name
      "ogg|ogv",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      0, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      0, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "Matroska",            // short name, no spaces
      "Matroska Multimedia Container", // Long format name
      "mkv",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "DV",            // short name, no spaces
      "Digital Video", // Long format name
      "dv",        // pipe "|" separated supported extension list
      1, //canRead;      // 0 - NO, 1 - YES
      0, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      0, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  },
  {
    "FLV",            // short name, no spaces
      "Adobe Flash Video", // Long format name
      "flv",        // pipe "|" separated supported extension list
      0, //canRead;      // 0 - NO, 1 - YES
      1, //canWrite;     // 0 - NO, 1 - YES
      0, //canReadMeta;  // 0 - NO, 1 - YES
      0, //canWriteMeta; // 0 - NO, 1 - YES
      1, //canWriteMultiPage;   // 0 - NO, 1 - YES
      //TDivFormatConstrains constrains ( w, h, pages, minsampl, maxsampl, minbitsampl, maxbitsampl, noLut )
    { 0, 0, 1, 1, 3, 1, 8, 0 } 
  }                
};

TDimFormatHeader dimFFMpegHeader = {

  sizeof(TDimFormatHeader),
  "1.0.4",
  "FFMPEG CODEC",
  "FFMPEG CODEC",

  20,                     // 0 or more, specify number of bytes needed to identify the file
  {1, DIM_FFMPEG_NUM_FORMATS, dimFFMpegItems},   // 

  dimFFMpegValidateFormatProc,
  // begin
  dimFFMpegAquireFormatProc, //TDimAquireFormatProc
  // end
  dimFFMpegReleaseFormatProc, //TDimReleaseFormatProc

  // params
  NULL, //TDimAquireIntParamsProc
  NULL, //TDimLoadFormatParamsProc
  NULL, //TDimStoreFormatParamsProc

  // image begin
  dimFFMpegOpenImageProc, //TDimOpenImageProc
  dimFFMpegCloseImageProc, //TDimCloseImageProc 

  // info
  dimFFMpegGetNumPagesProc, //TDimGetNumPagesProc
  dimFFMpegGetImageInfoProc, //TDimGetImageInfoProc


  // read/write
  dimFFMpegReadImageProc, //TDimReadImageProc 
  dimFFMpegWriteImageProc, //TDimWriteImageProc
  NULL, //TDimReadImageTileProc
  NULL, //TDimWriteImageTileProc
  NULL, //TDimReadImageLineProc
  NULL, //TDimWriteImageLineProc
  NULL, //TDimReadImageThumbProc
  NULL, //TDimWriteImageThumbProc
  NULL, //dimJpegReadImagePreviewProc, //TDimReadImagePreviewProc

  // meta data
  dimFFMpegReadMetaDataProc, //TDimReadMetaDataProc
  dimFFMpegAddMetaDataProc,  //TDimAddMetaDataProc
  dimFFMpegReadMetaDataAsTextProc, //TDimReadMetaDataAsTextProc
  ffmpeg_append_metadata, //TDimAppendMetaDataProc

  NULL,
  NULL,
  ""

};

extern "C" {

  TDimFormatHeader* dimFFMpegGetFormatHeader(void)
  {
    return &dimFFMpegHeader;
  }

} // extern C


