/*******************************************************************************

  Defines Image Class, it uses smart pointer technology to implement memory
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

  TODO:
    Add metadata structure and destroy it in the end

  History:
    03/23/2004 18:03 - First creation
      
  ver: 12
        
*******************************************************************************/

#ifndef DIM_IMAGE_H
#define DIM_IMAGE_H

#include "dim_img_format_interface.h"
#include "dim_img_format_utils.h"
#include "dim_buffer.h"
#include "dim_histogram.h"

#include "tag_map.h"

#define DIM_USE_IMAGEMANAGER

#ifdef DIM_USE_QT
#include <QImage>
#include <QPixmap>
#include <QPainter>
#endif //DIM_USE_QT

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#endif //WINDOWS

#ifdef BIM_USE_ITK
#include "itkImage.h"
#endif //BIM_USE_ITK

#include <cmath>
#include <vector>
#include <set>
#include <limits>
#include <map>

class DImageHistogram;
class DImageLut;

//------------------------------------------------------------------------------
// TDimImage
//------------------------------------------------------------------------------

class DImgRefs {
public:
  DImgRefs() { refs = 0; /* initImagePlanes( &bmp );*/ }
  ~DImgRefs() { }

  DImgRefs(const DImgRefs &ir) {
    this->refs = ir.refs; 
    this->bmp  = ir.bmp;
  }

public:
  TDimImageBitmap bmp;
  unsigned int    refs;
};

class TDimImage {
  public:
    enum ResizeMethod { 
      szNearestNeighbor=0, 
      szBiLinear=1, 
      szBiCubic=2
    };

  public:
    TDimImage();
    ~TDimImage();

    TDimImage(DIM_UINT width, DIM_UINT height, DIM_UINT depth, DIM_UINT samples);
    #ifdef DIM_USE_IMAGEMANAGER
    TDimImage(const char *fileName, int page);
    TDimImage(const std::string &fileName, int page);
    #endif //DIM_USE_IMAGEMANAGER

    #ifdef DIM_USE_QT
    TDimImage(const QImage &qimg);
    #endif //DIM_USE_QT


    // allow copy constructor, it will only point to the same memory area
    TDimImage(const TDimImage& );
    
    // it will only point to the same memory area
    TDimImage &operator=(const TDimImage & );

    // will create a new memory area and copy the image there
    TDimImage  deepCopy() const;

    // will create a new memory area
    int        alloc( DIM_UINT w, DIM_UINT h, DIM_UINT samples, DIM_UINT depth );
    // will connect image to the new empty memory area
    void       free( );

    bool       create ( DIM_UINT width, DIM_UINT height, DIM_UINT depth, DIM_UINT samples )
    { return this->alloc( width, height, samples, depth ) ? true : false; }
    void       reset () { this->free(); }
    void       clear () { this->free(); }
    
    DIM_UCHAR *sampleBits( DIM_UINT sample ) const;
    DIM_UCHAR *scanLine( DIM_UINT sample, DIM_ULONG y ) const;
    DIM_UCHAR *pixelBits( DIM_UINT sample, DIM_ULONG x, DIM_ULONG y ) const;
    void       scanRow( DIM_UINT sample, unsigned int x, DIM_UCHAR *buf ) const;

    template <typename T>
    T pixel( DIM_UINT sample, DIM_ULONG x, DIM_ULONG y ) const;

    // return a pointer to the buffer of line y formed in iterleaved format RGB
    // the image must be in 8 bpp, otherwise NULL is returned
    DIM_UCHAR *scanLineRGB( DIM_ULONG y );

    bool       isNull() const { if (bmp!=NULL) return bmp->bits[0] == NULL; else return false; }
    bool       isEmpty() const { return isNull(); }
    void      *bits(const unsigned int &sample) const;
    
    void       fill( double );

    inline DIM_ULONG    bytesPerChan( ) const;
    inline DIM_ULONG    bytesPerLine( ) const;
    inline DIM_ULONG    bytesPerRow( ) const;
    inline DIM_ULONG    bytesInPixels( int n ) const;
    inline double       bytesInImage( ) const { return bytesPerChan()*samples(); }
    inline DIM_ULONG    availableColors( ) const;
    inline DIM_ULONG    width()     const { if (bmp==NULL) return 0; else return bmp->i.width;  }
    inline DIM_ULONG    height()    const { if (bmp==NULL) return 0; else return bmp->i.height; }
    inline DIM_ULONG    numPixels() const { if (bmp==NULL) return 0; else return bmp->i.width * bmp->i.height; }
    inline DIM_UINT     depth()     const { if (bmp==NULL) return 0; else return bmp->i.depth;  }
    inline DIM_UINT     samples()   const { if (bmp==NULL) return 0; else return bmp->i.samples; }
    inline DIM_UINT     channels()  const { return samples(); }
    inline D_DataFormat pixelType() const { if (bmp==NULL) return D_FMT_UNSIGNED; else return bmp->i.pixelType; }
    inline DIM_UINT     numT()      const { if (bmp==NULL) return 0; else return bmp->i.number_t; }
    inline DIM_UINT     numZ()      const { if (bmp==NULL) return 0; else return bmp->i.number_z; }

    void updateGeometry( const unsigned int &z=0, const unsigned int &t=0 );
    void updateResolution( const double r[4] );
    void updateResolution( const std::vector< double > &r ) { updateResolution( &r[0] ); } 

    TDimImageBitmap *imageBitmap() { return bmp; }

    std::string getTextInfo() const;

    //--------------------------------------------------------------------------    
    // LUT - palette
    //--------------------------------------------------------------------------
    bool       hasLut() const;
    int        lutSize() const;
    TDimRGBA*  palette() const { return bmp->i.lut.rgba; }
    TDimRGBA   lutColor( DIM_UINT i ) const;
    void       setLutColor( DIM_UINT i, TDimRGBA c );
    void       setLutNumColors( DIM_UINT n );
    void       discardLut() { setLutNumColors(0); }

    //--------------------------------------------------------------------------    
    // OS/Lib dependent stuff
    //--------------------------------------------------------------------------

    #ifdef DIM_USE_QT
    QImage  toQImage  ( ) const;
    QPixmap toQPixmap ( ) const;
    void fromQImage( const QImage &qimg );
    // paint( QPaintDevice, ROI ) const;
    void paint( QPainter *p ) const;
    #endif //DIM_USE_QT

    #ifdef WIN32
    HBITMAP toWinHBITMAP ( ) const;
    // paint( HWINDOW, ROI );
    #endif //WIN32

    #ifdef BIM_USE_ITK
    template <typename PixelType, class ImageType = itk::Image< PixelType, 2 > >
    ImageType::Pointer TDimImage::toItkImage( const unsigned int &channel ) const;

    template <typename PixelType, class ImageType = itk::Image< PixelType, 2 > >
    void TDimImage::fromItkImage( const ImageType *image );
    #endif //BIM_USE_ITK

    #ifdef DIM_USE_IMAGEMANAGER
    bool fromFile( const char *fileName, int page );
    bool fromFile( const std::string &fileName, int page ) { 
      return fromFile( fileName.c_str(), page ); }

    bool toFile( const char *fileName, const char *formatName, const char *options=NULL );
    bool toFile( const std::string &fileName, const std::string &formatName ) {
      return toFile( fileName.c_str(), formatName.c_str() ); }
    bool toFile( const std::string &fileName, const std::string &formatName, const std::string &options ) {
      return toFile( fileName.c_str(), formatName.c_str(), options.c_str() ); }
    #endif //DIM_USE_IMAGEMANAGER

    //--------------------------------------------------------------------------    
    // Metadata
    //--------------------------------------------------------------------------

    DTagMap     get_metadata() const { return metadata; }
    std::string get_metadata_tag( const std::string &key, const std::string &def ) const { return metadata.get_value( key, def ); }
    int         get_metadata_tag_int( const std::string &key, const int &def ) const { return metadata.get_value_int( key, def ); }
    double      get_metadata_tag_double( const std::string &key, const double &def ) const { return metadata.get_value_double( key, def ); }

    void        set_metadata( const DTagMap &md ) { metadata = md; }

    //--------------------------------------------------------------------------    
    // some operations
    //--------------------------------------------------------------------------
    TDimImage convertToDepth( int depth, DimLut::LutType method = DimLut::ltLinearFullRange, D_DataFormat pxtype = D_FMT_UNDEFINED, DImageHistogram *hist=0 ) const;
    TDimImage convertToDepth( const DImageLut & ) const;
    TDimImage normalize( int to_bpp = 8, DImageHistogram *hist=0 ) const;
    bool      isUnTypedDepth() const;    
    TDimImage ensureTypedDepth() const;

    TDimImage ROI( DIM_UINT x, DIM_UINT y, DIM_UINT w, DIM_UINT h ) const;
    
    //--------------------------------------------------------------------------    
    // geometry
    //--------------------------------------------------------------------------

    TDimImage downSampleBy2x() const;
    // resample is the direct resampling, pure brute force
    TDimImage resample( int w, int h=0, ResizeMethod method = szNearestNeighbor, bool keep_aspect_ratio = false ) const;
    // resize will use image pyramid if size difference is quite large
    TDimImage resize( int w, int h=0, ResizeMethod method = szNearestNeighbor, bool keep_aspect_ratio = false ) const;

    // only available values now are +90, -90 and 180
    TDimImage rotate( double deg ) const;

    TDimImage negative() const;

    //--------------------------------------------------------------------------    
    // some generics
    //--------------------------------------------------------------------------

    // Generic arithmetic with other image
    template <typename T, typename F>
    bool pixel_arithmetic( const TDimImage &img, F func );

    // examples of arithmetic
    bool pixelArithmeticMax( const TDimImage &img );
    bool pixelArithmeticMin( const TDimImage &img );

    // generic operations with this image pixels
    template <typename T, typename F, typename A>
    bool pixel_operations( F func, const A &args );

    // examples of operations
    bool operationThershold( const double &th );

    //--------------------------------------------------------------------------    
    // Channels
    //--------------------------------------------------------------------------

    // fast but potentially dangerous function! It will affect all shared references to the same image!!!
    // do deepCopy() before if you might have some shared references
    // the result will have as many channels as there are entries in mapping
    // invalid channel numbers or -1 will become black channels
    // all black channels will point to the same area, so do deepCopy() if you'll modify them
    // all copies of channels in different positions will simply point to memory locations so you can't modify them directly!!!!
    void remapChannels( const std::vector<int> &mapping );
    void remapToRGB();

    // since this method produces only one output channel it will not have any multiple channel pointers to the same meory space
    // althogh it will still affect other shared images so that it is recommended to use deepCopy() if you have any
    void extractChannel( int c );

    // creates an image with channels from both images starting with this
    TDimImage appendChannels( const TDimImage & ) const;

    // CHANNEL FUSION
    // fuse functions are slower but safer than the remap functions, they will physically create memory space for all output channels
    // they also allow mixing channels into the same output channel

    // generic channel fusion by selecting the max intensity of all inputs
    // * the size of the mapping vector dictates how many output channels there will be, 
    // * vector for each channel indicates which input channels are mixed to produce the output channel
    // * the output channel intensity equals the max of all inputs
    // note: see how this will be used in the overloaded fuse methods
    TDimImage fuse( const std::vector< std::set<int> > &mapping ) const;

    // this overloaded method provides a simple way of mixing up to 7 channels into the output 3 channel image
    // this method is mostly good for fluorescence imagery with little colocalization 
    TDimImage fuse( int red, int green, int blue, int yellow=-1, int magenta=-1, int cyan=-1, int gray=-1 ) const;

    // mixing channels using associated weights
    // * the size of the mapping vector dictates how many output channels there will be, 
    // * vector for each channel indicates which input channels are mixed to produce the output channel
    // * the pair should contain a number of the channel as first and weight 0-1 as a second
    // note: see how this will be used in fuseToGrayscale
    TDimImage fuse( const std::vector< std::vector< std::pair<unsigned int,float> > > &mapping ) const;

    // this requires and RGB image
    TDimImage fuseToGrayscale() const;

  private:
    // pointer to a shared bitmap
    TDimImageBitmap *bmp;

    // not shared image metadata - most of the time it is empty
    DTagMap metadata;

    // not shared buffer
    DTypedBuffer<unsigned char> buf;

  private:
    static std::vector<DImgRefs*> refs;
    
    inline void print_debug();

    int  getCurrentRefId();
    int  getRefId( TDimImageBitmap *b );
    void connectToMemory( TDimImageBitmap *b );
    void connectToNewMemory();
    void disconnectFromMemory();

};

//------------------------------------------------------------------------------
// DImageHistogram
//------------------------------------------------------------------------------

class DImageHistogram {
  public:
    DImageHistogram( int channels=0, int bpp=0, const D_DataFormat &fmt=D_FMT_UNSIGNED );
    
    // mask has to be an 8bpp image where pixels >128 belong to the object of interest
    // if mask has 1 channel it's going to be used for all channels, otherwise they should match
    DImageHistogram( const TDimImage &img, const TDimImage *mask=0 ) { fromImage(img, mask); }
    ~DImageHistogram();

    void clear( ) { histograms.clear(); }
    bool isValid() const { return histograms.size()>0 && histograms[0].isValid(); }
    
    // mask has to be an 8bpp image where pixels >128 belong to the object of interest
    // if mask has 1 channel it's going to be used for all channels, otherwise they should match
    void fromImage( const TDimImage &, const TDimImage *mask=0 );
    
    int size() const { return (int) histograms.size(); }
    int channels() const { return size(); }

    const DimHistogram& histogram_for_channel( int c ) const { return histograms[c]; }
    inline const DimHistogram* operator[](unsigned int c) const { return &histograms[c]; }

  public:
    // I/O
    bool to(const std::string &fileName);
    bool to(std::ostream *s);
    bool from(const std::string &fileName);
    bool from(std::istream *s);

  protected:
    std::vector<DimHistogram> histograms;
};

//------------------------------------------------------------------------------
// DImageLut
//------------------------------------------------------------------------------

class DImageLut {
  public:
    DImageLut() {}
    DImageLut( const DImageHistogram &in, const DImageHistogram &out ) { init(in, out); }
    DImageLut( const DImageHistogram &in, const DImageHistogram &out, DimLut::LutType type ) { init(in, out, type); }
    DImageLut( const DImageHistogram &in, const DImageHistogram &out, DimLut::DimLutGenerator custom_generator ) { init(in, out, custom_generator); }
    ~DImageLut() {}

    void init( const DImageHistogram &in, const DImageHistogram &out );
    void init( const DImageHistogram &in, const DImageHistogram &out, DimLut::LutType type );
    void init( const DImageHistogram &in, const DImageHistogram &out, DimLut::DimLutGenerator custom_generator );

    void clear( ) { luts.clear(); }
    
    int size() const { return (int) luts.size(); }
    int channels() const { return size(); }

    int depthInput()  const { if (size()<=0) return 0; return luts[0].depthInput(); }
    int depthOutput() const { if (size()<=0) return 0; return luts[0].depthOutput(); }
    D_DataFormat dataFormatInput()  const { if (size()<=0) return D_FMT_UNDEFINED; return luts[0].dataFormatInput(); }
    D_DataFormat dataFormatOutput() const { if (size()<=0) return D_FMT_UNDEFINED; return luts[0].dataFormatOutput(); }

    const DimLut& lut_for_channel( int c ) const { return luts[c]; }
    inline const DimLut* operator[](unsigned int c) const { return &luts[c]; }

  protected:
    std::vector<DimLut> luts;
};

/******************************************************************************
  TDimImage member functions
******************************************************************************/


inline DIM_ULONG TDimImage::bytesPerLine() const {
  if (bmp==NULL) return 0;
  return (long) ceil( ((double)(bmp->i.width * bmp->i.depth)) / 8.0 );
}

inline DIM_ULONG TDimImage::bytesPerRow() const {
  if (bmp==NULL) return 0;
  return (long) ceil( ((double)(bmp->i.height * bmp->i.depth)) / 8.0 );
}

inline DIM_ULONG TDimImage::bytesInPixels( int n ) const {
  if (bmp==NULL) return 0;
  return (DIM_ULONG) ceil( ((double)(n * bmp->i.depth)) / 8.0 );
}

inline DIM_ULONG TDimImage::bytesPerChan( ) const {
  if (bmp==NULL) return 0;
  return (DIM_ULONG) ceil( ((double)(bmp->i.width * bmp->i.depth)) / 8.0 ) * bmp->i.height;
}

inline DIM_ULONG TDimImage::availableColors( ) const {
  if (bmp==NULL) return 0;
  return (DIM_ULONG) pow( 2.0f, (float)(bmp->i.depth * bmp->i.samples) );
}

inline DIM_UCHAR *TDimImage::sampleBits( DIM_UINT s ) const {
  if (bmp==NULL) return NULL;
  if (s >= 512) return NULL;
  return (DIM_UCHAR *) (bmp->bits[s] ? bmp->bits[s] : NULL);
}

inline DIM_UCHAR *TDimImage::scanLine( DIM_UINT s, DIM_ULONG y ) const {
  if (bmp==NULL) return NULL;
  if (s >= 512) return NULL;
  if (y >= bmp->i.height) return NULL;

  return ((DIM_UCHAR *) bmp->bits[s]) + bytesPerLine()*y;
}

inline DIM_UCHAR *TDimImage::pixelBits( DIM_UINT s, DIM_ULONG x, DIM_ULONG y ) const {
  DIM_UCHAR *l = scanLine( s, y );
  if (l==NULL) return NULL;
  return l + bytesInPixels(x);
}

template <typename T>
T TDimImage::pixel( DIM_UINT sample, DIM_ULONG x, DIM_ULONG y ) const {
  if (x>width()) return std::numeric_limits<T>::quiet_NaN();
  if (y>height()) return std::numeric_limits<T>::quiet_NaN();
  T *p = (T*) pixelBits( sample, x, y );
  return *p;
}

inline bool TDimImage::hasLut() const {
  if (!bmp) return false;
  return bmp->i.lut.count>0;
}

inline int TDimImage::lutSize() const {
  if (!bmp) return 0;
  return bmp->i.lut.count;
}

inline TDimRGBA TDimImage::lutColor( DIM_UINT i ) const {
  if (bmp==NULL) return 0;
  if ( i>=bmp->i.lut.count ) return 0;
  return bmp->i.lut.rgba[i];
}

#endif //DIM_IMAGE_H


