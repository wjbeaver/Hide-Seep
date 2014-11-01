/*****************************************************************************
  Igor binary file format v5 (IBW)
  UCSB/BioITR property
  Copyright (c) 2005 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  DEFINITIONS
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    10/19/2005 16:03 - First creation
            
  Ver : 1
*****************************************************************************/

#ifndef DIM_IBW_FORMAT_H
#define DIM_IBW_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <cstdio>
#include <vector>
#include <string>


// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimIbwGetFormatHeader(void);
}

/*****************************************************************************
  Igor binary file format - quick reference

  BinHeader5 structure  64 bytes
  WaveHeader5 structure excluding wData field 320 bytes
  Wave data Variable size
  Optional wave dependency formula  Variable size
  Optional wave note data Variable size
  Optional extended data units data Variable size
  Optional extended dimension units data  Variable size
  Optional dimension label data Variable size
  String indices used for text waves only Variable size

  Data order:
  row/column/layer/chunk

  Offsets:
  
  data_offset    = 64 + 320;
  formula_offset = data_offset + data_size;
  notes_offset   = formula_offset + formula_size;

  data_size = WaveHeader5.npnts * size_in_bytes_of[ WaveHeader5.type ]


*****************************************************************************/

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------

#define DIM_IBW_MAGIC_SIZE 2

const unsigned char ibwMagicWin[DIM_IBW_MAGIC_SIZE] = {0x05, 0x00};
const unsigned char ibwMagicMac[DIM_IBW_MAGIC_SIZE] = {0x00, 0x05};

#pragma pack(push, 2)

#define MAXDIMS 4

typedef struct BinHeader5 {
  DIM_INT16 version;                // Version number for backwards compatibility.
  DIM_INT16 checksum;               // Checksum over this header and the wave header.
  DIM_INT32 wfmSize;                // The size of the WaveHeader5 data structure plus the wave data.
  DIM_INT32 formulaSize;            // The size of the dependency formula, if any.
  DIM_INT32 noteSize;               // The size of the note text.
  DIM_INT32 dataEUnitsSize;         // The size of optional extended data units.
  DIM_INT32 dimEUnitsSize[MAXDIMS]; // The size of optional extended dimension units.
  DIM_INT32 dimLabelsSize[MAXDIMS]; // The size of optional dimension labels.
  DIM_INT32 sIndicesSize;           // The size of string indicies if this is a text wave.
  DIM_INT32 optionsSize1;           // Reserved. Write zero. Ignore on read.
  DIM_INT32 optionsSize2;           // Reserved. Write zero. Ignore on read.
} BinHeader5;


#define MAX_WAVE_NAME2 18 // Maximum length of wave name in version 1 and 2 files. Does not include the trailing null.
#define MAX_WAVE_NAME5 31 // Maximum length of wave name in version 5 files. Does not include the trailing null.
#define MAX_UNIT_CHARS 3

#define NT_CMPLX       1    // Complex numbers.
#define NT_FP32        2    // 32 bit fp numbers.
#define NT_FP64        4    // 64 bit fp numbers.
#define NT_I8          8    // 8 bit signed integer. Requires Igor Pro 2.0 or later.
#define NT_I16         0x10 // 16 bit integer numbers. Requires Igor Pro 2.0 or later.
#define NT_I32         0x20 // 32 bit integer numbers. Requires Igor Pro 2.0 or later.
#define NT_UNSIGNED    0x40 // Makes above signed integers unsigned. Requires Igor 

typedef struct WaveHeader5 {
  DIM_UINT32 nextPointer;    // link to next wave in linked list. Used in memory only. Write zero. Ignore on read.

  DIM_UINT32 creationDate;   // DateTime of creation.
  DIM_UINT32 modDate;        // DateTime of last modification.

  DIM_INT32  npnts;          // Total number of points (multiply dimensions up to first zero).
  DIM_INT16  type;           // See types (e.g. NT_FP64) above. Zero for text waves.
  DIM_INT16  dLock;          // Reserved. Write zero. Ignore on read.

  DIM_CHAR   whpad1[6];               // Reserved. Write zero. Ignore on read.
  DIM_INT16  whVersion;               // Write 1. Ignore on read.
  DIM_CHAR   bname[MAX_WAVE_NAME5+1]; // Name of wave plus trailing null.
  DIM_INT32  whpad2;                  // Reserved. Write zero. Ignore on read.
  DIM_UINT32 dFolderPointer;          // Used in memory only. Write zero. Ignore on read.

  // Dimensioning info. [0] == rows, [1] == cols etc
  DIM_INT32  nDim[MAXDIMS];           // Number of of items in a dimension -- 0 means no data.
  DIM_DOUBLE sfA[MAXDIMS];            // Index value for element e of dimension d = sfA[d]*e + sfB[d].
  DIM_DOUBLE sfB[MAXDIMS];

  // SI units
  DIM_CHAR   dataUnits[MAX_UNIT_CHARS+1];         // Natural data units go here - null if none.
  DIM_CHAR   dimUnits[MAXDIMS][MAX_UNIT_CHARS+1]; // Natural dimension units go here - null if none.

  DIM_INT16  fsValid;                   // TRUE if full scale values have meaning.
  DIM_INT16  whpad3;                    // Reserved. Write zero. Ignore on read.
  DIM_DOUBLE topFullScale;
  DIM_DOUBLE botFullScale;              // The max and max full scale value for wave.

  DIM_UINT32 dataEUnitsPointer;         // Used in memory only. Write zero. Ignore on read.
  DIM_UINT32 dimEUnitsPointer[MAXDIMS]; // Used in memory only. Write zero. Ignore on read.
  DIM_UINT32 dimLabelsPointer[MAXDIMS]; // Used in memory only. Write zero. Ignore on read.
  
  DIM_UINT32 waveNoteHPointer;          // Used in memory only. Write zero. Ignore on read.
  DIM_INT32  whUnused[16];              // Reserved. Write zero. Ignore on read.

  // The following stuff is considered private to Igor.

  DIM_INT16  aModified;       // Used in memory only. Write zero. Ignore on read.
  DIM_INT16  wModified;       // Used in memory only. Write zero. Ignore on read.
  DIM_INT16  swModified;      // Used in memory only. Write zero. Ignore on read.
  
  DIM_CHAR   useBits;         // Used in memory only. Write zero. Ignore on read.
  DIM_CHAR   kindBits;        // Reserved. Write zero. Ignore on read.
  DIM_UINT32 formulaPointer;  // Used in memory only. Write zero. Ignore on read.
  DIM_INT32  depID;           // Used in memory only. Write zero. Ignore on read.
  
  DIM_INT16  whpad4;          // Reserved. Write zero. Ignore on read.
  DIM_INT16  srcFldr;         // Used in memory only. Write zero. Ignore on read.
  DIM_UINT32 fileNamePointer; // Used in memory only. Write zero. Ignore on read.
  
  DIM_UINT32 sIndicesPointer; // Used in memory only. Write zero. Ignore on read.

  DIM_FLOAT  wData[1];        // The start of the array of data. Must be 64 bit aligned.
} WaveHeader5;

#pragma pack(pop) 


//----------------------------------------------------------------------------
// Internal Format Info Struct
//----------------------------------------------------------------------------

class DIbwParams {
public:
  DIbwParams(): little_endian(false), formula_offset(0), notes_offset(0) { i = initTDimImageInfo(); }

  TDimImageInfo i;
  BinHeader5  bh;
  WaveHeader5 wh;
  long data_offset;
  bool little_endian;
  long formula_offset;
  long notes_offset;
  long data_size;
  int  real_bytespp;
  DIM_DataType real_type;

  std::string note;
};

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------
void             dimIbwCloseImageProc     ( TDimFormatHandle *fmtHndl);


#endif // DIM_IBW_FORMAT_H
