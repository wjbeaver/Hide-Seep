/*******************************************************************************

  Defines Image Stack Class
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    03/23/2004 18:03 - First creation
      
  ver: 1
        
*******************************************************************************/

#include <cmath>

#include "dim_image_stack.h"

#include <meta_format_manager.h>
#include <xstring.h>
#include <xtypes.h>

#include <string>
#include <sstream>
#include <map>
#include <vector>

#include <cstring>

//------------------------------------------------------------------------------
// DImageStack
//------------------------------------------------------------------------------

DImageStack::DImageStack() {
  init();
}

DImageStack::~DImageStack() {
  free();
}

DImageStack::DImageStack( const char *fileName, unsigned int limit_width, unsigned int limit_height, int only_channel ) {
  init();
  fromFile(fileName, limit_width, limit_height, only_channel);
}

DImageStack::DImageStack( const std::string &fileName, unsigned int limit_width, unsigned int limit_height, int only_channel ) {
  init();
  fromFile(fileName, limit_width, limit_height, only_channel);
}

void DImageStack::free() {
  images.clear();
}

void DImageStack::init() {
  cur_position = 0;
  handling_image = false;
  progress_proc = NULL;
  error_proc = NULL;
  test_abort_proc = NULL;
}

DImageStack DImageStack::deepCopy() const {
  DImageStack stack = *this;
  for (unsigned int i=0; i<images.size(); ++i)
    stack.images[i] = images[i].deepCopy();
  return stack;
}

bool DImageStack::isEmpty() const { 
  if ( images.size()<=0 || handling_image ) return true;
  return imageAt(0)->isNull(); 
}

TDimImage *DImageStack::imageAt( unsigned int position ) const {
  if ( (position >= images.size()) || handling_image ) 
    return (TDimImage *) &empty_image;
  return (TDimImage *) &images.at(position);
}

TDimImage *DImageStack::image() const {
  if (handling_image) return (TDimImage *) &empty_image;
  return imageAt( cur_position );
}
    
bool DImageStack::positionNext() {
  if (handling_image) return false;
  if (cur_position < (int)images.size()-1) {
    cur_position++;
    return true;
  }
  return false;
}

bool DImageStack::positionPrev() {
  if (handling_image) return false;
  if (cur_position > 0) {
    cur_position--;
    return true;
  }
  return false;
}

bool DImageStack::position0() {
  if (handling_image) return false;
  cur_position = 0;
  return true;
}

bool DImageStack::positionLast() {
  if (handling_image) return false;
  cur_position = images.size()-1;
  return true;
}

bool DImageStack::positionSet( unsigned int l ) { 
  if (handling_image) return false;
  if (l>=images.size()) return false;
  cur_position = l; 
  return true;
}

//------------------------------------------------------------------------------

bool DImageStack::fromFile( const char *fileName, unsigned int limit_width, unsigned int limit_height, int channel ) {
  int res = 0;
  handling_image = true;
  images.clear();
  metadata.clear();
  cur_position = 0;
  TMetaFormatManager fm;

  if ( (res = fm.sessionStartRead(fileName)) == 0) {

    int pages = fm.sessionGetNumberOfPages();
    for (int page=0; page<pages; ++page) {
      do_progress( page+1, pages, "Loading stack" );
      if (progress_abort()) break;

      // load page image, needs new clear image due to memory sharing
      TDimImage img;
      if (fm.sessionReadImage( img.imageBitmap(), page ) != 0) break;
      
      // use channel constraint
      if ( channel>=0 )
        img.extractChannel( channel );

      // use size limits
      if ( (limit_width>0 || limit_height>0) && (limit_width<img.width() || limit_height<img.height()) )
        img = img.resample( limit_width, limit_height, TDimImage::szBiCubic, true );
      images.push_back( img );
      
      if (page==0) {
        fm.sessionParseMetaData(0);
        metadata = fm.get_metadata();   
      }
    }
  }
  fm.sessionEnd(); 

  handling_image = false;
  return (res==0);
}

bool DImageStack::toFile( const char *fileName, const char *formatName ) {
  int res = 1;
  handling_image = true;
  TMetaFormatManager fm;
  int pages = images.size();

  if (fm.isFormatSupportsWMP( formatName ) == false) return false;

  if ( (res = fm.sessionStartWrite(fileName, formatName)) == 0) {

    for (int page=0; page<pages; ++page) {
      do_progress( page+1, pages, "Writing stack" );
      if (progress_abort()) break;
      fm.sessionWriteImage( images[page].imageBitmap(), page );
    }
  }
  fm.sessionEnd(); 

  handling_image = false;
  return (res==0);
}

//------------------------------------------------------------------------------

void DImageStack::convertToDepth( int depth, DimLut::LutType method, bool planes_independent, D_DataFormat pxtype ) {
  
  if (!planes_independent) {
    DStackHistogram stack_histogram ( *this );
    if (pxtype==D_FMT_UNDEFINED) pxtype = this->pixelType();
    DImageHistogram out( this->samples(), depth, pxtype );
    DImageLut lut( stack_histogram, out, method );
    convertToDepth( lut );
  } else {
    for (unsigned int i=0; i<images.size(); ++i) {
      do_progress( i, images.size(), "Converting depth" );
      images[i] = images[i].convertToDepth( depth, method, pxtype );
    }
  }
}

void DImageStack::convertToDepth( const DImageLut &lut ) {
  for (unsigned int i=0; i<images.size(); ++i) {
    do_progress( i, images.size(), "Converting depth" );
    images[i] = images[i].convertToDepth( lut );
  }
}

void DImageStack::normalize( int to_bpp, bool planes_independent ) {
  this->convertToDepth( to_bpp, DimLut::ltLinearDataRange, planes_independent, D_FMT_UNSIGNED );
}

void DImageStack::ensureTypedDepth() {
  for (unsigned int i=0; i<images.size(); ++i)
    images[i] = images[i].ensureTypedDepth();
}

//------------------------------------------------------------------------------

void DImageStack::negative() {
  for (unsigned int i=0; i<images.size(); ++i)
    images[i] = images[i].negative();
}

void DImageStack::discardLut() {
  for (unsigned int i=0; i<images.size(); ++i)
    images[i].discardLut();
}

void DImageStack::rotate( double deg ) {
  for (unsigned int i=0; i<images.size(); ++i) {
    do_progress( i, images.size(), "Rotate" );
    images[i] = images[i].rotate(deg);
  }
}

void DImageStack::remapChannels( const std::vector<int> &mapping ) {
  for (unsigned int i=0; i<images.size(); ++i)
    images[i].remapChannels(mapping);
}

//------------------------------------------------------------------------------

inline double ensure_range( double a, double minv, double maxv ) {
  if (a<minv) a=minv;
  else
  if (a>maxv) a=maxv;
  return a;
}

void DImageStack::ROI( DIM_UINT x, DIM_UINT y, DIM_UINT z, DIM_UINT w, DIM_UINT h, DIM_UINT d ) {
  
  if (isEmpty()) return;
  if ( x==0 && y==0 && z==0 && width()==w && height()==h && length()==d ) return;

  if (x >= width())  x = 0;
  if (y >= height()) y = 0;
  if (z >= length()) z = 0;
  if (w<=0) w = width();
  if (h<=0) h = height();
  if (w+x > width())  w = width()-x;
  if (h+y > height()) h = height()-y;
  if (d+z > length()) d = length()-z;

  // first get rid of unused planes
  if (d+z>0 && d+z < length())
    images.erase( images.begin()+(d+z), images.end() );

  if (z > 0)
    images.erase( images.begin(), images.begin()+(z-1) );

  // then run ROI on each plane
  if ( x==0 && y==0 && width()==w && height()==h ) return;
  for (unsigned int i=0; i<images.size(); ++i) {
    do_progress( i, images.size(), "ROI" );
    images[i] = images[i].ROI(x,y,w,h);
  }
}

DImageStack DImageStack::deepCopy( double x, double y, double z, double w, double h, double d ) const {
  if (isEmpty()) return DImageStack();
  if ( x==0 && y==0 && z==0 && width()==w && height()==h && length()==d ) return this->deepCopy();

  x = dim::round<int>( ensure_range( x, 0.0, width()-1.0 ) );
  w = dim::round<int>( ensure_range( w, 0.0, width()-x ) );
  y = dim::round<int>( ensure_range( y, 0.0, height()-1.0 ) );
  h = dim::round<int>( ensure_range( h, 0.0, height()-y ) );
  z = dim::round<int>( ensure_range( z, 0.0, length()-1.0 ) );
  d = dim::round<int>( ensure_range( d, 0.0, length()-z ) );

  DImageStack stack;
  for (unsigned int i=z; i<z+d; ++i)
    stack.images.push_back( images[i].ROI( x, y, w, h ) );
  return stack;
}

//------------------------------------------------------------------------------

TDimImage DImageStack::projectionXZAxis( unsigned int y ) {
  TDimImage img;
  if (y >= height()) return img;
  
  unsigned int w = width();
  unsigned int h = length();
  unsigned int s = samples();
  unsigned int d = depth();
  if (img.alloc( w, h, s, d ) != 0) return img;
  
  for (unsigned int z=0; z<length(); ++z) {
    for (unsigned int c=0; c<samples(); ++c) {
      unsigned char *line_in = images[z].scanLine( c, y );
      unsigned char *line_out = img.scanLine( c, z );
      memcpy( line_out, line_in, img.bytesPerLine() );
    } // for c
  } // for z 

  return img;
}

TDimImage DImageStack::projectionYZAxis( unsigned int x ) {
  TDimImage img;
  if (x >= width()) return img;
  
  unsigned int w = height();
  unsigned int h = length();
  unsigned int s = samples();
  unsigned int d = depth();
  if (img.alloc( w, h, s, d ) != 0) return img;

  std::vector<unsigned char> row_buf( img.bytesPerLine() );
  
  for (unsigned int z=0; z<length(); ++z) {
    for (unsigned int c=0; c<samples(); ++c) {
      images[z].scanRow( c, x, &row_buf[0] );
      unsigned char *line_out = img.scanLine( c, z );
      memcpy( line_out, &row_buf[0], img.bytesPerLine() );
    } // for c
  } // for z 

  return img;
}

void DImageStack::resize( int w, int h, int d, TDimImage::ResizeMethod method, bool keep_aspect_ratio ) {

  // first interpolate X,Y
  if (w!=0 || h!=0)
  if (w != width() || h != height() )
  for (unsigned int i=0; i<images.size(); ++i) {
    do_progress( i, images.size(), "Interpolating X/Y" );
    images[i] = images[i].resize(w, h, method, keep_aspect_ratio);
  }

  // then interpolate Z
  if (d==0 || d==length()) return;

  // create stack with corect size 
  DImageStack stack = *this;
  stack.images.clear();
  for (int z=0; z<d; ++z)
    stack.append( images[0].deepCopy() );
  
  // now interpolate Z
  w = images[0].width();
  for (unsigned int y=0; y<stack.height(); ++y) {
    do_progress( y, stack.height(), "Interpolating Z" );
    TDimImage img_xz = projectionXZAxis( y );
    img_xz = img_xz.resize(w, d, method, false );
    for (int z=0; z<d; ++z) {
      for (unsigned int c=0; c<stack.samples(); ++c) {
        unsigned char *line_out = stack.images[z].scanLine( c, y );
        unsigned char *line_in = img_xz.scanLine( c, z );
        memcpy( line_out, line_in, img_xz.bytesPerLine() );
      } // for c
    } // for z
  } // for y

  *this = stack;
}

void DImageStack::setNumberPlanes( unsigned int n ) {
  if (n == images.size()) return;
  if (n < images.size()) { images.resize( n ); return; }

  int d = n - images.size();
  TDimImage l = images[ images.size()-1 ];

  for (int z=0; z<d; ++z)
    this->append( l.deepCopy() );
}

TDimImage DImageStack::pixelArithmeticMax() const {
  TDimImage p = images[0].deepCopy();
  for (int z=1; z<images.size(); ++z)
    p.pixelArithmeticMax( images[z] );
  return p;
}

TDimImage DImageStack::pixelArithmeticMin() const {
  TDimImage p = images[0].deepCopy();
  for (int z=1; z<images.size(); ++z)
    p.pixelArithmeticMin( images[z] );
  return p;
}

//------------------------------------------------------------------------------
// DStackHistogram
//------------------------------------------------------------------------------

DStackHistogram::DStackHistogram() {

}

DStackHistogram::~DStackHistogram() {

}

void DStackHistogram::fromImageStack( const DImageStack &stack ) {  
  histograms.clear(); 
  histograms.resize( stack.samples() ); 

  // iterate first to get stats - slower but will not fail on float data
  for (unsigned int c=0; c<stack.samples(); ++c) {
    histograms[c].init( stack.depth(), stack.pixelType() );
    for (unsigned int i=0; i<stack.length(); ++i)
      histograms[c].updateStats( stack[i]->bits(c), stack[i]->numPixels() );
  } // channels  

  // iterate first to create histograms
  for (unsigned int c=0; c<stack.samples(); ++c) {
    for (unsigned int i=0; i<stack.length(); ++i)
      histograms[c].addData( stack[i]->bits(c), stack[i]->numPixels()  );
  } // channels
}

void DStackHistogram::fromImageStack( const DImageStack &stack, const TDimImage *mask ) {  
  if (mask && mask->depth()!=8) return;
  if (mask && mask->samples()>1 && mask->samples()<stack.samples() ) return;
  histograms.clear(); 
  histograms.resize( stack.samples() ); 

  // iterate first to get stats - slower but will not fail on float data
  for (unsigned int c=0; c<stack.samples(); ++c) {
    histograms[c].init( stack.depth(), stack.pixelType() );
    for (unsigned int i=0; i<stack.length(); ++i)
      histograms[c].updateStats( stack[i]->bits(c), stack[i]->numPixels(), (unsigned char *) mask->bits(c) );
  } // channels  

  // iterate first to create histograms
  for (unsigned int c=0; c<stack.samples(); ++c) {
    for (unsigned int i=0; i<stack.length(); ++i)
      histograms[c].addData( stack[i]->bits(c), stack[i]->numPixels(), (unsigned char *) mask->bits(c) );
  } // channels
}

void DStackHistogram::fromImageStack( const DImageStack &stack, const DImageStack *mask ) {  
  if (mask && mask->depth()!=8) return;
  if (mask && mask->samples()>1 && mask->samples()<stack.samples() ) return;
  if (mask && mask->length()<stack.length() ) return;
  histograms.clear(); 
  histograms.resize( stack.samples() ); 

  // iterate first to get stats - slower but will not fail on float data
  for (unsigned int c=0; c<stack.samples(); ++c) {
    histograms[c].init( stack.depth(), stack.pixelType() );
    for (unsigned int i=0; i<stack.length(); ++i)
      histograms[c].updateStats( stack[i]->bits(c), stack[i]->numPixels(), (unsigned char *) mask->imageAt(i)->bits(c) );
  } // channels  

  // iterate first to create histograms
  for (unsigned int c=0; c<stack.samples(); ++c) {
    for (unsigned int i=0; i<stack.length(); ++i)
      histograms[c].addData( stack[i]->bits(c), stack[i]->numPixels(), (unsigned char *) mask->imageAt(i)->bits(c) );
  } // channels
}
