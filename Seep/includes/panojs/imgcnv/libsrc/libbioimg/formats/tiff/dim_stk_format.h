/*****************************************************************************
  STK definitions
  
  Writen by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>
  
  History:
    04/20/2004 23:33 - First creation
        
  Ver : 1
*****************************************************************************/


#ifndef DIM_STK_FORMAT_H
#define DIM_STK_FORMAT_H

#include "dim_tiff_format.h"

#define DIM_STK_AutoScale              0
#define DIM_STK_MinScale               1
#define DIM_STK_MaxScale               2
#define DIM_STK_SpatialCalibration     3
#define DIM_STK_XCalibration           4
#define DIM_STK_YCalibration           5
#define DIM_STK_CalibrationUnits       6
#define DIM_STK_Name                   7
#define DIM_STK_ThreshState            8
#define DIM_STK_ThreshStateRed         10
#define DIM_STK_ThreshStateGreen       11
#define DIM_STK_ThreshStateBlue        12
#define DIM_STK_ThreshStateLo          13
#define DIM_STK_ThreshStateHi          14
#define DIM_STK_Zoom                   15
#define DIM_STK_CreateTime             16
#define DIM_STK_LastSavedTime          17
#define DIM_STK_currentBuffer          18
#define DIM_STK_grayFit                19
#define DIM_STK_grayPointCount         20
#define DIM_STK_grayX                  21
#define DIM_STK_grayY                  22
#define DIM_STK_grayMin                23
#define DIM_STK_grayMax                24
#define DIM_STK_grayUnitName           25
#define DIM_STK_StandardLUT            26
#define DIM_STK_wavelength             27
#define DIM_STK_StagePosition          28
#define DIM_STK_CameraChipOffset       29
#define DIM_STK_OverlayMask            30
#define DIM_STK_OverlayCompress        31
#define DIM_STK_Overlay                32
#define DIM_STK_SpecialOverlayMask     33
#define DIM_STK_SpecialOverlayCompress 34
#define DIM_STK_SpecialOverlay         35
#define DIM_STK_ImageProperty          36 
#define DIM_STK_StageLabel             37
#define DIM_STK_AutoScaleLoInfo        38
#define DIM_STK_AutoScaleHiInfo        39
#define DIM_STK_AbsoluteZ              40
#define DIM_STK_AbsoluteZValid         41
#define DIM_STK_Gamma                  42
#define DIM_STK_GammaRed               43
#define DIM_STK_GammaGreen             44
#define DIM_STK_GammaBlue              45

static int stk_tag_sizes_long[46] = 
{ 
   1, 1, 1, 1,
   2, 2, 
   1, // contains the size of following string in bytes
   1, // contains the size of following string in bytes
   1, 1, 1, 1,
   1, 1, 1, 2,
   2, 1, 1, 1,
   2, 2, 2, 2,
   1, // contains the size of following string in bytes
   1, 1, 
   4, // 4*N longs
   4, // 4*N longs
   1, 1, 1, 1, 1, 1, 1,
   1, // N longs
   2, 2,
   2, // 2*N longs 
   1, // N longs
   1, 1, 1, 1
};

typedef struct TDimStkRational {
  DIM_INT32 num;
  DIM_INT32 den;
} TDimStkRational;

// each dynamic array here is f size N
typedef struct TDimStkMetaData {
  int N;

  // UIC1 and UIC4
  DIM_INT32 AutoScale;
  DIM_INT32 MinScale;
  DIM_INT32 MaxScale;
  DIM_INT32 SpatialCalibration;
  DIM_INT32 XCalibration[2];
  DIM_INT32 YCalibration[2];
  char *CalibrationUnits;
  char *Name;
  DIM_INT32 ThreshState;
  DIM_UINT32 ThreshStateRed;
  DIM_UINT32 ThreshStateGreen;
  DIM_UINT32 ThreshStateBlue;
  DIM_UINT32 ThreshStateLo;
  DIM_UINT32 ThreshStateHi;
  DIM_INT32 Zoom;
  DIM_INT32 CreateTime[2];
  DIM_INT32 LastSavedTime[2];
  DIM_INT32 currentBuffer;
  DIM_INT32 grayFit;
  DIM_INT32 grayPointCount;
  DIM_INT32 grayX[2];
  DIM_INT32 grayY[2];
  DIM_INT32 grayMin[2];
  DIM_INT32 grayMax[2];
  char *grayUnitName;
  DIM_INT32 StandardLUT;
  //DIM_INT32 wavelength; // discard it, don't read, the UIC3 value should be used
  
  // begin: Used internally by MetaMorph
  DIM_INT32 OverlayMask;
  DIM_INT32 OverlayCompress;
  DIM_INT32 Overlay;
  DIM_INT32 SpecialOverlayMask;
  DIM_INT32 SpecialOverlayCompress;
  DIM_INT32 SpecialOverlay;
  DIM_INT32 ImageProperty;
  // end: Used internally by MetaMorph

  DIM_INT32 AutoScaleLoInfo[2];
  DIM_INT32 AutoScaleHiInfo[2];
  DIM_INT32 Gamma; 
  DIM_INT32 GammaRed; 
  DIM_INT32 GammaGreen; 
  DIM_INT32 GammaBlue; 

  // UIC3
  TDimStkRational *wavelength;

  // UIC2
  TDimStkRational *zDistance;
  DIM_INT32 *creationDate;
  DIM_INT32 *creationTime;
  DIM_INT32 *modificationDate;
  DIM_INT32 *modificationTime;

  //UIC4
  TDimStkRational *StagePositionX;
  TDimStkRational *StagePositionY;
  TDimStkRational *CameraChipOffsetX;
  TDimStkRational *CameraChipOffsetY;
  char *StageLabel; 
  TDimStkRational *AbsoluteZ; 
  DIM_INT32 *AbsoluteZValid; 

} TDimStkMetaData;

/*
#pragma pack(push, 1)
typedef struct DStkTagTuple {
  DIM_INT32  id;
  DIM_UINT32 val;
} DStkTagTuple; 
#pragma pack(pop)
*/

class DStkInfo {
public:
  DStkInfo();
  ~DStkInfo();

  D_TIFF_STRP_TYPE strips_per_plane; // strips per plane
  D_TIFF_OFFS_TYPE *strip_offsets; // offsets of each strip
  D_TIFF_BCNT_TYPE *strip_bytecounts; // offsets of each strip
  TDimStkMetaData metaData;

public:
  void init();
  void clear();
  void clearOffsets();
  void allocMetaInfo( DIM_INT32 N );
};

#endif // DIM_STK_FORMAT_H
