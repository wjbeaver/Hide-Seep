/*******************************************************************************

  Implementation of the Image Class, it uses smart pointer technology to implement memory
  sharing, simple cope operations simply point to the same memory addresses
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  Image file structure:
    
    1) Page:   each file may contain 1 or more pages, each page is independent

    2) Sample: in each page each pixel can contain 1 or more samples
               preferred number of samples: 1 (GRAY), 3(RGB) and 4(RGBA)

    3) Depth:  each sample can be of 1 or more bits of depth
               preferred depths are: 8 and 16 bits per sample

    4) Allocation: each sample is allocated having in mind:
               All lines are stored contiguasly from top to bottom where
               having each particular line is byte alligned, i.e.:
               each line is allocated using minimal necessary number of bytes
               to store given depth. Which for the image means:
               size = ceil( ( (width*depth) / 8 ) * height )

  As a result of Sample/Depth structure we can get images of different 
  Bits per Pixel as: 1, 8, 24, 32, 48 and any other if needed

  History:
    03/23/2004 18:03 - First creation
      
  ver: 1
        
*******************************************************************************/

#include <string>
#include <iostream>
#include <fstream>
#include <limits>

#include <cstring>

#include "xtypes.h"
#include "dim_image.h"
#include "dim_img_format_utils.h"
#include "dim_buffer.h"
#include "dim_histogram.h"
#include "dim_image_pyramid.h"

//#include <blob_manager.h>

#ifdef DIM_USE_IMAGEMANAGER
#include <meta_format_manager.h>
#endif //DIM_USE_IMAGEMANAGER

#ifdef DIM_USE_QT
#include <dim_image_qt.cpp>
#endif //DIM_USE_QT

#ifdef WIN32
#include <dim_image_win.cpp>
#endif //WIN32

#include "resize.h"
#include "rotate.h"

#include "typeize_buffer.cpp"

std::vector<DImgRefs*> TDimImage::refs;

TDimImage::TDimImage() {
  bmp = NULL;
  connectToNewMemory();
}

TDimImage::~TDimImage() {
  disconnectFromMemory();
}

TDimImage::TDimImage(const TDimImage& img) { 
  bmp = NULL;
  connectToMemory( img.bmp );
  if (img.metadata.size()>0) metadata = img.metadata;
}

TDimImage::TDimImage(DIM_UINT width, DIM_UINT height, DIM_UINT depth, DIM_UINT samples) { 
  bmp = NULL;
  connectToNewMemory();
  create( width, height, depth, samples ); 
}

#ifdef DIM_USE_IMAGEMANAGER
TDimImage::TDimImage(const char *fileName, int page) {
  bmp = NULL;
  connectToNewMemory();
  fromFile( fileName, page ); 
}

TDimImage::TDimImage(const std::string &fileName, int page) { 
  bmp = NULL;
  connectToNewMemory();
  fromFile( fileName, page ); 
}
#endif //DIM_USE_IMAGEMANAGER

TDimImage &TDimImage::operator=( const TDimImage & img ) { 
  connectToMemory( img.bmp );
  if (img.metadata.size()>0) this->metadata = img.metadata;
  return *this; 
}

TDimImage TDimImage::deepCopy() const { 
  TDimImage img;
  if (bmp==NULL) return img;
  img.alloc( this->width(), this->height(), this->samples(), this->depth() );
  
  DIM_UINT sample, chan_size = img.bytesPerChan();
  for (sample=0; sample<bmp->i.samples; sample++) {
    memcpy( img.bmp->bits[sample], bmp->bits[sample], chan_size );
  }
  return img;
}

//------------------------------------------------------------------------------------
// shared memory part
//------------------------------------------------------------------------------------

inline void TDimImage::print_debug() {
  /*
  for (int i=0; i<refs.size(); ++i) {
    std::cout << "[" << i << ": " << refs[i]->refs << " " << ((int*)&refs[i]->bmp);
    std::cout << " " << refs[i]->bmp.i.width << "*" << refs[i]->bmp.i.height;
    std::cout << "] ";
  }
  std::cout << "\n";
  */
}

int TDimImage::getRefId( TDimImageBitmap *b ) {
  if (b == NULL) return -1;
  for (unsigned int i=0; i<refs.size(); ++i)
    if (b == &refs[i]->bmp) 
      return i;
  return -1;
}

int TDimImage::getCurrentRefId() {
  return getRefId( bmp );
}

void TDimImage::connectToMemory( TDimImageBitmap *b ) {
  disconnectFromMemory();

  int ref_id = getRefId( b );
  if (ref_id == -1) return;

  bmp = &refs.at(ref_id)->bmp;
  refs.at(ref_id)->refs++;
}

void TDimImage::connectToNewMemory() {
  disconnectFromMemory();
  
  // create a new reference
  DImgRefs *new_ref = new DImgRefs;
  refs.push_back( new_ref );
  int ref_id = refs.size()-1;

  refs.at(ref_id)->refs++;
  bmp = &refs.at(ref_id)->bmp;
  initImagePlanes( bmp );
}

void TDimImage::disconnectFromMemory() {
  int ref_id = getCurrentRefId();
  bmp = NULL;

  // decrease the reference to the image
  if (ref_id == -1) return;
  refs.at(ref_id)->refs--;

  // check if current reference will be zero, then delete image
  if (refs[ref_id]->refs < 1) {
    DImgRefs *new_ref = refs[ref_id];
    deleteImg( &refs.at(ref_id)->bmp );
    refs.erase( refs.begin() + ref_id );
    delete new_ref;
  }
}

//------------------------------------------------------------------------------------
// allocation
//------------------------------------------------------------------------------------

int TDimImage::alloc( DIM_UINT w, DIM_UINT h, DIM_UINT samples, DIM_UINT depth ) {
  DIM_UINT sample=0;
  this->free();
  if (bmp==NULL) return 1;

  bmp->i.width   = w;
  bmp->i.height  = h;
  bmp->i.samples = samples;
  bmp->i.depth   = depth;
  long size     = bytesPerChan( );

  for (sample=0; sample<bmp->i.samples; sample++) {
    try {
      bmp->bits[sample] = new DIM_UCHAR [ size ];
    } 
    catch (...) {
      bmp->bits[sample] = NULL;
      deleteImg( bmp );
      bmp->i = initTDimImageInfo();      
      return 1;
    }
  }
  return 0;  
}

void TDimImage::free( ) {
  connectToNewMemory();
  metadata.clear();
}

void* TDimImage::bits(const unsigned int &sample) const { 
  if (!bmp) return NULL;
  unsigned int c = dim::trim<unsigned int, unsigned int>( sample, 0, samples()-1); 
  return (void *) bmp->bits[c]; 
}

void TDimImage::setLutColor( DIM_UINT i, TDimRGBA c ) {
  if (bmp==NULL) return;
  if ( i>=bmp->i.lut.count ) return;
  bmp->i.lut.rgba[i] = c;
}

void TDimImage::setLutNumColors( DIM_UINT n ) {
  if (bmp==NULL) return;
  if ( n>256 ) return;
  bmp->i.lut.count = n;
}

TDimImage TDimImage::convertToDepth( const DImageLut &lut ) const {

  TDimImage img;
  if (bmp==NULL) return img;
  if (lut.size() < (int)this->samples()) return *this;

  int depth = lut.depthOutput();
  D_DataFormat pxt = lut.dataFormatOutput();
  DIM_UINT w = bmp->i.width;
  DIM_UINT h = bmp->i.height;
  unsigned int num_pix = w * h;

  if ( img.alloc( w, h, bmp->i.samples, depth ) == 0 ) {
    img.bmp->i = this->bmp->i;
    img.bmp->i.depth = depth;
    img.bmp->i.pixelType = pxt;
    for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {
      lut[sample]->apply( bmp->bits[sample], img.bits(sample), num_pix );
    } // sample
  }

  return img;
}

TDimImage TDimImage::convertToDepth( int depth, DimLut::LutType method, D_DataFormat pxt ) const {
  TDimImage img;
  if (bmp==NULL) return img;

  DIM_UINT w = bmp->i.width;
  DIM_UINT h = bmp->i.height;
  unsigned int num_pix = w * h;
    
  if ( img.alloc( w, h, bmp->i.samples, depth ) == 0 ) {
    img.bmp->i = this->bmp->i;
    img.bmp->i.depth = depth;
    if (pxt!=D_FMT_UNDEFINED) img.bmp->i.pixelType = pxt;
    
    DImageHistogram ih(*this);
    DImageHistogram oh( img.samples(), img.depth(), img.pixelType() );
    DImageLut       lut(ih, oh, method);

    for (unsigned int sample=0; sample<bmp->i.samples; ++sample )
      lut[sample]->apply( bmp->bits[sample], img.bits(sample), num_pix );
  }

  return img;
}

TDimImage TDimImage::normalize( int to_bpp ) const {

  if (bmp==NULL) return TDimImage();
  if (bmp->i.depth == to_bpp) return *this;

  return convertToDepth( to_bpp, DimLut::ltLinearDataRange, D_FMT_UNSIGNED );
}

TDimImage TDimImage::ROI( DIM_UINT x, DIM_UINT y, DIM_UINT w, DIM_UINT h ) const {

  TDimImage img;
  if (bmp==NULL) return img;
  if ( x==0 && y==0 && bmp->i.width==w && bmp->i.height==h ) return *this;
  if (x >= bmp->i.width)  x = 0;
  if (y >= bmp->i.height) y = 0;
  if (w+x > bmp->i.width)  w = bmp->i.width-x;
  if (h+y > bmp->i.height) h = bmp->i.height-y;

  if ( img.alloc( w, h, bmp->i.samples, bmp->i.depth ) == 0 ) {
  
    int newLineSize = img.bytesPerLine();
    int oldLineSize = this->bytesPerLine();
    int Bpp = (long) ceil( ((double)bmp->i.depth) / 8.0 );

    for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {
      unsigned char *pl  = (unsigned char *) img.bits(sample);
      unsigned char *plo = ( (unsigned char *) bmp->bits[sample] ) + y*oldLineSize + x*Bpp;
      register unsigned int yi;

      for (yi=0; yi<h; yi++) {
        memcpy( pl, plo, w*Bpp );      
        pl  += newLineSize;
        plo += oldLineSize;
      } // for yi
    } // sample
  } // allocated image

  img.bmp->i = this->bmp->i;
  img.bmp->i.width = w;
  img.bmp->i.height = h;
  return img;
}

std::string TDimImage::getTextInfo() const {
  std::string str = getImageInfoText( (TDimImageBitmap *) bmp ); 
  return str;
}

#ifdef DIM_USE_IMAGEMANAGER
bool TDimImage::fromFile( const char *fileName, int page ) {
  this->free();
  if (bmp==NULL) return false;

  TMetaFormatManager fm;
  bool res = true;

  if (fm.sessionStartRead( fileName ) == 0) {
    fm.sessionReadImage( bmp, page );

    // getting metadata fields
    fm.sessionParseMetaData(0);
    metadata = fm.get_metadata();   

  } else res = false;

  fm.sessionEnd();
  return res;
}

bool TDimImage::toFile( const char *fileName, const char *formatName, const char *options ) {
  TMetaFormatManager fm;
  fm.writeImage ( fileName, bmp,  formatName, options);
  return true;
}

#endif //DIM_USE_IMAGEMANAGER


template <typename T>
void fill_channel ( T *p, const T &v, const unsigned int &num_points ) {
  for (unsigned int x=0; x<num_points; ++x)
    p[x] = v;
}

void TDimImage::fill( double v ) {

  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {
    if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_UNSIGNED)
      fill_channel<DIM_UINT8>( (DIM_UINT8*) bmp->bits[sample], (DIM_UINT8) v, bmp->i.width*bmp->i.height );
    else
    if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_UNSIGNED)
      fill_channel<DIM_UINT16>( (DIM_UINT16*) bmp->bits[sample], (DIM_UINT16) v, bmp->i.width*bmp->i.height );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_UNSIGNED)
      fill_channel<DIM_UINT32>( (DIM_UINT32*) bmp->bits[sample], (DIM_UINT32) v, bmp->i.width*bmp->i.height );
    else
    if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_SIGNED)
      fill_channel<DIM_INT8>( (DIM_INT8*) bmp->bits[sample], (DIM_INT8) v, bmp->i.width*bmp->i.height );
    else
    if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_SIGNED)
      fill_channel<DIM_INT16>( (DIM_INT16*) bmp->bits[sample], (DIM_INT16) v, bmp->i.width*bmp->i.height );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_SIGNED)
      fill_channel<DIM_INT32>( (DIM_INT32*) bmp->bits[sample], (DIM_INT32) v, bmp->i.width*bmp->i.height );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_FLOAT)
      fill_channel<DIM_FLOAT32>( (DIM_FLOAT32*) bmp->bits[sample], (DIM_FLOAT32) v, bmp->i.width*bmp->i.height );
    else
    if (bmp->i.depth==64 && bmp->i.pixelType==D_FMT_FLOAT)
      fill_channel<DIM_FLOAT64>( (DIM_FLOAT64*) bmp->bits[sample], (DIM_FLOAT64) v, bmp->i.width*bmp->i.height );

  } // sample
}

bool TDimImage::isUnTypedDepth() const {
  if ( bmp->i.depth==8) return false;
  else
  if ( bmp->i.depth==16) return false;
  else
  if ( bmp->i.depth==32) return false;
  else
  if ( bmp->i.depth==64) return false;
  else
  return true;
}

TDimImage TDimImage::ensureTypedDepth( ) const {
  TDimImage img;
  if (bmp==NULL) return img;

  if (bmp->i.depth != 12 && bmp->i.depth != 4 && bmp->i.depth != 1) {
    return *this;
  }

  DIM_UINT w = bmp->i.width;
  DIM_UINT h = bmp->i.height;
  
  unsigned int out_depth = 8;
  if (bmp->i.depth == 12) out_depth = 16;

  if ( img.alloc( w, h, bmp->i.samples, out_depth ) == 0 )
  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {
    for (unsigned int y=0; y<h; ++y ) {
      void *dest = img.scanLine( sample, y ); 
      void *src = this->scanLine( sample, y );

      if (bmp->i.depth == 1)
        cnv_buffer_1to8bit( (unsigned char *)dest, (unsigned char *)src, w );
      else
      if (bmp->i.depth == 4)
        cnv_buffer_4to8bit( (unsigned char *)dest, (unsigned char *)src, w );
      else
      if (bmp->i.depth == 12)
        cnv_buffer_12to16bit( (unsigned char *)dest, (unsigned char *)src, w );

    } // for y
  } // sample
  
  img.bmp->i = this->bmp->i;
  img.bmp->i.depth = out_depth;
  //img.bmp->i.pixelType = DIM_TAG_SHORT;

  return img;
}

// return a pointer to the buffer of line y formed in iterleaved format RGB
// the image must be in 8 bpp, otherwise NULL is returned
DIM_UCHAR *TDimImage::scanLineRGB( DIM_ULONG y ) {
  if (depth() != 8) return 0;
  buf.allocate( width()*3 );
  buf.fill(0);
  int chans = dim::min<int>( 3, samples() );

  for (unsigned int s=0; s<(unsigned int) chans; ++s) {
    DIM_UCHAR *line_o = buf.bytes() + s;
    DIM_UCHAR *line_i = scanLine( s, y );        
    for (unsigned int x=0; x<width(); ++x) {
       *line_o = line_i[x];
       line_o += 3;
    } // x
  } // s

  return buf.bytes();
}

//------------------------------------------------------------------------------------
// channels
//------------------------------------------------------------------------------------

DTagMap remapMetadata( const DTagMap &md, unsigned int samples, const std::vector<int> &mapping ) {

  if (md.size()==0) return md;
  DTagMap metadata = md;

  std::vector<std::string> channel_names;
  for (int i=0; i<samples; ++i)
    channel_names.push_back( metadata.get_value(xstring::xprintf("channel_%d_name",i)) );

  for (int i=0; i<mapping.size(); ++i) {
    xstring new_name("empty");
    if (mapping[i]>=0 || mapping[i]<samples)
      new_name = channel_names[mapping[i]];
    metadata.set_value( xstring::xprintf("channel_%d_name",i), new_name);
  }

  return metadata;
}

// fast but potentially dangerous function! It will affect all shared references to the same image!!!
// do deepCopy() before if you might have some shared references
// the result will have as many channels as there are entries in mapping
// invalid channel numbers or -1 will become black channels
// all black channels will point to the same area, so do deepCopy() if you'll modify them
void TDimImage::remapChannels( const std::vector<int> &mapping ) {

  metadata = remapMetadata( metadata, bmp->i.samples, mapping );

  // check if we have any black channels
  bool empty_channels = false;
  for (unsigned int i=0; i<mapping.size(); ++i)
    if (mapping[i]<0 || mapping[i]>=(int)bmp->i.samples) {
      empty_channels = true;
      break;
    }

  // if there are empty channels, allocate space for one channel and init it to 0
  void *empty_buffer = NULL;
  long size = bytesPerChan( );
  if (empty_channels) {
    empty_buffer = new DIM_UCHAR [ size ];
    memset( empty_buffer, 0, size );
  }

  // create a map to actual channel pointers
  std::vector<void*> channel_map( mapping.size() );
  
  for (unsigned int i=0; i<mapping.size(); ++i)
    if (mapping[i]<0 || mapping[i]>=(int)bmp->i.samples)
      channel_map[i] = empty_buffer;
    else
      channel_map[i] = bmp->bits[mapping[i]];

  // find unreferenced channels and destroy them
  for (unsigned int sample=0; sample<bmp->i.samples; ++sample) {
    bool found = false;
    for (unsigned int j=0; j<channel_map.size(); ++j)
      if (bmp->bits[sample] == channel_map[j]) { found = true; break; }
    
    if (!found) {
      delete [] (DIM_UCHAR*) bmp->bits[sample];
      bmp->bits[sample] = NULL;
    }
  }

  // reinit channels
  for (unsigned int sample=0; sample<DIM_MAX_CHANNELS; ++sample)
    bmp->bits[sample] = NULL;

  // map channels
  for (unsigned int sample=0; sample<channel_map.size(); ++sample)
    bmp->bits[sample] = channel_map[sample];
  
  bmp->i.samples = channel_map.size();
}

//------------------------------------------------------------------------------------
// resize
//------------------------------------------------------------------------------------

DTagMap resizeMetadata( const DTagMap &md, unsigned int w_to, unsigned int h_to, unsigned int w_in, unsigned int h_in ) {
  DTagMap metadata = md;
  if ( metadata.hasKey("pixel_resolution_x") ) {
    double new_res = metadata.get_value_double("pixel_resolution_x", 0) * ((double) w_in /(double) w_to );
    metadata.set_value("pixel_resolution_x", new_res);
  }
  if ( metadata.hasKey("pixel_resolution_y") ) {
    double new_res = metadata.get_value_double("pixel_resolution_y", 0) * ((double) h_in /(double) h_to );
    metadata.set_value("pixel_resolution_y", new_res);
  }
  return metadata;
}

template <typename T>
void downsample_line ( void *pdest, void *psrc1, void *psrc2, unsigned int &w ) {
  T *src1 = (T*) psrc1;
  T *src2 = (T*) psrc2;
  T *dest = (T*) pdest;

  unsigned int x2=0;
  for (unsigned int x=0; x<w; ++x) {
    dest[x] = (src1[x2] + src1[x2+1] + src2[x2] + src2[x2+1]) / 4;
    x2+=2;
  }
}

TDimImage TDimImage::downSampleBy2x( ) const {
  TDimImage img;
  if (bmp==NULL) return img;
  DIM_UINT w = bmp->i.width / 2;
  DIM_UINT h = bmp->i.height / 2;

  if ( img.alloc( w, h, bmp->i.samples, bmp->i.depth ) == 0 )
  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {
    unsigned int y2 = 0;
    for (unsigned int y=0; y<h; ++y ) {
      void *dest = img.scanLine( sample, y ); 
      void *src1 = this->scanLine( sample, y2 );
      void *src2 = this->scanLine( sample, y2+1 );
      y2 += 2;

      if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_UNSIGNED)
        downsample_line<DIM_UINT8> ( dest, src1, src2, w );
      else
      if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_UNSIGNED)
        downsample_line<DIM_UINT16> ( dest, src1, src2, w );
      else
      if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_UNSIGNED)
        downsample_line<DIM_UINT32> ( dest, src1, src2, w );
      else
      if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_SIGNED)
        downsample_line<DIM_INT8> ( dest, src1, src2, w );
      else
      if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_SIGNED)
        downsample_line<DIM_INT16> ( dest, src1, src2, w );
      else
      if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_SIGNED)
        downsample_line<DIM_INT32> ( dest, src1, src2, w );
      else
      if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_FLOAT)
        downsample_line<DIM_FLOAT32> ( dest, src1, src2, w );
      else
      if (bmp->i.depth==64 && bmp->i.pixelType==D_FMT_FLOAT)
        downsample_line<DIM_FLOAT64> ( dest, src1, src2, w );
    }

  } // sample

  img.bmp->i = this->bmp->i;
  img.bmp->i.width  = w;
  img.bmp->i.height = h;
  img.metadata = resizeMetadata( this->metadata, w, h, width(), height() );
  return img;
}

//------------------------------------------------------------------------------------
// Interpolation
//------------------------------------------------------------------------------------

template <typename T, typename Tw>
void image_resample ( T *pdest, unsigned int w_to, unsigned int h_to, unsigned int offset_to,
                      const T *psrc,  unsigned int w_in, unsigned int h_in, unsigned int offset_in, TDimImage::ResizeMethod method ) {

  if (method == TDimImage::szNearestNeighbor)
    image_resample_NN<T, Tw>( pdest, w_to, h_to, offset_to, psrc, w_in, h_in, offset_in );

  if (method == TDimImage::szBiLinear)
    image_resample_BL<T, Tw>( pdest, w_to, h_to, offset_to, psrc, w_in, h_in, offset_in );

  if (method == TDimImage::szBiCubic)
    image_resample_BC<T, Tw>( pdest, w_to, h_to, offset_to, psrc, w_in, h_in, offset_in );

}

TDimImage TDimImage::resample( int w, int h, ResizeMethod method, bool keep_aspect_ratio ) const {
  TDimImage img;
  if (bmp==NULL) return img;
  if (bmp->i.width==w && bmp->i.height==h) return *this;
  if (w==0 && h==0) return *this;

  if (keep_aspect_ratio)
    if ( (width()/(float)w) >= (height()/(float)h) ) h = 0; else w = 0;

  // it's allowed to specify only one of the sizes, the other one will be computed
  if (w == 0)
    w = dim::round<unsigned int>( width() / (height()/(float)h) );
  if (h == 0)
    h = dim::round<unsigned int>( height() / (width()/(float)w) );

  if ( img.alloc( w, h, bmp->i.samples, bmp->i.depth ) == 0 )
  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {

    if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_UNSIGNED)
      image_resample<DIM_UCHAR, float>( (DIM_UCHAR*) img.bits(sample), img.width(), img.height(),img.width(),
                                 (DIM_UCHAR*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );
    else
    if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_UNSIGNED)
      image_resample<DIM_UINT16, float>( (DIM_UINT16*) img.bits(sample), img.width(), img.height(),img.width(),
                                  (DIM_UINT16*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_UNSIGNED)
      image_resample<DIM_UINT32, double>( (DIM_UINT32*) img.bits(sample), img.width(), img.height(),img.width(),
                                  (DIM_UINT32*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );
    else
    if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_SIGNED)
      image_resample<DIM_INT8, float>( (DIM_INT8*) img.bits(sample), img.width(), img.height(),img.width(),
                                 (DIM_INT8*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );
    else
    if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_SIGNED)
      image_resample<DIM_INT16, float>( (DIM_INT16*) img.bits(sample), img.width(), img.height(),img.width(),
                                  (DIM_INT16*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_SIGNED)
      image_resample<DIM_INT32, double>( (DIM_INT32*) img.bits(sample), img.width(), img.height(),img.width(),
                                  (DIM_INT32*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_FLOAT)
      image_resample<DIM_FLOAT32, double>( (DIM_FLOAT32*) img.bits(sample), img.width(), img.height(),img.width(),
                                   (DIM_FLOAT32*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );
    else
    if (bmp->i.depth==64 && bmp->i.pixelType==D_FMT_FLOAT)
      image_resample<DIM_FLOAT64, double>( (DIM_FLOAT64*) img.bits(sample), img.width(), img.height(),img.width(),
                                   (DIM_FLOAT64*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, method );

  } // sample

  img.bmp->i = this->bmp->i;
  img.bmp->i.width  = w;
  img.bmp->i.height = h;
  img.metadata = resizeMetadata( this->metadata, w, h, width(), height() );
  return img;
}

TDimImage TDimImage::resize( int w, int h, ResizeMethod method, bool keep_aspect_ratio ) const {
  if (bmp==NULL) return TDimImage();
  if (bmp->i.width==w && bmp->i.height==h) return *this;
  if (w==0 && h==0) return *this;

  if (keep_aspect_ratio)
    if ( (width()/(float)w) >= (height()/(float)h) ) h = 0; else w = 0;

  // it's allowed to specify only one of the sizes, the other one will be computed
  if (w == 0)
    w = dim::round<unsigned int>( width() / (height()/(float)h) );
  if (h == 0)
    h = dim::round<unsigned int>( height() / (width()/(float)w) );

  // use pyramid if the size difference is large enough
  double vr = (double) std::max<double>(w, width()) / (double) std::min<double>(w, width());
  double hr = (double) std::max<double>(h, height()) / (double) std::min<double>(h, height());
  double rat = std::max(vr, hr);
  if (rat<1.9 || width()<=4096 && height()<=4096 || w>=width() && h>=height()) 
    return resample( w, h, method, keep_aspect_ratio );

  // use image pyramid
  DImagePyramid pyramid;
  pyramid.createFrom( *this );
  int level = pyramid.levelClosestTop( w, h );
  TDimImage *image = pyramid.imageAt(level);
  if (image->isNull()) return resample( w, h, method, keep_aspect_ratio );
  TDimImage img = image->resample( w, h, method, keep_aspect_ratio );
  pyramid.clear();

  // copy some important info stored in the original image
  img.bmp->i = this->bmp->i;
  img.bmp->i.width  = w;
  img.bmp->i.height = h;
  img.metadata = resizeMetadata( this->metadata, w, h, width(), height() );
  return img;
}


//------------------------------------------------------------------------------------
// Rotation
//------------------------------------------------------------------------------------

DTagMap rotateMetadata( const DTagMap &md, const double &deg ) {
  DTagMap metadata = md;
  if (fabs(deg)!=90) return metadata;

  if ( metadata.hasKey("pixel_resolution_x") && metadata.hasKey("pixel_resolution_y") ) {
    double y_res = metadata.get_value_double("pixel_resolution_x", 0);
    double x_res = metadata.get_value_double("pixel_resolution_y", 0);
    metadata.set_value("pixel_resolution_x", x_res);
    metadata.set_value("pixel_resolution_y", y_res);
  }
  return metadata;
}

template <typename T>
void image_rotate ( T *pdest, unsigned int w_to, unsigned int h_to, unsigned int offset_to,
                      const T *psrc,  unsigned int w_in, unsigned int h_in, unsigned int offset_in, 
                      double deg ) {

  if (deg == 90)
    image_rotate_right<T>( pdest, w_to, h_to, offset_to, psrc, w_in, h_in, offset_in );

  if (deg == -90)
    image_rotate_left<T>( pdest, w_to, h_to, offset_to, psrc, w_in, h_in, offset_in );

  if (deg == 180) {
    image_flip<T>( pdest, w_to, h_to, offset_to, psrc, w_in, h_in, offset_in );
    image_mirror<T>( pdest, w_to, h_to, offset_to, pdest, w_to, h_to, offset_to );
  }

}

// only available values now are +90, -90 and 180
TDimImage TDimImage::rotate( double deg ) const {
  TDimImage img;
  if (bmp==NULL) return img;
  if (deg!=90 && deg!=-90 && deg!=180) return *this;

  int w = this->width();
  int h = this->height();
  if (fabs(deg) == 90) {
    h = this->width();
    w = this->height();
  }

  if ( img.alloc( w, h, bmp->i.samples, bmp->i.depth ) == 0 )
  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {

    if (bmp->i.depth==8)
      image_rotate<DIM_UCHAR>( (DIM_UCHAR*) img.bits(sample), img.width(), img.height(),img.width(),
                               (DIM_UCHAR*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, deg );
    else
    if (bmp->i.depth==16)
      image_rotate<DIM_UINT16>( (DIM_UINT16*) img.bits(sample), img.width(), img.height(),img.width(),
                                (DIM_UINT16*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, deg );
    else
    if (bmp->i.depth==32)
      image_rotate<DIM_UINT32>( (DIM_UINT32*) img.bits(sample), img.width(), img.height(),img.width(),
                                (DIM_UINT32*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, deg );
    else
    if (bmp->i.depth==64)
      image_rotate<DIM_FLOAT64>( (DIM_FLOAT64*) img.bits(sample), img.width(), img.height(),img.width(),
                                (DIM_FLOAT64*) bmp->bits[sample], bmp->i.width, bmp->i.height, bmp->i.width, deg );
  } // sample

  img.bmp->i = this->bmp->i;
  img.bmp->i.width  = w;
  img.bmp->i.height = h;
  img.metadata = rotateMetadata( this->metadata, deg );
  return img;
}

//------------------------------------------------------------------------------------
// Projection
//------------------------------------------------------------------------------------


template <typename T>
void max_lines ( void *pdest, void *psrc, unsigned int &w ) {
  T *src = (T*) psrc;
  T *dest = (T*) pdest;

  for (unsigned int x=0; x<w; ++x)
    dest[x] = dim::max<T>(src[x], dest[x]);
}

template <typename T>
void min_lines ( void *pdest, void *psrc, unsigned int &w ) {
  T *src = (T*) psrc;
  T *dest = (T*) pdest;

  for (unsigned int x=0; x<w; ++x)
    dest[x] = dim::min<T>(src[x], dest[x]);
}

template <typename T, typename F>
bool TDimImage::pixel_arithmetic( const TDimImage &img, F func ) {

  if (bmp==NULL) return false;
  if (img.width() != this->width()) return false;
  if (img.height() != this->height()) return false;
  if (img.samples() != this->samples()) return false;
  if (img.depth() != this->depth()) return false;

  DIM_UINT w = bmp->i.width;
  DIM_UINT h = bmp->i.height;

  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {
    for (unsigned int y=0; y<h; ++y ) {
      void *src = img.scanLine( sample, y ); 
      void *dest = this->scanLine( sample, y );
      func ( dest, src, w );
    }
  } // sample
  return true;
}

bool TDimImage::pixelArithmeticMin( const TDimImage &img ) {
  if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_arithmetic<DIM_UINT8> ( img, min_lines<DIM_UINT8> );
  else
  if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_arithmetic<DIM_UINT16> ( img, min_lines<DIM_UINT16> );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_arithmetic<DIM_UINT32> ( img, min_lines<DIM_UINT32> );
  else
  if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_arithmetic<DIM_INT8> ( img, min_lines<DIM_INT8> );
  else
  if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_arithmetic<DIM_INT16> ( img, min_lines<DIM_INT16> );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_arithmetic<DIM_INT32> ( img, min_lines<DIM_INT32> );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_FLOAT)
    return pixel_arithmetic<DIM_FLOAT32> ( img, min_lines<DIM_FLOAT32> );
  else
  if (bmp->i.depth == 64 && bmp->i.pixelType==D_FMT_FLOAT)
    return pixel_arithmetic<DIM_FLOAT64> ( img, min_lines<DIM_FLOAT64> );
  else
  return false;
}

bool TDimImage::pixelArithmeticMax( const TDimImage &img ) {
  if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_arithmetic<DIM_UINT8> ( img, max_lines<DIM_UINT8> );
  else
  if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_arithmetic<DIM_UINT16> ( img, max_lines<DIM_UINT16> );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_arithmetic<DIM_UINT32> ( img, max_lines<DIM_UINT32> );
  else
  if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_arithmetic<DIM_INT8> ( img, max_lines<DIM_INT8> );
  else
  if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_arithmetic<DIM_INT16> ( img, max_lines<DIM_INT16> );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_arithmetic<DIM_INT32> ( img, max_lines<DIM_INT32> );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_FLOAT)
    return pixel_arithmetic<DIM_FLOAT32> ( img, max_lines<DIM_FLOAT32> );
  else
  if (bmp->i.depth == 64 && bmp->i.pixelType==D_FMT_FLOAT)
    return pixel_arithmetic<DIM_FLOAT64> ( img, max_lines<DIM_FLOAT64> );
  else
  return false;
}


//------------------------------------------------------------------------------------
// Negative
//------------------------------------------------------------------------------------

template <typename T>
void image_negative ( void *pdest, void *psrc, unsigned int &w ) {
  T *src = (T*) psrc;
  T *dest = (T*) pdest;
  T max_val = std::numeric_limits<T>::max();

  for (unsigned int x=0; x<w; ++x)
    dest[x] = max_val - src[x];
}

TDimImage TDimImage::negative() const {
  TDimImage img;
  if (bmp==NULL) return img;
  img = this->deepCopy();
  unsigned int plane_size_pixels = img.width()*img.height();

  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {

    if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_UNSIGNED)
      image_negative<DIM_UINT8>( img.bits(sample), bmp->bits[sample], plane_size_pixels );
    else
    if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_UNSIGNED)
      image_negative<DIM_UINT16>( img.bits(sample), bmp->bits[sample], plane_size_pixels );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_UNSIGNED)
      image_negative<DIM_UINT32>( img.bits(sample), bmp->bits[sample], plane_size_pixels );
    else
    if (bmp->i.depth==8 && bmp->i.pixelType==D_FMT_SIGNED)
      image_negative<DIM_INT8>( img.bits(sample), bmp->bits[sample], plane_size_pixels );
    else
    if (bmp->i.depth==16 && bmp->i.pixelType==D_FMT_SIGNED)
      image_negative<DIM_INT16>( img.bits(sample), bmp->bits[sample], plane_size_pixels );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_SIGNED)
      image_negative<DIM_INT32>( img.bits(sample), bmp->bits[sample], plane_size_pixels );
    else
    if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_FLOAT)
      image_negative<DIM_FLOAT32>( img.bits(sample), bmp->bits[sample], plane_size_pixels );
    else
    if (bmp->i.depth==64 && bmp->i.pixelType==D_FMT_FLOAT)
      image_negative<DIM_FLOAT64>( img.bits(sample), bmp->bits[sample], plane_size_pixels );

  } // sample

  return img;
}

//------------------------------------------------------------------------------------
// Row scan
//------------------------------------------------------------------------------------
template <typename T>
void image_row_scan ( void *pdest, const TDimImage &img, unsigned int &sample, unsigned int &x ) {
  T *dest = (T*) pdest;
  for (unsigned int y=0; y<img.height(); ++y) {
    T *src = (T*) img.scanLine( sample, y );
    dest[y] = src[x];
  }
}

void TDimImage::scanRow( DIM_UINT sample, unsigned int x, DIM_UCHAR *buf ) const {

  if (bmp->i.depth==8)
    image_row_scan<DIM_UCHAR>( buf, *this, sample, x );
  else
  if (bmp->i.depth==16)
    image_row_scan<DIM_UINT16>( buf, *this, sample, x );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType!=D_FMT_FLOAT)
    image_row_scan<DIM_UINT32>( buf, *this, sample, x );
  else
  if (bmp->i.depth==32 && bmp->i.pixelType==D_FMT_FLOAT)
    image_row_scan<DIM_FLOAT32>( buf, *this, sample, x );
  else
  if (bmp->i.depth==64 && bmp->i.pixelType==D_FMT_FLOAT)
    image_row_scan<DIM_FLOAT64>( buf, *this, sample, x );
}

//------------------------------------------------------------------------------------
// Operations generics
//------------------------------------------------------------------------------------

template <typename T, typename F, typename A>
bool TDimImage::pixel_operations( F func, const A &args ) {

  if (bmp==NULL) return false;
  DIM_UINT w = bmp->i.width;
  DIM_UINT h = bmp->i.height;

  for (unsigned int sample=0; sample<bmp->i.samples; ++sample ) {
    for (unsigned int y=0; y<h; ++y ) {
      void *src = this->scanLine( sample, y );
      func ( src, w, args );
    }
  } // sample
  return true;
}

//------------------------------------------------------------------------------------
// Threshold
//------------------------------------------------------------------------------------

template <typename T>
void operation_threshold ( void *p, unsigned int &w, const double &th ) {
  T *src = (T*) p;
  T max_val = std::numeric_limits<T>::max();
  for (unsigned int x=0; x<w; ++x)
    if (src[x] < th) 
      src[x] = 0; 
    else 
      src[x] = max_val;
}

bool TDimImage::operationThershold( const double &th ) {

  if (bmp->i.depth == 8 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_operations<DIM_UINT8>  ( operation_threshold<DIM_UINT8>, th );
  else
  if (bmp->i.depth == 16 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_operations<DIM_UINT16> ( operation_threshold<DIM_UINT16>, th );
  else      
  if (bmp->i.depth == 32 && bmp->i.pixelType==D_FMT_UNSIGNED)
    return pixel_operations<DIM_UINT32> ( operation_threshold<DIM_UINT32>, th );
  else
  if (bmp->i.depth == 8 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_operations<DIM_INT8>  ( operation_threshold<DIM_INT8>, th );
  else
  if (bmp->i.depth == 16 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_operations<DIM_INT16> ( operation_threshold<DIM_INT16>, th );
  else      
  if (bmp->i.depth == 32 && bmp->i.pixelType==D_FMT_SIGNED)
    return pixel_operations<DIM_INT32> ( operation_threshold<DIM_INT32>, th );
  else    
  if (bmp->i.depth == 32 && bmp->i.pixelType==D_FMT_FLOAT)
    return pixel_operations<DIM_FLOAT32> ( operation_threshold<DIM_FLOAT32>, th );
  else      
  if (bmp->i.depth == 64 && bmp->i.pixelType==D_FMT_FLOAT)
    return pixel_operations<DIM_FLOAT64> ( operation_threshold<DIM_FLOAT64>, th );
  else
  return false;
}

//*****************************************************************************
//  DImageHistogram
//*****************************************************************************

DImageHistogram::DImageHistogram( int channels, int bpp, const D_DataFormat &fmt ) {
  if (channels > 0)
    histograms.resize( channels ); 

  if (bpp > 0)  
  for (int c=0; c<channels; ++c) {
    histograms[c].init( bpp, fmt );
  }
}

DImageHistogram::~DImageHistogram() {

}

// mask has to be an 8bpp image where pixels >128 belong to the object of interest
// if mask has 1 channel it's going to be used for all channels, otherwise they should match
void DImageHistogram::fromImage( const TDimImage &img, const TDimImage *mask ) {  
  if (mask && mask->depth()!=8) return;
  if (mask && mask->samples()>1 && mask->samples()<img.samples() ) return;
  
  histograms.resize( img.samples() ); 

  if (mask)
    for (unsigned int c=0; c<img.samples(); ++c)
      histograms[c].newData( img.depth(), img.bits(c), img.numPixels(), img.pixelType(), (unsigned char*) mask->bits(c) );
  else
    for (unsigned int c=0; c<img.samples(); ++c)
      histograms[c].newData( img.depth(), img.bits(c), img.numPixels(), img.pixelType() );
}

//*****************************************************************************
//  DImageLut
//*****************************************************************************

void DImageLut::init( const DImageHistogram &in, const DImageHistogram &out ) {
  if (in.size() != out.size()) return;
  luts.resize( in.size() );
  for (int i=0; i<in.size(); ++i) {
    luts[i].init( *in[i], *out[i] );
  }
}

void DImageLut::init( const DImageHistogram &in, const DImageHistogram &out, DimLut::LutType type ) {
  if (in.size() != out.size()) return;
  luts.resize( in.size() );
  for (int i=0; i<in.size(); ++i) {
    luts[i].init( *in[i], *out[i], type );
  }
}

void DImageLut::init( const DImageHistogram &in, const DImageHistogram &out, DimLut::DimLutGenerator custom_generator ) {
  if (in.size() != out.size()) return;
  luts.resize( in.size() );
  for (int i=0; i<in.size(); ++i) {
    luts[i].init( *in[i], *out[i], custom_generator );
  }
}


