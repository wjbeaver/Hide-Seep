/*****************************************************************************
  Carl Zeiss LSM definitions 
  Copyright (c) 2006 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation

  Ver : 1
*****************************************************************************/

#ifndef DIM_CZ_LSM_FORMAT_H
#define DIM_CZ_LSM_FORMAT_H

#include <vector>
#include <map>

#include <dim_img_format_interface.h>

//----------------------------------------------------------------------------
// Internal LSM Structures
//----------------------------------------------------------------------------

#define UINT32 DIM_UINT32
#define SINT32 DIM_INT32
#define FLOAT64 DIM_FLOAT64


#pragma pack(push, 1)
typedef struct CZ_LSMINFO {
  UINT32   u32MagicNumber;
  SINT32   s32StructureSize;
  SINT32   s32DimensionX;              
  SINT32   s32DimensionY;
  SINT32   s32DimensionZ;
  SINT32   s32DimensionChannels;
  SINT32   s32DimensionTime;
  SINT32   s32DataType;                
  SINT32   s32ThumbnailX;               
  SINT32   s32ThumbnailY;
  FLOAT64  f64VoxelSizeX;               
  FLOAT64  f64VoxelSizeY;               
  FLOAT64  f64VoxelSizeZ;              
  UINT32   u32ScanType;
  UINT32   u32DataType;
  UINT32   u32OffsetVectorOverlay;
  UINT32   u32OffsetInputLut;
  UINT32   u32OffsetOutputLut;
  UINT32   u32OffsetChannelColors; // points to CZ_ChannelColors
  FLOAT64  f64TimeIntervall;
  UINT32   u32OffsetChannelDataTypes;
  UINT32   u32OffsetScanInformation; // points to CZ_ScanInformation
  UINT32   u32OffsetKsData;
  UINT32   u32OffsetTimeStamps; // points to CZ_TimeStamps
  UINT32   u32OffsetEventList;
  UINT32   u32OffsetRoi;
  UINT32   u32OffsetBleachRoi;
  UINT32   u32OffsetNextRecording;
  UINT32   u32Reserved [ 90 ];
} CZ_LSMINFO;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct CZ_ChannelColors {
  SINT32 s32BlockSize;    // Size of the structure in bytes including the name strings and colors.
  SINT32 s32NumberColors; // Number of colors in the color array; should be the same as the number of channels.
  SINT32 s32NumberNames;  // Number of character strings for the channel names; should be the same as the number of channels.
  SINT32 s32ColorsOffset; // Offset relative ti the start of the structure to the “UINT32” array of channel colors. 
                          // Each array entry contains a color with intensity values in the range 0..255 for the three color components
  SINT32 s32NamesOffset;  // Offset relative ti the start of the structure to the list of channel names. The list of channel names is a series of “\0”-terminated ANSI character strings.
  SINT32 s32Mono;         // If unequal zero the “Mono” button in the LSM-imagefenster  window was peressed
} CZ_ChannelColors;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct CZ_TimeStamps {
  SINT32  s32Size;              // Size, in bytes, of the whole block used for time stamps.
  SINT32  s32_NumberTimeStamps;	// Number of time stamps in the following list.
  FLOAT64 f64TimeStamp[1];      // Time stamps in seconds relative to the start time of 
  //FLOAT64	f64TimeStamp2	the LSM 510 electronic module controller program.
  //...	
  //FLOAT64	f64TimeStampN	
} CZ_TimeStamps;
#pragma pack(pop)

// Data types
#define TYPE_SUBBLOCK 0
#define TYPE_ASCII    2
#define TYPE_LONG     4
#define TYPE_RATIONAL 5

//Every subblock starts with a list entry of the type TYPE_SUBBLOCK. The u32Entry field can be:

#define SUBBLOCK_RECORDING             0x10000000
#define SUBBLOCK_TRACKS				         0x20000000
#define SUBBLOCK_LASERS                0x30000000
#define SUBBLOCK_TRACK 				         0x40000000
#define SUBBLOCK_LASER				         0x50000000
#define SUBBLOCK_DETECTION_CHANNELS    0x60000000
#define SUBBLOCK_DETECTION_CHANNEL 	   0x70000000
#define SUBBLOCK_ILLUMINATION_CHANNELS 0x80000000
#define SUBBLOCK_ILLUMINATION_CHANNEL  0x90000000
#define SUBBLOCK_BEAM_SPLITTERS		     0xA0000000
#define SUBBLOCK_BEAM_SPLITTER 		     0xB0000000
#define SUBBLOCK_DATA_CHANNELS	       0xC0000000
#define SUBBLOCK_DATA_CHANNEL          0xD0000000
#define SUBBLOCK_TIMERS    		         0x11000000
#define SUBBLOCK_TIMER      		    	 0x12000000
#define SUBBLOCK_MARKERS               0x13000000
#define SUBBLOCK_MARKER                0x14000000
#define SUBBLOCK_END                   0xFFFFFFFF


#pragma pack(push, 1)
typedef struct CZ_ScanInformation {
  UINT32 u32Entry; // A value that specifies which data are stored
  UINT32 u32Type;  //	A value that specifies the type of the data stored in the "Varaibable length data" field.
                   // TYPE_SUBBLOCK	- start or end of a subblock
                   // TYPE_LONG		  - 32 bit signed integer
                   // TYPE_RATIONAL - 64 bit floatingpoint 
                   // TYPE_ASCII 		- zero terminated string.
  UINT32 u32Size;	 // Size, in bytes, of the "Varaibable length data" field.
} CZ_ScanInformation;
#pragma pack(pop)

//----------------------------------------------------------------------------
// DLsmScanInfoEntry
//----------------------------------------------------------------------------
class DLsmScanInfoEntry {
public:
  DLsmScanInfoEntry(): entry_type(0), data_type(0) {}

  unsigned int entry_type;
  unsigned int data_type;
  std::vector<unsigned char> data;
  std::string path;

public:
  int readEntry (TIFF *tif, uint32 offset);
  inline unsigned int offsetSize() const;
};

//----------------------------------------------------------------------------
// TIFF Codec-wise Fluoview Structure 
//----------------------------------------------------------------------------
class DLsmInfo {
public:
  DLsmInfo(): ch(0), z_slices(0), t_frames(0), pages(0) {}

  int ch;        // number of channels in each image of the sequence
  int z_slices;  // number of Z slices for each time instance
  int t_frames;  // number of time frames in the sequence
  int pages;     // the value of consequtive images generated by driver
  double res[4]; // pixel resolution for XYZT
  
  CZ_LSMINFO         lsm_info; // actual header structure retreived form the image
  CZ_ChannelColors   lsm_colors;
  CZ_TimeStamps      lsm_TimeStamps;

  std::vector< DLsmScanInfoEntry > scan_info_entries;
  std::map< unsigned int, std::string > key_names;
  std::vector< std::string > channel_names;
  std::string objective;
  std::string name;

  //internal
  int pages_tiff;
};


#undef UINT32
#undef SINT32
#undef FLOAT64

#endif // DIM_CZ_LSM_FORMAT_H

