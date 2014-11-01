/*******************************************************************************

  histogram and lut
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    10/20/2006 20:21 - First creation
    2007-06-26 12:18 - added lut class
    2010-01-22 17:06 - changed interface to support floating point data

  ver: 3
        
*******************************************************************************/

#ifndef DIM_HISTOGRAM_H
#define DIM_HISTOGRAM_H

#include <vector>

#include "dim_img_format_interface.h"

//******************************************************************************
// DimHistogram
//******************************************************************************

class DimLut;

class DimHistogram {
  public:
    static const unsigned int defaultSize=256; // 256, 65536

    DimHistogram( const unsigned int &bpp=0, const D_DataFormat &fmt=D_FMT_UNSIGNED );
    DimHistogram( const unsigned int &bpp, void *data, const unsigned int &num_data_points, const D_DataFormat &fmt=D_FMT_UNSIGNED, unsigned char *mask=0 );
    ~DimHistogram();

    void newData( const unsigned int &bpp, void *data, const unsigned int &num_data_points, const D_DataFormat &fmt=D_FMT_UNSIGNED, unsigned char *mask=0 );
    void clear();

    void init( const unsigned int &bpp, const D_DataFormat &fmt=D_FMT_UNSIGNED );
    void updateStats( void *data, const unsigned int &num_data_points, unsigned char *mask=0 );
    // careful with this function operating on data with 32 bits and above, stats should be properly updated for all data chunks if vary
    // use the updateStats method on each data chunk prior to calling addData on >=32bit data
    void addData( void *data, const unsigned int &num_data_points, unsigned char *mask=0 );

    unsigned int getDefaultSize() const { return default_size; }
    void         setDefaultSize(const unsigned int &v) { default_size=v; }

    unsigned int dataBpp()    const { return data_bpp; }
    D_DataFormat dataFormat() const { return data_fmt; }

    int bin_of_max_value() const;
    int bin_of_min_value() const;
    double max_value() const;
    double min_value() const;

    int bin_of_first_nonzero() const;
    int bin_of_last_nonzero()  const;
    int bin_number_nonzero()   const;
    int first_pos()  const { return bin_of_first_nonzero(); }
    int last_pos()   const { return bin_of_last_nonzero(); }
    int num_unique() const { return bin_number_nonzero(); }

    inline unsigned int bin_number() const { return size(); }
    inline unsigned int size() const { return hist.size(); }
    const std::vector<unsigned int> &get_hist( ) const { return hist; }

    double average() const;
    double std() const;
    unsigned int cumsum( const unsigned int &bin ) const;

    double get_shift() const { return shift; }
    double get_scale() const { return scale; }

    unsigned int get_value( const unsigned int &bin ) const;
    void set_value( const unsigned int &bin, const unsigned int &val );
    void append_value( const unsigned int &bin, const unsigned int &val );

    inline unsigned int operator[](unsigned int x) const { return hist[x]; }

    template <typename T>
    inline unsigned int bin_from( const T &data_point ) const;
    
  protected:
    unsigned int data_bpp;
    D_DataFormat data_fmt; // signed, unsigned, float
    int default_size;
    double shift, scale;
    double value_min, value_max;
    bool reversed_min_max;
    std::vector<unsigned int> hist;

    template <typename T> 
    void init_stats();

    void initStats();
    void getStats( void *data, const unsigned int &num_data_points, unsigned char *mask=0 );

    inline void recompute_shift_scale();

    template <typename T>
    void get_data_stats( T *data, const unsigned int &num_data_points, unsigned char *mask=0 );

    template <typename T>
    void update_data_stats( T *data, const unsigned int &num_data_points, unsigned char *mask=0 );

    template <typename T>
    void add_from_data( T *data, const unsigned int &num_data_points, unsigned char *mask=0 );
    
    template <typename T>
    void add_from_data_scale( T *data, const unsigned int &num_data_points, unsigned char *mask=0 );

    friend class DimLut;

};

//******************************************************************************
// DimLut
//******************************************************************************

class DimLut {
  public:

    enum LutType { 
      ltLinearFullRange=0, 
      ltLinearDataRange=1, 
      ltLinearDataTolerance=2,
      ltEqualize=3
    };

    typedef double StorageType; 
    typedef void (*DimLutGenerator)( const DimHistogram &in, std::vector<StorageType> &lut, unsigned int out_phys_range, void *args );

    DimLut( );
    DimLut( const DimHistogram &in, const DimHistogram &out );
    DimLut( const DimHistogram &in, const DimHistogram &out, const LutType &type );
    DimLut( const DimHistogram &in, const DimHistogram &out, DimLutGenerator custom_generator );
    ~DimLut();

    void init( const DimHistogram &in, const DimHistogram &out );
    void init( const DimHistogram &in, const DimHistogram &out, const LutType &type );
    void init( const DimHistogram &in, const DimHistogram &out, DimLutGenerator custom_generator );
    void clear( );

    unsigned int size() const { return lut.size(); }
    const std::vector<StorageType> &get_lut( ) const { return lut; }

    int depthInput()  const { return h_in.dataBpp(); }
    int depthOutput() const { return h_out.dataBpp(); }
    D_DataFormat dataFormatInput()  const { return h_in.dataFormat(); }
    D_DataFormat dataFormatOutput() const { return h_out.dataFormat(); }

    template <typename Tl>
    void         set_lut( const std::vector<Tl> & );
    
    StorageType   get_value( const unsigned int &pos ) const;
    void          set_value( const unsigned int &pos, const StorageType &val );
    inline StorageType operator[](unsigned int x) const { return lut[x]; }

    void apply( void *ibuf, const void *obuf, const unsigned int &num_data_points ) const;
    // generates values of output histogram, given the current lut and in histogram
    void apply( const DimHistogram &in, DimHistogram &out ) const;

  protected:
    std::vector<StorageType> lut;
    DimLutGenerator generator;
    DimHistogram h_in, h_out;
    void *internal_arguments;

    template <typename Ti, typename To>
    void apply_lut( const Ti *ibuf, To *obuf, const unsigned int &num_data_points ) const;

    template <typename Ti, typename To>
    void apply_lut_scale_from( const Ti *ibuf, To *obuf, const unsigned int &num_data_points ) const;

    template <typename Ti>
    inline void do_apply_lut( const Ti *ibuf, const void *obuf, const unsigned int &num_data_points ) const;

    template <typename Ti>
    inline void do_apply_lut_scale_from( const Ti *ibuf, const void *obuf, const unsigned int &num_data_points ) const;
};

//******************************************************************************
// Generators - default
//******************************************************************************

// misc
template <typename Tl>
void linear_range_generator( int b, int e, unsigned int out_phys_range, std::vector<Tl> &lut );

template <typename Tl>
void linear_full_range_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void *args );

template <typename Tl>
void linear_data_range_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void *args );

template <typename Tl>
void linear_data_tolerance_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void *args );

template <typename Tl>
void equalized_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void *args );

// args is a pointer to two int values
template <typename Tl>
void linear_custom_range_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void *args );

#endif //DIM_HISTOGRAM_H


