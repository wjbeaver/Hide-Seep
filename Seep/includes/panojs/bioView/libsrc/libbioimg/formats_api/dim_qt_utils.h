/*******************************************************************************

  Defines Image Format - Qt4 Utilities
  rely on: DimFiSDK version: 1
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    09/13/2005 19:50 - First creation
      
  ver: 1
        
*******************************************************************************/

#ifndef DIM_QT_UTL_H
#define DIM_QT_UTL_H

#include <QImage>
#include <QPixmap>

#include "dim_img_format_interface.h"

QImage  qImagefromDimImage  (const TDimImageBitmap &img );
QPixmap qPixmapfromDimImage (const TDimImageBitmap &img );


#endif //DIM_QT_UTL_H


