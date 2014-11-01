/*******************************************************************************

  Implementation of the Image Class for Windows API
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    03/24/2006 12:45 - First creation
      
  ver: 1
        
*******************************************************************************/

#include "dim_image.h"
#include "dim_img_format_utils.h"

#ifdef WIN32

#pragma message("TDimImage: WinAPI support methods")

HBITMAP TDimImage::toWinHBITMAP ( )
{
  HBITMAP hbmp = 0;

  return hbmp;
}

#endif //WIN32

