/*****************************************************************************
  BIORAD PIC support 
  UCSB/BioITR property
  Copyright (c) 2004 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  IMPLEMENTATION
  
  Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    04/22/2004 13:06 - First creation
    09/12/2005 17:34 - updated to api version 1.3
        
  Ver : 5
*****************************************************************************/

#ifndef DIM_BIORADPIC_FORMAT_H
#define DIM_BIORADPIC_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <cstdio>
#include <string>

// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimBioRadPicGetFormatHeader(void);
}

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------

void dimBioRadPicCloseImageProc (TDimFormatHandle *fmtHndl);


class DBioRadPicParams {
public:
  DBioRadPicParams();

  TDimImageInfo i;
  long num_images;

  long page_size_bytes;
  long data_offset;
  long notes_offset;
  long has_notes;


  // metadata
  int magnification;  
  std::string datetime;
  double pixel_size[10]; 

  std::string note01;  
  std::string note20;
  std::string note21;
};

#endif // DIM_BIORADPIC_FORMAT_H
