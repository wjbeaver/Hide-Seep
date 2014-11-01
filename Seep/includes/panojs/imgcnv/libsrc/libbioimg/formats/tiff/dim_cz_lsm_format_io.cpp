/*****************************************************************************
  Carl Zeiss LSM IO 
  Copyright (c) 2006 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation
        
  Ver : 1
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <algorithm>

#include "dim_tiff_format.h"
#include "dim_xtiffio.h"
#include "xstring.h"

void read_text_tag(TIFF *tif, TDimTiffIFD *ifd, DIM_UINT tag, MemIOBuf *outIOBuf);


//----------------------------------------------------------------------------
// PSIA MISC FUNCTIONS
//----------------------------------------------------------------------------

void initMetaHash(DLsmInfo *lsm) {
  if (lsm->key_names.size()>0) return;
  lsm->key_names[0x10000001] = "Name";
  lsm->key_names[0x4000000c] = "Name";
  lsm->key_names[0x50000001] = "Name";
  lsm->key_names[0x90000001] = "Name";
  lsm->key_names[0x90000005] = "Detection Channel Name";
  lsm->key_names[0xb0000003] = "Name";
  lsm->key_names[0xd0000001] = "Name";
  lsm->key_names[0x12000001] = "Name";
  lsm->key_names[0x14000001] = "Name";
  lsm->key_names[0x10000002] = "Description";
  lsm->key_names[0x14000002] = "Description";
  lsm->key_names[0x10000003] = "Notes";
  lsm->key_names[0x10000004] = "Objective";
  lsm->key_names[0x10000005] = "Processing Summary";
  lsm->key_names[0x10000006] = "Special Scan Mode";
  lsm->key_names[0x10000007] = "Scan Type";
  lsm->key_names[0x10000008] = "Scan Mode";
  lsm->key_names[0x10000009] = "Number of Stacks";
  lsm->key_names[0x1000000a] = "Lines Per Plane";
  lsm->key_names[0x1000000b] = "Samples Per Line";
  lsm->key_names[0x1000000c] = "Planes Per Volume";
  lsm->key_names[0x1000000d] = "Images Width";
  lsm->key_names[0x1000000e] = "Images Height";
  lsm->key_names[0x1000000f] = "Number of Planes";
  lsm->key_names[0x10000010] = "Number of Stacks";
  lsm->key_names[0x10000011] = "Number of Channels";
  lsm->key_names[0x10000012] = "Linescan XY Size";
  lsm->key_names[0x10000013] = "Scan Direction";
  lsm->key_names[0x10000014] = "Time Series";
  lsm->key_names[0x10000015] = "Original Scan Data";
  lsm->key_names[0x10000016] = "Zoom X";
  lsm->key_names[0x10000017] = "Zoom Y";
  lsm->key_names[0x10000018] = "Zoom Z";
  lsm->key_names[0x10000019] = "Sample 0X";
  lsm->key_names[0x1000001a] = "Sample 0Y";
  lsm->key_names[0x1000001b] = "Sample 0Z";
  lsm->key_names[0x1000001c] = "Sample Spacing";
  lsm->key_names[0x1000001d] = "Line Spacing";
  lsm->key_names[0x1000001e] = "Plane Spacing";
  lsm->key_names[0x1000001f] = "Plane Width";
  lsm->key_names[0x10000020] = "Plane Height";
  lsm->key_names[0x10000021] = "Volume Depth";
  lsm->key_names[0x10000034] = "Rotation";
  lsm->key_names[0x10000035] = "Precession";
  lsm->key_names[0x10000036] = "Sample 0Time";
  lsm->key_names[0x10000037] = "Start Scan Trigger In";
  lsm->key_names[0x10000038] = "Start Scan Trigger Out";
  lsm->key_names[0x10000039] = "Start Scan Event";
  lsm->key_names[0x10000040] = "Start Scan Time";
  lsm->key_names[0x10000041] = "Stop Scan Trigger In";
  lsm->key_names[0x10000042] = "Stop Scan Trigger Out";
  lsm->key_names[0x10000043] = "Stop Scan Event";
  lsm->key_names[0x10000044] = "Stop Scan Time";
  lsm->key_names[0x10000045] = "Use ROIs";
  lsm->key_names[0x10000046] = "Use Reduced Memory ROIs";
  lsm->key_names[0x10000047] = "User";
  lsm->key_names[0x10000048] = "Use B|C Correction";
  lsm->key_names[0x10000049] = "Position B|C Contrast 1";
  lsm->key_names[0x10000050] = "Position B|C Contrast 2";
  lsm->key_names[0x10000051] = "Interpolation Y";
  lsm->key_names[0x10000052] = "Camera Binning";
  lsm->key_names[0x10000053] = "Camera Supersampling";
  lsm->key_names[0x10000054] = "Camera Frame Width";
  lsm->key_names[0x10000055] = "Camera Frame Height";
  lsm->key_names[0x10000056] = "Camera Offset X";
  lsm->key_names[0x10000057] = "Camera Offset Y";
  lsm->key_names[0x40000001] = "Multiplex Type";
  lsm->key_names[0x40000002] = "Multiplex Order";
  lsm->key_names[0x40000003] = "Sampling Mode";
  lsm->key_names[0x40000004] = "Sampling Method";
  lsm->key_names[0x40000005] = "Sampling Number";
  lsm->key_names[0x40000006] = "Acquire";
  lsm->key_names[0x50000002] = "Acquire";
  lsm->key_names[0x7000000b] = "Acquire";
  lsm->key_names[0x90000004] = "Acquire";
  lsm->key_names[0xd0000017] = "Acquire";
  lsm->key_names[0x40000007] = "Sample Observation Time";
  lsm->key_names[0x40000008] = "Time Between Stacks";
  lsm->key_names[0x4000000d] = "Collimator 1 Name";
  lsm->key_names[0x4000000e] = "Collimator 1 Position";
  lsm->key_names[0x4000000f] = "Collimator 2 Name";
  lsm->key_names[0x40000010] = "Collimator 2 Position";
  lsm->key_names[0x40000011] = "Is Bleach Track";
  lsm->key_names[0x40000012] = "Bleach After Scan Number";
  lsm->key_names[0x40000013] = "Bleach Scan Number";
  lsm->key_names[0x40000014] = "Trigger In";
  lsm->key_names[0x12000004] = "Trigger In";
  lsm->key_names[0x14000003] = "Trigger In";
  lsm->key_names[0x40000015] = "Trigger Out";
  lsm->key_names[0x12000005] = "Trigger Out";
  lsm->key_names[0x14000004] = "Trigger Out";
  lsm->key_names[0x40000016] = "Is Ratio Track";
  lsm->key_names[0x40000017] = "Bleach Count";
  lsm->key_names[0x40000018] = "SPI Center Wavelength";
  lsm->key_names[0x40000019] = "Pixel Time";
  lsm->key_names[0x40000020] = "ID Condensor Frontlens";
  lsm->key_names[0x40000021] = "Condensor Frontlens";
  lsm->key_names[0x40000022] = "ID Field Stop";
  lsm->key_names[0x40000023] = "Field Stop Value";
  lsm->key_names[0x40000024] = "ID Condensor Aperture";
  lsm->key_names[0x40000025] = "Condensor Aperture";
  lsm->key_names[0x40000026] = "ID Condensor Revolver";
  lsm->key_names[0x40000027] = "Condensor Revolver";
  lsm->key_names[0x40000028] = "ID Transmission Filter 1";
  lsm->key_names[0x40000029] = "ID Transmission 1";
  lsm->key_names[0x40000030] = "ID Transmission Filter 2";
  lsm->key_names[0x40000031] = "ID Transmission 2";
  lsm->key_names[0x40000032] = "Repeat Bleach";
  lsm->key_names[0x40000033] = "Enable Spot Bleach Pos";
  lsm->key_names[0x40000034] = "Spot Bleach Position X";
  lsm->key_names[0x40000035] = "Spot Bleach Position Y";
  lsm->key_names[0x40000036] = "Bleach Position Z";
  lsm->key_names[0x50000003] = "Power";
  lsm->key_names[0x90000002] = "Power";
  lsm->key_names[0x70000003] = "Detector Gain";
  lsm->key_names[0x70000005] = "Amplifier Gain";
  lsm->key_names[0x70000007] = "Amplifier Offset";
  lsm->key_names[0x70000009] = "Pinhole Diameter";
  lsm->key_names[0x7000000c] = "Detector Name";
  lsm->key_names[0x7000000d] = "Amplifier Name";
  lsm->key_names[0x7000000e] = "Pinhole Name";
  lsm->key_names[0x7000000f] = "Filter Set Name";
  lsm->key_names[0x70000010] = "Filter Name";
  lsm->key_names[0x70000013] = "Integrator Name";
  lsm->key_names[0x70000014] = "Detection Channel Name";
  lsm->key_names[0x70000015] = "Detector Gain B|C 1";
  lsm->key_names[0x70000016] = "Detector Gain B|C 2";
  lsm->key_names[0x70000017] = "Amplifier Gain B|C 1";
  lsm->key_names[0x70000018] = "Amplifier Gain B|C 2";
  lsm->key_names[0x70000019] = "Amplifier Offset B|C 1";
  lsm->key_names[0x70000020] = "Amplifier Offset B|C 2";
  lsm->key_names[0x70000021] = "Spectral Scan Channels";
  lsm->key_names[0x70000022] = "SPI Wavelength Start";
  lsm->key_names[0x70000023] = "SPI Wavelength End";
  lsm->key_names[0x70000026] = "Dye Name";
  lsm->key_names[0xd0000014] = "Dye Name";
  lsm->key_names[0x70000027] = "Dye Folder";
  lsm->key_names[0xd0000015] = "Dye Folder";
  lsm->key_names[0x90000003] = "Wavelength";
  lsm->key_names[0x90000006] = "Power B|C 1";
  lsm->key_names[0x90000007] = "Power B|C 2";
  lsm->key_names[0xb0000001] = "Filter Set";
  lsm->key_names[0xb0000002] = "Filter";
  lsm->key_names[0xd0000004] = "Color";
  lsm->key_names[0xd0000005] = "Sample Type";
  lsm->key_names[0xd0000006] = "Bits Per Sample";
  lsm->key_names[0xd0000007] = "Ratio Type";
  lsm->key_names[0xd0000008] = "Ratio Track 1";
  lsm->key_names[0xd0000009] = "Ratio Track 2";
  lsm->key_names[0xd000000a] = "Ratio Channel 1";
  lsm->key_names[0xd000000b] = "Ratio Channel 2";
  lsm->key_names[0xd000000c] = "Ratio Const. 1";
  lsm->key_names[0xd000000d] = "Ratio Const. 2";
  lsm->key_names[0xd000000e] = "Ratio Const. 3";
  lsm->key_names[0xd000000f] = "Ratio Const. 4";
  lsm->key_names[0xd0000010] = "Ratio Const. 5";
  lsm->key_names[0xd0000011] = "Ratio Const. 6";
  lsm->key_names[0xd0000012] = "Ratio First Images 1";
  lsm->key_names[0xd0000013] = "Ratio First Images 2";
  lsm->key_names[0xd0000016] = "Spectrum";
  lsm->key_names[0x12000003] = "Interval";
}

//----------------------------------------------------------------------------
// PSIA MISC FUNCTIONS
//----------------------------------------------------------------------------

bool lsmIsTiffValid(TIFF *tif) {
  if (tif->tif_flags&TIFF_BIGTIFF) return false;      
  char *b_list = NULL;
  int16   d_list_count;
  int res[3] = {0,0,0};

  if (tif == 0) return FALSE;
  res[0] = TIFFGetField(tif, TIFFTAG_CZ_LSMINFO, &d_list_count, &b_list);

  if (res[0] == 1) return TRUE;
  return FALSE;
}


bool lsmIsTiffValid(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return false;
  if (tiffParams->dimTiff->tif_flags&TIFF_BIGTIFF) return false;    
  if (isTagPresentInFirstIFD( &tiffParams->ifds, TIFFTAG_CZ_LSMINFO ) == TRUE) return TRUE;
  return FALSE;
}

void doSwabLSMINFO(CZ_LSMINFO *b) {
  
  TIFFSwabArrayOfLong( (uint32*) &b->u32MagicNumber, 10 );
  TIFFSwabArrayOfDouble( (double*) &b->f64VoxelSizeX, 3 );
  TIFFSwabArrayOfLong( (uint32*) &b->u32ScanType, 6 );
  TIFFSwabDouble( (double*) &b->f64TimeIntervall );
  TIFFSwabArrayOfLong( (uint32*) &b->u32OffsetChannelDataTypes, 8 );
}

void doSwabLSMCOLORS(CZ_ChannelColors *b) {
  TIFFSwabArrayOfLong( (uint32*) &b->s32BlockSize, 6 );
}

void lsm_read_ScanInformation (DTiffParams *tiffParams);

#include <iostream>
#include <fstream>

int lsmGetInfo (DTiffParams *tiffParams) {
  if (tiffParams == NULL) return 1;
  if (tiffParams->dimTiff == NULL) return 1;
  if (tiffParams->ifds.count <= 0) return 1;

  TDimImageInfo *info = &tiffParams->info;
  DLsmInfo *lsm = &tiffParams->lsmInfo;
  CZ_LSMINFO *lsmi = &lsm->lsm_info;


  lsm->pages_tiff = tiffParams->info.number_pages;
  lsm->pages = tiffParams->info.number_pages / 2;
  tiffParams->info.number_pages = lsm->pages;

  lsm->ch = tiffParams->info.samples;
  if (tiffParams->info.samples > 1) 
    tiffParams->info.imageMode = DIM_RGB;
  else
    tiffParams->info.imageMode = DIM_GRAYSCALE;    

  uint16 bitspersample = 1;  
  TIFFGetField(tiffParams->dimTiff, TIFFTAG_BITSPERSAMPLE, &bitspersample);  
  tiffParams->info.depth = bitspersample;

  // ------------------------------------------
  // get LSM INFO STRUCTURE
  // ------------------------------------------

  DIM_UCHAR *buf = NULL;
  uint32 size, type;
  if (!isTagPresentInFirstIFD( &tiffParams->ifds, TIFFTAG_CZ_LSMINFO )) return 1;
  readTiffTag (tiffParams->dimTiff, &tiffParams->ifds.ifds[0], TIFFTAG_CZ_LSMINFO, size, type, &buf);
  if ( (size <= 0) || (buf == NULL) ) return 1;
  if (bigendian) doSwabLSMINFO((CZ_LSMINFO *) buf);

  lsm->lsm_info = * (CZ_LSMINFO *) buf;
  freeTiffTagBuf( &buf );

  //---------------------------------------------------------------
  // read CZ_ChannelColors
  //---------------------------------------------------------------
  memset( &lsm->lsm_colors, 0, sizeof(CZ_ChannelColors) );
  if (lsm->lsm_info.u32OffsetChannelColors > 0) {
    readTiffBufNoAlloc( tiffParams->dimTiff, lsm->lsm_info.u32OffsetChannelColors, sizeof(CZ_ChannelColors), TIFF_LONG, (unsigned char *) &lsm->lsm_colors );

    // here we have to assign channel names reading them from the file, but never seen this used...
  }

  //---------------------------------------------------------------
  // read CZ_ScanInformation
  //---------------------------------------------------------------
  lsm_read_ScanInformation (tiffParams);


  //---------------------------------------------------------------
  // read CZ_TimeStamps
  //---------------------------------------------------------------
  memset( &lsm->lsm_TimeStamps, 0, sizeof(CZ_TimeStamps) );
  if (lsm->lsm_info.u32OffsetTimeStamps > 0) {
    readTiffBufNoAlloc( tiffParams->dimTiff, lsm->lsm_info.u32OffsetTimeStamps, sizeof(CZ_TimeStamps), TIFF_LONG, (unsigned char *) &lsm->lsm_TimeStamps );

    // here we have to assign channel names reading them from the file, but never seen this used...
  }


  //---------------------------------------------------------------
  // retreive meta-data
  //---------------------------------------------------------------
  lsm->ch       = lsmi->s32DimensionChannels;
  lsm->t_frames = lsmi->s32DimensionTime;
  lsm->z_slices = lsmi->s32DimensionZ;

  lsm->res[0] = lsmi->f64VoxelSizeX; // x in meters
  lsm->res[1] = lsmi->f64VoxelSizeY; // y in meters
  lsm->res[2] = lsmi->f64VoxelSizeZ; // z in meters
  lsm->res[3] = lsmi->f64TimeIntervall; // t in seconds

  //---------------------------------------------------------------
  // define dims
  //---------------------------------------------------------------
  info->number_z = lsm->z_slices;
  info->number_t = lsm->t_frames;

  if (lsm->z_slices > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_Z;
  }

  if (lsm->t_frames > 1) {
    info->number_dims = 4;
    info->dimensions[3].dim = DIM_DIM_T;
  }

  if ((lsm->z_slices > 1) && (lsm->t_frames > 1)) {
    info->number_dims = 5;
    info->dimensions[3].dim = DIM_DIM_Z;        
    info->dimensions[4].dim = DIM_DIM_T;
  }

  return 0;
}

void lsmGetCurrentPageInfo(DTiffParams *tiffParams) {
  if (tiffParams == NULL) return;
  TDimImageInfo *info = &tiffParams->info;

  if ( tiffParams->subType == tstCzLsm ) {
    DLsmInfo *lsm = &tiffParams->lsmInfo;  
    info->resUnits = DIM_RES_um;
    info->xRes = lsm->res[0] * 1000000;
    info->yRes = lsm->res[1] * 1000000;
  }

}



//----------------------------------------------------------------------------
// READ CZ_ScanInformation
//----------------------------------------------------------------------------

inline  unsigned int DLsmScanInfoEntry::offsetSize() const {
  unsigned int size = sizeof( CZ_ScanInformation );
  size += this->data.size();
  return size;
}

int DLsmScanInfoEntry::readEntry (TIFF *tif, uint32 offset) {

  CZ_ScanInformation si;
  int size = sizeof(CZ_ScanInformation);
  tif->tif_seekproc((thandle_t) tif->tif_fd, offset, SEEK_SET);  
  if (tif->tif_readproc((thandle_t) tif->tif_fd, &si, size) < (int) size) return 1;

  if (bigendian) {
    TIFFSwabArrayOfLong( (uint32*) &si.u32Entry, 1 );
    TIFFSwabArrayOfLong( (uint32*) &si.u32Type, 1 );
    TIFFSwabArrayOfLong( (uint32*) &si.u32Size, 1 );
  }

  this->entry_type = si.u32Entry;
  this->data_type  = si.u32Type;

  if (si.u32Size>0) {
    size = si.u32Size;
    this->data.resize(size);
    if (tif->tif_readproc((thandle_t) tif->tif_fd, &this->data[0], size) < (int) size) return 1;
  }
  return 0;
}

void lsm_read_ScanInformation (DTiffParams *tiffParams) {
  if (tiffParams == NULL) return;
  if (tiffParams->dimTiff == NULL) return;
  if (tiffParams->ifds.count <= 0) return;

  TDimImageInfo *info = &tiffParams->info;
  DLsmInfo *lsm = &tiffParams->lsmInfo;
  CZ_LSMINFO *lsmi = &lsm->lsm_info;

  //---------------------------------------------------------------
  // read CZ_ScanInformation
  //---------------------------------------------------------------
  std::vector< std::string > path;
  int block_track=1, block_laser=1, block_detection_channel=1, block_illumination_channel=1, 
      block_beam_plitter=1, block_data_channel=1, block_timer=1, block_marker=1;

  int level = 0;
  lsm->scan_info_entries.clear();
  //unsigned int offset = lsm->lsm_info.u32OffsetScanInformation;
  unsigned int offset = lsm->lsm_info.u32OffsetNextRecording;  //dima: this is a hack... dunno why
  while (offset > 0) {
    DLsmScanInfoEntry block;
    if (block.readEntry (tiffParams->dimTiff, offset)!=0) break;

    if (block.data_type == TYPE_SUBBLOCK) {

      if (block.entry_type == SUBBLOCK_RECORDING) { level++; path.push_back("Recording"); }
      if (block.entry_type == SUBBLOCK_END) {level--; path.pop_back(); }

      if (block.entry_type == SUBBLOCK_LASERS) { level++; path.push_back("Lasers"); }
      if (block.entry_type == SUBBLOCK_TRACKS) { level++; path.push_back("Tracks"); }
      if (block.entry_type == SUBBLOCK_DETECTION_CHANNELS) { level++; path.push_back("Detection channels"); }
      if (block.entry_type == SUBBLOCK_ILLUMINATION_CHANNELS) { level++; path.push_back("Illumination channels"); }
      if (block.entry_type == SUBBLOCK_BEAM_SPLITTERS) { level++; path.push_back("Beam splitters"); }
      if (block.entry_type == SUBBLOCK_DATA_CHANNELS) { level++; path.push_back("Data channels"); }
      if (block.entry_type == SUBBLOCK_TIMERS) { level++; path.push_back("Timers"); }
      if (block.entry_type == SUBBLOCK_MARKERS) { level++; path.push_back("Markers"); }

      xstring s;
      if (block.entry_type == SUBBLOCK_TRACK) { level++; s.sprintf("Track%d", block_track++); path.push_back(s); }
      if (block.entry_type == SUBBLOCK_LASER) { level++; s.sprintf("Laser%d", block_laser++); path.push_back(s); }
      if (block.entry_type == SUBBLOCK_DETECTION_CHANNEL) { level++; s.sprintf("Detection channel%d", block_detection_channel++); path.push_back(s); }
      if (block.entry_type == SUBBLOCK_ILLUMINATION_CHANNEL) { level++; s.sprintf("Illumination channel%d", block_illumination_channel++); path.push_back(s); }
      if (block.entry_type == SUBBLOCK_BEAM_SPLITTER) { level++; s.sprintf("Beam splitter%d", block_beam_plitter++); path.push_back(s); }
      if (block.entry_type == SUBBLOCK_DATA_CHANNEL) { level++; s.sprintf("Data channel%d", block_data_channel++); path.push_back(s); }
      if (block.entry_type == SUBBLOCK_TIMER) { level++; s.sprintf("Timer%d", block_timer++); path.push_back(s); }
      if (block.entry_type == SUBBLOCK_MARKER) { level++; s.sprintf("Marker%d", block_marker++); path.push_back(s); }
      
      offset += 12;
    }
    else 
      offset += block.offsetSize();

    if (level > 0) {  
      xstring path_str;
      if (path.size() > 0)
      for (int i=0; i<path.size(); ++i) {
        path_str += path[i];
        path_str += "/";
      }
      block.path = path_str;
      lsm->scan_info_entries.push_back( block );
    }

    if (level == 0) break;
  }

}

//----------------------------------------------------------------------------
// READ/WRITE FUNCTIONS
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// METADATA FUNCTIONS
//----------------------------------------------------------------------------

DIM_UINT append_metadata_lsm (TDimFormatHandle *fmtHndl, DTagMap *hash ) {
  if (fmtHndl == NULL) return 1;
  if (fmtHndl->internalParams == NULL) return 1;
  if (!hash) return 1;

  DTiffParams *tiffParams = (DTiffParams *) fmtHndl->internalParams;
  if (tiffParams == NULL) return 1;
  DLsmInfo *lsm = &tiffParams->lsmInfo;
  CZ_LSMINFO *lsmi = &lsm->lsm_info;

  hash->append_tag( bim::IMAGE_NUM_Z, lsm->z_slices );
  hash->append_tag( bim::IMAGE_NUM_T, lsm->t_frames );
  hash->append_tag( bim::IMAGE_NUM_C, lsm->ch );

  hash->append_tag( bim::PIXEL_RESOLUTION_X, lsm->res[0]*1000000 );
  hash->append_tag( bim::PIXEL_RESOLUTION_Y, lsm->res[1]*1000000 );
  hash->append_tag( bim::PIXEL_RESOLUTION_Z, lsm->res[2]*1000000 );
  hash->append_tag( bim::PIXEL_RESOLUTION_T, lsm->res[3] );

  hash->set_value( bim::PIXEL_RESOLUTION_UNIT_X, bim::PIXEL_RESOLUTION_UNIT_MICRONS );
  hash->set_value( bim::PIXEL_RESOLUTION_UNIT_Y, bim::PIXEL_RESOLUTION_UNIT_MICRONS );
  hash->set_value( bim::PIXEL_RESOLUTION_UNIT_Z, bim::PIXEL_RESOLUTION_UNIT_MICRONS );
  hash->set_value( bim::PIXEL_RESOLUTION_UNIT_T, bim::PIXEL_RESOLUTION_UNIT_SECONDS );


  // All other tags in custom field
  initMetaHash(lsm);
  xstring key, val;
  for (int i=0; i<lsm->scan_info_entries.size(); i++) {
    if (lsm->scan_info_entries[i].data.size()<=0) continue;
    if (lsm->key_names[lsm->scan_info_entries[i].entry_type]=="") continue;

    key = xstring(bim::CUSTOM_TAGS_PREFIX) + lsm->scan_info_entries[i].path + lsm->key_names[lsm->scan_info_entries[i].entry_type];

    if (lsm->scan_info_entries[i].data_type == TYPE_ASCII && lsm->scan_info_entries[i].data.size()>0) {
      char* line = (char*) &lsm->scan_info_entries[i].data[0];
      val = line;
    } else
    if (lsm->scan_info_entries[i].data_type == TYPE_LONG && lsm->scan_info_entries[i].data.size()>0) {
      val.sprintf("%d", *(int*) &lsm->scan_info_entries[i].data[0]);
    } else
    if (lsm->scan_info_entries[i].data_type == TYPE_RATIONAL && lsm->scan_info_entries[i].data.size()>0) {
      val.sprintf("%f", *(double*) &lsm->scan_info_entries[i].data[0]);
    }

    hash->append_tag( key, val );

    //if (lsm->scan_info_entries[i].entry_type == 0x90000001) lsm->channel_names.push_back(val);
    if (lsm->scan_info_entries[i].entry_type == 0x10000004) hash->append_tag( bim::OBJECTIVE_DESCRIPTION, val );
    if (lsm->scan_info_entries[i].entry_type == 0x10000001) hash->append_tag( bim::IMAGE_LABEL, val );
  }

  // objective
  /*
  std::map< std::string, std::string >::const_iterator it = hash->find("custom/Recording/Objective");
  if (it != hash->end() )
    hash->append_tag( "objective", it->second );

  // name
  it = hash->find("custom/Recording/Name");
  if (it != hash->end() )
    hash->append_tag( "label", it->second );
    */

  //----------------------------------------------------------------------------
  // Channel names and preferred mapping
  //----------------------------------------------------------------------------
  TDimImageInfo *info = &tiffParams->info;
  for (unsigned int i=0; i<std::min<size_t>(info->samples, lsm->channel_names.size()); ++i) {
    xstring tag_name;
    tag_name.sprintf( bim::CHANNEL_NAME_TEMPLATE.c_str(), i );
    hash->append_tag( tag_name, lsm->channel_names[i] );
  }
  
  /*
  if ( fvi->display_lut.size() == 3 ) {
    hash->append_tag( "display_channel_red",   fvi->display_lut[0] );
    hash->append_tag( "display_channel_green", fvi->display_lut[1] );
    hash->append_tag( "display_channel_blue",  fvi->display_lut[2] );
  }*/


  return 0;
}

