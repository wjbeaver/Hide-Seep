/*****************************************************************************
  BMP support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    04/22/2004 13:06 - First creation
        
  Ver : 2
*****************************************************************************/

#ifndef DIM_BMP_FORMAT_H
#define DIM_BMP_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <stdio.h>


// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimBmpGetFormatHeader(void);
}


//----------------------------------------------------------------------------
// BMP internal structures, following Windows GDI:
//
//  [Structure]         [Bytes] 
//  BITMAPFILEHEADER    0x00 0x0D 
//  BITMAPINFOHEADER    0x0E 0x35 
//  RGBQUAD array       0x36 0x75 
//  Color-index array   0x76 0x275 
//----------------------------------------------------------------------------

void dimBmpCloseImageProc (TDimFormatHandle *fmtHndl);


#pragma pack(push, 1)
struct TDimBITMAPFILEHEADER { 
  DIM_UINT16  bfType;      // Specifies the file type, must be BM -> (0x42 0x4d)
  DIM_UINT32  bfSize;      // Specifies the size, in bytes, of the bitmap file. 
  DIM_UINT16  bfReserved1; // Reserved; must be zero. 
  DIM_UINT16  bfReserved2; // Reserved; must be zero. 
  DIM_UINT32  bfOffBits;   // Specifies the offset, in bytes, from the beginning of the 
                           // BITMAPFILEHEADER structure to the bitmap bits. 
}; 
#pragma pack(pop)

const int BMP_FILEHDR_SIZE = 14;	// size of BMP_FILEHDR data

const int DIM_BMP_OLD  = 12;			// old Windows/OS2 BMP size
const int DIM_BMP_WIN  = 40;			// new Windows BMP size
const int DIM_BMP_OS2  = 64;			// new OS/2 BMP size

const int DIM_BMP_RGB  = 0;				// no compression
const int DIM_BMP_RLE8 = 1;				// run-length encoded, 8 bits
const int DIM_BMP_RLE4 = 2;				// run-length encoded, 4 bits
const int DIM_BMP_BITFIELDS = 3;	// RGB values encoded in data as bit-fields

#pragma pack(push, 1)
struct TDimBITMAPINFOHEADER {
  DIM_UINT32  biSize;          // size of this struct
  DIM_UINT32  biWidth;         // pixmap width
  DIM_UINT32  biHeight;        // pixmap height 
  DIM_UINT16  biPlanes;        // should be 1
  DIM_UINT16  biBitCount;      // number of bits per pixel
  DIM_UINT32  biCompression;   // compression method
  DIM_UINT32  biSizeImage;     // size of image
  DIM_UINT32  biXPelsPerMeter; // horizontal resolution 
  DIM_UINT32  biYPelsPerMeter; // vertical resolution 
  DIM_UINT32  biClrUsed;       // number of colors used
  DIM_UINT32  biClrImportant;  // number of important colors
};
#pragma pack(pop) 

typedef struct TDimRGBQUAD {
  DIM_UINT8  rgbBlue; 
  DIM_UINT8  rgbGreen; 
  DIM_UINT8  rgbRed; 
  DIM_UINT8  rgbReserved; 
} TDimRGBQUAD; 

//----------------------------------------------------------------------------
// Internal Format Info Struct
//----------------------------------------------------------------------------

typedef struct TDimBmpParams {
  TDimImageInfo i;
  TDimBITMAPFILEHEADER bf;
  TDimBITMAPINFOHEADER bi;
} TDimBmpParams;



#endif // DIM_BMP_FORMAT_H
