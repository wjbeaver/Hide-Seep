/*****************************************************************************
  PNG support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    07/29/2004 18:09 - First creation
        
  Ver : 1
*****************************************************************************/

#ifndef DIM_PNG_FORMAT_H
#define DIM_PNG_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <stdio.h>

#include <png.h>


// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimPngGetFormatHeader(void);
}

void dimPngCloseImageProc (TDimFormatHandle *fmtHndl);

//----------------------------------------------------------------------------
// MetaData tags
//----------------------------------------------------------------------------

#define DIM_PNG_TAG_TITLE        0
#define DIM_PNG_TAG_AUTHOR       1
#define DIM_PNG_TAG_DESCRIPTION  2
#define DIM_PNG_TAG_COPYRIGHT    3
#define DIM_PNG_TAG_TIME         4
#define DIM_PNG_TAG_SOFTWARE     5
#define DIM_PNG_TAG_DISCLAIMER   6
#define DIM_PNG_TAG_WARNING      7
#define DIM_PNG_TAG_SOURCE       8
#define DIM_PNG_TAG_COMMENT      9

//----------------------------------------------------------------------------
// Internal Format Info Struct
//----------------------------------------------------------------------------

const unsigned char png_magic[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

typedef struct TDimPngParams
{
  TDimImageInfo i;
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;
} TDimPngParams;


#endif // DIM_PNG_FORMAT_H
