/*****************************************************************************
  FLUOVIEW definitions 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
    
  History:
    03/29/2004 22:23 - First creation
    10/12/2005 15:03 - updates for 64 bit machines       

  Ver : 2
*****************************************************************************/

#ifndef DIM_FLUOVIEW_FORMAT_H
#define DIM_FLUOVIEW_FORMAT_H

#include <vector>
#include <string>

#include <dim_img_format_interface.h>

//----------------------------------------------------------------------------
// Internal Fluoview Structures
//----------------------------------------------------------------------------

#define MMHEADER  34361

#define IMAGE_NAME_LENGTH   256
#define SPATIAL_DIMENSION   10
#define DIMNAME_LENGTH      16
#define UNITS_LENGTH        64

typedef DIM_UINT32 MM_HANDLE;      // Size (bytes):     4

#pragma pack(push, 1)
typedef struct MM_DIM_INFO
{
  DIM_CHAR      Name[DIMNAME_LENGTH]; //Dimension name e.g. Width   16
  DIM_UINT32    Size;                 //Image width etc              4
  DIM_DOUBLE    Origin;               //Origin                       8
  DIM_DOUBLE    Resolution;           //Image resolution             8
  DIM_CHAR      Units[UNITS_LENGTH];  //Image calibration units     64
} MM_DIM_INFO;                        // Total Size (bytes):       100  

typedef struct MM_HEAD
{
  DIM_INT16     HeaderFlag;                 //Size of header structure             2
  DIM_UCHAR     ImageType;                  //Image Type                           1
  DIM_CHAR      Name[IMAGE_NAME_LENGTH];    //Image name                         256
  DIM_UCHAR     Status;                     //image status                         1
  DIM_UINT32    Data;                       //Handle to the data field             4
  DIM_UINT32    NumberOfColors;             //Number of colors in palette          4
  DIM_UINT32    MM_256_Colors;              //handle to the palette field          4
  DIM_UINT32    MM_All_Colors;              //handle to the palette field          4
  DIM_UINT32    CommentSize;                //Size of comments field               4
  DIM_UINT32    Comment;                    //handle to the comment field          4
  MM_DIM_INFO   DimInfo[SPATIAL_DIMENSION]; //Dimension Info                    1000    
  DIM_UINT32    SpatialPosition;            //obsolete???????????                  4
  DIM_INT16     MapType;                    //Display mapping type                 2
  //short         reserved;                   //Display mapping type                 2
  DIM_DOUBLE    MapMin;                     //Display mapping minimum              8
  DIM_DOUBLE    MapMax;                     //Display mapping maximum              8
  DIM_DOUBLE    MinValue;                   //Image histogram minimum              8
  DIM_DOUBLE    MaxValue;                   //Image histogram maximum              8
  MM_HANDLE     Map;                        //Handle to gray level mapping array   4
  DIM_DOUBLE    Gamma;                      //Image gray level correction factor   8
  DIM_DOUBLE    Offset;                     //Image gray level correction offset   8
  MM_DIM_INFO   Gray;                       //                                   100
  MM_HANDLE     ThumbNail;                  //handle to the ThumbNail field        4
  DIM_UINT32    UserFieldSize;              //Size of Voice field                  4
  MM_HANDLE     UserFieldHandle;            //handle to the Voice field            4
} MM_HEAD;                                  // Total Size (bytes):              1456
#pragma pack(pop)

//----------------------------------------------------------------------------
// TIFF Codec-wise Fluoview Structure 
//----------------------------------------------------------------------------
class DFluoviewInfo {
public:
  DFluoviewInfo(): ch(0), z_slices(0), t_frames(0), pages(0), xR(0), yR(0), zR(0), pages_tiff(0) {}

  int ch;       // number of channels in each image of the sequence
  int z_slices; // number of Z slices for each time instance
  int t_frames; // number of time frames in the sequence
  int pages;    // the value of consequtive images generated by driver
  double xR, yR, zR; // pixel resolution for XYZ
  MM_HEAD head; // actual header structure retreived form the image

  //internal
  int pages_tiff;
  std::vector<std::string> sample_names;
  std::vector<int> display_lut; // rgb -> chan numbers
};


#endif // DIM_FLUOVIEW_FORMAT_H















