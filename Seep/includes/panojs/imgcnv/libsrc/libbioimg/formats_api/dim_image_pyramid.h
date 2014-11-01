/*******************************************************************************

  Defines Image Pyramid Class
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    03/23/2004 18:03 - First creation
      
  ver: 1
        
*******************************************************************************/

#ifndef DIM_IMAGE_PYRAMID_H
#define DIM_IMAGE_PYRAMID_H

#include <vector>

#include "dim_image.h"
#include "dim_image_stack.h"

const int d_min_image_size = 50;

class DImagePyramid: public DImageStack {
  public:
    DImagePyramid();
    ~DImagePyramid();

    void init();

    // return true if the level exists
    bool levelDown() { return positionNext(); }
    bool levelUp() { return positionPrev(); }
    bool level0() { return position0(); }
    bool levelLowest() { return positionLast(); }
    bool levelSet( int l ) { return positionSet(l); }
    int  levelCurrent() const { return positionCurrent(); }
    int  levelClosestTop( unsigned int w, unsigned int h ) const;
    int  levelClosestBottom( unsigned int w, unsigned int h ) const;
    int  numberLevels() const { return size(); }

    void setMinImageSize( int v ) { min_width = v; min_height = v; }

    void createFrom( const TDimImage &img );
    bool fromFile( const char *fileName, int page );
    bool fromFile( const std::string &fileName, int page ) { return fromFile( fileName.c_str(), page ); }

  protected:
    int  min_width;
    int  min_height;

  protected:
    // prohibit copy-constructor
    DImagePyramid( const DImagePyramid& ) {}
    bool fromFile( const char * /*fileName*/ ) { return false; }
    bool fromFile( const std::string & /*fileName*/ ) { return false; }
};


#endif //DIM_IMAGE_PYRAMID_H


