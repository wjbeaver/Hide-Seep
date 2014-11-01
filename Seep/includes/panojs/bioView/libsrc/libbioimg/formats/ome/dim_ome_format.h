/*****************************************************************************
  OME XML file format (Open Microscopy Environment)
  UCSB/BioITR property
  Copyright (c) 2005 by Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>

  DEFINITIONS
  
  Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    11/21/2005 15:43 - First creation
            
  Ver : 1
*****************************************************************************/

#ifndef DIM_OME_FORMAT_H
#define DIM_OME_FORMAT_H

#include <dim_img_format_interface.h>
#include <dim_img_format_utils.h>

#include <stdio.h>
#include <vector>
#include <string>


// DLL EXPORT FUNCTION
extern "C" {
TDimFormatHeader* dimOmeGetFormatHeader(void);
}


/*****************************************************************************
  OME XML file format - quick reference

  OME/src/xml/schemas/BinaryFile.xsd

*****************************************************************************/


//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------

#define DIM_OME_MAGIC_SIZE 6

const char omeMagic[DIM_OME_MAGIC_SIZE] = "<?xml";

//----------------------------------------------------------------------------
// Internal Format Info Struct
//----------------------------------------------------------------------------

typedef struct TDimOmeParams
{
  TDimImageInfo i;
  
} TDimOmeParams;

//----------------------------------------------------------------------------
// Internal Format Structs
//----------------------------------------------------------------------------
void             dimOmeCloseImageProc     ( TDimFormatHandle *fmtHndl);


#endif // DIM_OME_FORMAT_H
