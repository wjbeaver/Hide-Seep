/*******************************************************************************

  Defines Image Stack Class
  
  Programmer: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    03/23/2004 18:03 - First creation
      
  ver: 1
        
*******************************************************************************/

#ifndef DIM_IMAGE_STACK_H
#define DIM_IMAGE_STACK_H

#include <vector>
#include <map>

#include "tag_map.h"
#include "dim_image.h"
#include "dim_histogram.h"

class TMetaFormatManager;

//------------------------------------------------------------------------------
// DImageStack
//------------------------------------------------------------------------------

class DImageStack {
  public:
    DImageStack();
    DImageStack( const char *fileName, unsigned int limit_width=0, unsigned int limit_height=0, int only_channel=-1 );
    DImageStack( const std::string &fileName, unsigned int limit_width=0, unsigned int limit_height=0, int only_channel=-1 );
    ~DImageStack();

    void        free();
    void        clear() { free(); }
    DImageStack deepCopy() const;  
    DImageStack deepCopy( double x, double y, double z, double w=0, double h=0, double d=0 ) const;

    bool       isReady() const { return !handling_image; } // true if images can be used
    bool       isEmpty() const;

    int        size() const { return (int) images.size(); }
    int        count() const { return size(); }
    int        numberImages() const { return size(); }
    int        numberPlanes() const { return size(); }
    bool       planeInRange( int p ) const { if (p<0) return false; if (p>=size()) return false; return true; }
    void       setNumberPlanes( unsigned int );

    double     bytesInStack() const;

    inline DIM_ULONG    width()     const { if (size()<=0) return 0; else return images[0].width();  }
    inline DIM_ULONG    height()    const { if (size()<=0) return 0; else return images[0].height(); }
    inline DIM_ULONG    length()    const { return size(); }
    inline DIM_UINT     depth()     const { if (size()<=0) return 0; else return images[0].depth();  }
    inline DIM_UINT     samples()   const { if (size()<=0) return 0; else return images[0].samples(); }
    inline D_DataFormat pixelType() const { if (size()<=0) return D_FMT_UNSIGNED; else return images[0].pixelType(); }

    TDimImage *imageAt( unsigned int position ) const;
    TDimImage *image() const; // image at the currently selected position
    inline TDimImage *operator[](unsigned int p) const { return imageAt(p); }

    void append( const TDimImage &img ) { images.push_back(img); }

    // return true if the level exists
    bool positionPrev();
    bool positionNext();
    bool position0();
    bool positionFirst() { return position0(); }
    bool positionLast();
    bool positionSet( unsigned int l );
    int  positionCurrent() const { return cur_position; }

    const std::map<std::string, std::string> &get_metadata() const { return metadata; }
    std::string get_metadata_tag( const std::string &key, const std::string &def ) const { return metadata.get_value( key, def ); }
    int         get_metadata_tag_int( const std::string &key, const int &def ) const { return metadata.get_value_int( key, def ); }
    double      get_metadata_tag_double( const std::string &key, const double &def ) const { return metadata.get_value_double( key, def ); }

    // I/O

    virtual bool fromFile( const char *fileName, unsigned int limit_width=0, unsigned int limit_height=0, int channel=-1 );
    bool fromFile( const std::string &fileName, unsigned int limit_width=0, unsigned int limit_height=0, int channel=-1 ) 
    { return fromFile( fileName.c_str(), limit_width, limit_height, channel ); }

    bool fromFileManager( TMetaFormatManager *m, const std::vector<unsigned int> &pages );

    virtual bool toFile( const char *fileName, const char *formatName );
    bool toFile( const std::string &fileName, const std::string &formatName ) {
      return toFile( fileName.c_str(), formatName.c_str() ); }

    // Operations
    void convertToDepth( int depth, DimLut::LutType method = DimLut::ltLinearFullRange, bool planes_independent = false, D_DataFormat pxtype = D_FMT_UNDEFINED );
    void convertToDepth( const DImageLut & );
    void normalize( int to_bpp = 8, bool planes_independent = false );
    void ensureTypedDepth();

    void resize( int w, int h=0, int d=0, TDimImage::ResizeMethod method = TDimImage::szNearestNeighbor, bool keep_aspect_ratio = false );

    void ROI( DIM_UINT x, DIM_UINT y, DIM_UINT z, DIM_UINT w, DIM_UINT h, DIM_UINT d );

    // only available values now are +90, -90 and 180
    void rotate( double deg );

    void negative();
    void discardLut();
    void remapChannels( const std::vector<int> &mapping );

    TDimImage projectionXZAxis( unsigned int y );
    TDimImage projectionYZAxis( unsigned int x );

    TDimImage pixelArithmeticMax() const;
    TDimImage pixelArithmeticMin() const;

  protected:
    std::vector<TDimImage> images;
    DTagMap metadata;

    TDimImage empty_image;
    int  cur_position;
    bool handling_image;

  protected:
    void init();

  // callbacks
  public:
    TDimProgressProc progress_proc;
    TDimErrorProc error_proc;
    TDimTestAbortProc test_abort_proc;

  protected:
    void do_progress( long done, long total, char *descr ) {
      if (progress_proc) progress_proc( done, total, descr );
    }
    bool progress_abort( ) {
      if (!test_abort_proc) return false;
      return (test_abort_proc() != 0);
    }

};


//------------------------------------------------------------------------------
// DStackHistogram
//------------------------------------------------------------------------------

class DStackHistogram: public DImageHistogram {
public:
  DStackHistogram( );
  DStackHistogram( const DImageStack &stack ) { fromImageStack(stack); }
  DStackHistogram( const DImageStack &stack, const TDimImage *mask ) { fromImageStack(stack, mask); }
  DStackHistogram( const DImageStack &stack, const DImageStack *mask ) { fromImageStack(stack, mask); }
  ~DStackHistogram();

  void fromImageStack( const DImageStack &stack );

  // mask has to be an 8bpp image where pixels >128 belong to the object of interest
  // if mask has 1 channel it's going to be used for all channels, otherwise they should match
  void fromImageStack( const DImageStack &, const TDimImage *mask );

  // mask has to be an 8bpp image where pixels >128 belong to the object of interest
  // if mask has 1 channel it's going to be used for all channels, otherwise they should match
  void fromImageStack( const DImageStack &, const DImageStack *mask );
};

#endif //DIM_IMAGE_STACK_H


