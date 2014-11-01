/*****************************************************************************
  RAW support 
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    12/01/2005 15:27 - First creation
    2007-07-12 21:01 - reading raw
        
  Ver : 2
*****************************************************************************/

#ifndef DIM_RAW_FORMAT_H
#define DIM_RAW_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <stdio.h>


// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimRawGetFormatHeader(void);
}

void dimRawCloseImageProc (TDimFormatHandle *fmtHndl);

//----------------------------------------------------------------------------
// Internal Format Info Struct
//----------------------------------------------------------------------------

typedef struct TDimRawParams {
  TDimImageInfo i;
  unsigned int header_offset;
  bool big_endian;
} TDimRawParams;




#endif // DIM_BMP_FORMAT_H
