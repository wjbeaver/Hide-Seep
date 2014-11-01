/*******************************************************************************

  histogram and lutf
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>

  History:
    10/20/2006 20:21 - First creation
    2007-06-26 12:18 - added lut class
    2010-01-22 17:06 - changed interface to support floating point data

  ver: 3
        
*******************************************************************************/

#include "dim_histogram.h"
#include "xtypes.h"

#include "cmath"

#include <limits>

//******************************************************************************
// DimHistogram
//******************************************************************************

DimHistogram::DimHistogram( const unsigned int &bpp, const D_DataFormat &fmt ) {
  default_size = DimHistogram::defaultSize;
  init( bpp, fmt ); 
}

DimHistogram::DimHistogram( const unsigned int &bpp, void *data, const unsigned int &num_data_points, const D_DataFormat &fmt, unsigned char *mask ) {
  default_size = DimHistogram::defaultSize;
  newData( bpp, data, num_data_points, fmt, mask );
}

DimHistogram::~DimHistogram() {

}

void DimHistogram::init( const unsigned int &bpp, const D_DataFormat &fmt ) {
  data_bpp = bpp;
  data_fmt = fmt;

  shift=0; scale=1;
  value_min=0; value_max=0;
  reversed_min_max = false;
  hist.resize(0);

  if (bpp==0) return;
  else
  if ( bpp<=16 && (data_fmt==D_FMT_UNSIGNED || data_fmt==D_FMT_SIGNED) ) {
    hist.resize( (unsigned int) pow(2.0, fabs((double) bpp)), 0 );
  } else 
  if (bpp>16 || data_fmt==D_FMT_FLOAT) {
    hist.resize( default_size );
  }
  initStats();
}

void DimHistogram::clear( ) {
  hist = std::vector<unsigned int>(hist.size(), 0); 
}

void DimHistogram::newData( const unsigned int &bpp, void *data, const unsigned int &num_data_points, const D_DataFormat &fmt, unsigned char *mask ) {
  init( bpp, fmt );
  clear( );
  getStats( data, num_data_points, mask );
  addData( data, num_data_points, mask );
}

template <typename T> 
void DimHistogram::init_stats() {
  value_min = std::numeric_limits<T>::min();
  value_max = std::numeric_limits<T>::max();
  reversed_min_max = false;
  recompute_shift_scale();
}

void DimHistogram::initStats() {
  if (data_bpp==8  && data_fmt==D_FMT_UNSIGNED) init_stats<DIM_UINT8>();
  else
  if (data_bpp==16 && data_fmt==D_FMT_UNSIGNED) init_stats<DIM_UINT16>();
  else   
  if (data_bpp==32 && data_fmt==D_FMT_UNSIGNED) init_stats<DIM_UINT32>();
  else
  if (data_bpp==8  && data_fmt==D_FMT_SIGNED)   init_stats<DIM_INT8>();
  else
  if (data_bpp==16 && data_fmt==D_FMT_SIGNED)   init_stats<DIM_INT16>();
  else
  if (data_bpp==32 && data_fmt==D_FMT_SIGNED)   init_stats<DIM_INT32>();
  else
  if (data_bpp==32 && data_fmt==D_FMT_FLOAT)    init_stats<DIM_FLOAT32>();
  else
  if (data_bpp==64 && data_fmt==D_FMT_FLOAT)    init_stats<DIM_FLOAT64>();
  else
  if (data_bpp==80 && data_fmt==D_FMT_FLOAT)    init_stats<DIM_FLOAT80>();
}

inline void DimHistogram::recompute_shift_scale() {
  shift = value_min;
  scale = ((double) bin_number()-1) / (value_max-value_min);
}

template <typename T>
void DimHistogram::update_data_stats( T *data, const unsigned int &num_data_points, unsigned char *mask ) {
  if (!data) return;
  if (num_data_points==0) return;
  if (!reversed_min_max) {
    value_min = std::numeric_limits<T>::max();
    value_max = std::numeric_limits<T>::min();
    reversed_min_max = true;
  }

  T *p = (T *) data;  
  if (mask == 0) {
    for (unsigned int i=0; i<num_data_points; ++i) {
      if (*p<value_min) value_min = *p;
      if (*p>value_max) value_max = *p;
      ++p;
    }
  } else {
    for (unsigned int i=0; i<num_data_points; ++i) {
      if (mask[i]>128) {
        if (*p<value_min) value_min = *p;
        if (*p>value_max) value_max = *p;
      }
      ++p;
    }
  }
  
  recompute_shift_scale();
}

template <typename T>
void DimHistogram::get_data_stats( T *data, const unsigned int &num_data_points, unsigned char *mask ) {
  init_stats<T>();
  value_min = std::numeric_limits<T>::max();
  value_max = std::numeric_limits<T>::min();
  reversed_min_max = true;
  update_data_stats(data, num_data_points, mask);
}

void DimHistogram::getStats( void *data, const unsigned int &num_data_points, unsigned char *mask ) {
  if (data_fmt==D_FMT_UNSIGNED) {
    if (data_bpp == 32) get_data_stats<DIM_UINT32>( (DIM_UINT32*)data, num_data_points, mask );
  } else
  if (data_fmt==D_FMT_SIGNED) {
    if (data_bpp == 32) get_data_stats<DIM_INT32>( (DIM_INT32*)data, num_data_points, mask );
  } else
  if (data_fmt==D_FMT_FLOAT) {
    if (data_bpp == 32) get_data_stats<DIM_FLOAT32>( (DIM_FLOAT32*)data, num_data_points, mask );
    else
    if (data_bpp == 64) get_data_stats<DIM_FLOAT64>( (DIM_FLOAT64*)data, num_data_points, mask );
    else
    if (data_bpp == 80) get_data_stats<DIM_FLOAT80>( (DIM_FLOAT80*)data, num_data_points, mask );
  }
}

template <typename T>
inline unsigned int DimHistogram::bin_from( const T &data_point ) const {
  return dim::trim<unsigned int, double>( (data_point - shift) * scale, 0, bin_number() );
}

template <typename T>
void DimHistogram::add_from_data( T *data, const unsigned int &num_data_points, unsigned char *mask ) {
  if (!data) return;
  if (num_data_points==0) return;

  T *p = (T *) data;  
  if (mask==0) {
    for (unsigned int i=0; i<num_data_points; ++i) {
      ++hist[*p];
      ++p;
    }
  } else {
    for (unsigned int i=0; i<num_data_points; ++i) {
      if (mask[i]>128) ++hist[*p];
      ++p;
    }
  }
}

template <typename T>
void DimHistogram::add_from_data_scale( T *data, const unsigned int &num_data_points, unsigned char *mask ) {
  if (!data) return;
  if (num_data_points==0) return;
  //get_data_stats( data, num_data_points );

  T *p = (T *) data; 
  unsigned int bn = bin_number();
  if (mask==0) {
    for (unsigned int i=0; i<num_data_points; ++i) {
      double v = (((double)*p) - shift) * scale;
      unsigned int bin = dim::trim<unsigned int, double>( v, 0, bn-1 );
      ++hist[bin];
      ++p;
    }
  } else {
    for (unsigned int i=0; i<num_data_points; ++i) {
      if (mask[i]>128) {
        double v = (((double)*p) - shift) * scale;
        unsigned int bin = dim::trim<unsigned int, double>( v, 0, bn-1 );
        ++hist[bin];
      }
      ++p;
    }
  }
}

void DimHistogram::updateStats( void *data, const unsigned int &num_data_points, unsigned char *mask ) {
  if (data_fmt==D_FMT_UNSIGNED) {
    if (data_bpp == 32) update_data_stats<DIM_UINT32>( (DIM_UINT32*)data, num_data_points, mask );
  } else
  if (data_fmt==D_FMT_SIGNED) {
    if (data_bpp == 32) update_data_stats<DIM_INT32>( (DIM_INT32*)data, num_data_points, mask );
  } else
  if (data_fmt==D_FMT_FLOAT) {
    if (data_bpp == 32) update_data_stats<DIM_FLOAT32>( (DIM_FLOAT32*)data, num_data_points, mask );
    else
    if (data_bpp == 64) update_data_stats<DIM_FLOAT64>( (DIM_FLOAT64*)data, num_data_points, mask );
    else
    if (data_bpp == 80) update_data_stats<DIM_FLOAT80>( (DIM_FLOAT80*)data, num_data_points, mask );
  }
}

void DimHistogram::addData( void *data, const unsigned int &num_data_points, unsigned char *mask ) {

  if (data_fmt==D_FMT_UNSIGNED) {
    if (data_bpp==8)    add_from_data<DIM_UINT8>( (DIM_UINT8*)data, num_data_points, mask );
    else
    if (data_bpp == 16) add_from_data<DIM_UINT16>( (DIM_UINT16*)data, num_data_points, mask );
    else
    if (data_bpp == 32) add_from_data_scale<DIM_UINT32>( (DIM_UINT32*)data, num_data_points, mask );
  } else
  if (data_fmt==D_FMT_SIGNED) {
    if (data_bpp==8)    add_from_data_scale<DIM_INT8>( (DIM_INT8*)data, num_data_points, mask );
    else
    if (data_bpp == 16) add_from_data_scale<DIM_INT16>( (DIM_INT16*)data, num_data_points, mask );
    else
    if (data_bpp == 32) add_from_data_scale<DIM_INT32>( (DIM_INT32*)data, num_data_points, mask );
  } else
  if (data_fmt==D_FMT_FLOAT) {
    if (data_bpp == 32) add_from_data_scale<DIM_FLOAT32>( (DIM_FLOAT32*)data, num_data_points, mask );
    else
    if (data_bpp == 64) add_from_data_scale<DIM_FLOAT64>( (DIM_FLOAT64*)data, num_data_points, mask );
    else
    if (data_bpp == 80) add_from_data_scale<DIM_FLOAT80>( (DIM_FLOAT80*)data, num_data_points, mask );
  }
}



int DimHistogram::bin_of_last_nonzero() const {
  for (int i=hist.size()-1; i>=0; --i)
    if (hist[i] != 0)
      return i;
  return 0;
}

int DimHistogram::bin_of_first_nonzero() const {
  for (unsigned int i=0; i<hist.size(); ++i)
    if (hist[i] != 0)
      return i;
  return 0;
}

int DimHistogram::bin_of_max_value() const {
  int bin = 0;
  unsigned int val = hist[0];
  for (unsigned int i=0; i<hist.size(); ++i)
    if (hist[i] > val) {
      val = hist[i];
      bin = i;
    }
  return bin;
}

int DimHistogram::bin_of_min_value() const {
  int bin = 0;
  unsigned int val = hist[0];
  for (unsigned int i=0; i<hist.size(); ++i)
    if (hist[i] < val) {
      val = hist[i];
      bin = i;
    }
  return bin;
}

double DimHistogram::max_value() const {
  if (data_fmt==D_FMT_UNSIGNED && data_bpp<=16)
    return bin_of_last_nonzero();
  else
    return value_max;
}

double DimHistogram::min_value() const {
  if (data_fmt==D_FMT_UNSIGNED && data_bpp<=16)
    return bin_of_first_nonzero();
  else
    return value_min;
}

int DimHistogram::bin_number_nonzero() const {
  int unique = 0;
  for (unsigned int i=0; i<hist.size(); ++i) 
    if (hist[i] != 0) ++unique;
  return unique;
}

double DimHistogram::average() const {
  double a=0.0, s=0.0;
  for (unsigned int i=0; i<hist.size(); ++i)
    if (hist[i]>0) {
      a += i * hist[i];
      s += hist[i];
    }
  double mu = a/s;
  return (mu/scale) + shift;
}

double DimHistogram::std() const {
  double mu = average();
  double a=0.0, s=0.0;
  for (unsigned int i=0; i<hist.size(); ++i)
    if (hist[i]>0) {
      s += hist[i];
      a += (((double)i)-mu)*(((double)i)-mu) * hist[i];
    }
  double sig_sq = sqrt( a/(s-1) ); 
  return (sig_sq/scale) + shift;
}

unsigned int DimHistogram::cumsum( const unsigned int &bin ) const {
  unsigned int b = bin;
  if ( b >= hist.size() ) b = hist.size()-1;
  unsigned int sum=0;
  for (unsigned int i=0; i<=b; ++i) 
    sum += hist[i];
  return sum;
}

unsigned int DimHistogram::get_value( const unsigned int &bin ) const {
  if ( bin < hist.size() )
    return hist[bin];
  else
    return 0;
}

void DimHistogram::set_value( const unsigned int &bin, const unsigned int &val ) {
  if ( bin < hist.size() )
    hist[bin] = val;
}

void DimHistogram::append_value( const unsigned int &bin, const unsigned int &val ) {
  if ( bin < hist.size() )
    hist[bin] += val;
}

//******************************************************************************
// DimLut
//******************************************************************************

DimLut::DimLut( ) {
  lut.resize( 0 ); 
  this->generator = linear_full_range_generator;
}

DimLut::DimLut( const DimHistogram &in, const DimHistogram &out ) {
  init( in, out );
}

DimLut::DimLut( const DimHistogram &in, const DimHistogram &out, const LutType &type ) {
  init( in, out, type );
}

DimLut::DimLut( const DimHistogram &in, const DimHistogram &out, DimLutGenerator custom_generator ) {
  init( in, out, custom_generator );
}

DimLut::~DimLut() {

}

void DimLut::init( const DimHistogram &in, const DimHistogram &out ) {
  lut.resize( in.size() );
  clear( );
  h_in = in;
  h_out = out;
  if (generator) generator( in, lut, out.size(), NULL );
}

void DimLut::init( const DimHistogram &in, const DimHistogram &out, const LutType &type ) {
  if (type == ltLinearFullRange)     generator = linear_full_range_generator;
  if (type == ltLinearDataRange)     generator = linear_data_range_generator;
  if (type == ltLinearDataTolerance) generator = linear_data_tolerance_generator;
  if (type == ltEqualize)            generator = equalized_generator;
  init( in, out );
}

void DimLut::init( const DimHistogram &in, const DimHistogram &out, DimLutGenerator custom_generator ) {
  generator = custom_generator;
  init( in, out );
}

void DimLut::clear( ) {
  lut = std::vector<DimLut::StorageType>(lut.size(), 0); 
}

template <typename Tl>
void DimLut::set_lut( const std::vector<Tl> &new_lut ) {
  lut.assign( new_lut.begin(), new_lut.end() );
}
template void DimLut::set_lut<unsigned char>( const std::vector<unsigned char> &new_lut );
template void DimLut::set_lut<unsigned int>( const std::vector<unsigned int> &new_lut );
template void DimLut::set_lut<double>( const std::vector<double> &new_lut );

DimLut::StorageType DimLut::get_value( const unsigned int &pos ) const {
  if ( pos < lut.size() )
    return lut[pos];
  else
    return 0;
}

void DimLut::set_value( const unsigned int &pos, const DimLut::StorageType &val ) {
  if ( pos < lut.size() )
    lut[pos] = val;
}

template <typename Ti, typename To>
void DimLut::apply_lut( const Ti *ibuf, To *obuf, const unsigned int &num_data_points ) const {
  if (!ibuf || !obuf) return;

  for (unsigned int i=0; i<num_data_points; ++i)
    obuf[i] = (To) lut[ ibuf[i] ];
}

template <typename Ti, typename To>
void DimLut::apply_lut_scale_from( const Ti *ibuf, To *obuf, const unsigned int &num_data_points ) const {
  if (!ibuf || !obuf) return;
  double scale = h_in.scale;
  double shift = h_in.shift;
  double range = lut.size();

  for (unsigned int i=0; i<num_data_points; ++i)
    obuf[i] = (To) lut[ dim::trim<unsigned int, double>( ((double)ibuf[i]-shift)*scale, 0, range-1) ];
}

//------------------------------------------------------------------------------------
// ok, follows crazy code bloat of instantiations, will change this eventually
//------------------------------------------------------------------------------------

// this guy instantiates real method based on input template
template <typename Ti>
inline void DimLut::do_apply_lut( const Ti *ibuf, const void *obuf, const unsigned int &num_data_points ) const {
  
  if (h_out.dataBpp()==8 && h_out.dataFormat()!=D_FMT_FLOAT )
    apply_lut<Ti, DIM_UINT8>( ibuf, (DIM_UINT8*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==16 && h_out.dataFormat()!=D_FMT_FLOAT )
    apply_lut<Ti, DIM_UINT16>( ibuf, (DIM_UINT16*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==32 && h_out.dataFormat()!=D_FMT_FLOAT )
    apply_lut<Ti, DIM_UINT32>( ibuf, (DIM_UINT32*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==32 && h_out.dataFormat()==D_FMT_FLOAT )
    apply_lut<Ti, DIM_FLOAT32>( ibuf, (DIM_FLOAT32*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==64 && h_out.dataFormat()==D_FMT_FLOAT )
    apply_lut<Ti, DIM_FLOAT64>( ibuf, (DIM_FLOAT64*) obuf, num_data_points );
}

// this guy instantiates real method based on input template
template <typename Ti>
inline void DimLut::do_apply_lut_scale_from( const Ti *ibuf, const void *obuf, const unsigned int &num_data_points ) const {
  if (h_out.dataBpp()==8 && h_out.dataFormat()!=D_FMT_FLOAT )
    apply_lut_scale_from<Ti, DIM_UINT8>( ibuf, (DIM_UINT8*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==16 && h_out.dataFormat()!=D_FMT_FLOAT )
    apply_lut_scale_from<Ti, DIM_UINT16>( ibuf, (DIM_UINT16*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==32 && h_out.dataFormat()!=D_FMT_FLOAT )
    apply_lut_scale_from<Ti, DIM_UINT32>( ibuf, (DIM_UINT32*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==32 && h_out.dataFormat()==D_FMT_FLOAT )
    apply_lut_scale_from<Ti, DIM_FLOAT32>( ibuf, (DIM_FLOAT32*) obuf, num_data_points );
  else
  if (h_out.dataBpp()==64 && h_out.dataFormat()==D_FMT_FLOAT )
    apply_lut_scale_from<Ti, DIM_FLOAT64>( ibuf, (DIM_FLOAT64*) obuf, num_data_points );
}

void DimLut::apply( void *ibuf, const void *obuf, const unsigned int &num_data_points ) const {
  if (lut.size() <= 0) return;

  // uint
  if (h_in.dataBpp()==8 && h_in.dataFormat()==D_FMT_UNSIGNED)
    do_apply_lut<DIM_UINT8>( (DIM_UINT8*) ibuf, obuf, num_data_points );
  else
  if (h_in.dataBpp()==16 && h_in.dataFormat()==D_FMT_UNSIGNED)
    do_apply_lut<DIM_UINT16>( (DIM_UINT16*) ibuf, obuf, num_data_points );
  else
  if (h_in.dataBpp()==32 && h_in.dataFormat()==D_FMT_UNSIGNED)
    do_apply_lut_scale_from<DIM_UINT32>( (DIM_UINT32*) ibuf, obuf, num_data_points );
  else
  // int
  if (h_in.dataBpp()==8 && h_in.dataFormat()==D_FMT_SIGNED)
    do_apply_lut_scale_from<DIM_INT8>( (DIM_INT8*) ibuf, obuf, num_data_points );
  else
  if (h_in.dataBpp()==16 && h_in.dataFormat()==D_FMT_SIGNED)
    do_apply_lut_scale_from<DIM_INT16>( (DIM_INT16*) ibuf, obuf, num_data_points );
  else
  if (h_in.dataBpp()==32 && h_in.dataFormat()==D_FMT_SIGNED)
    do_apply_lut_scale_from<DIM_INT32>( (DIM_INT32*) ibuf, obuf, num_data_points );
  else
  // float: current implementation would provide poor quality for float2float because of the LUT binning size
  // should look into doing this by applying generator function for each element of the data ignoring LUT at all...
  if (h_in.dataBpp()==32 && h_in.dataFormat()==D_FMT_FLOAT)
    do_apply_lut_scale_from<DIM_FLOAT32>( (DIM_FLOAT32*) ibuf, obuf, num_data_points );
  else
  if (h_in.dataBpp()==64 && h_in.dataFormat()==D_FMT_FLOAT)
    do_apply_lut_scale_from<DIM_FLOAT64>( (DIM_FLOAT64*) ibuf, obuf, num_data_points );
}

void DimLut::apply( const DimHistogram &in, DimHistogram &out ) const {
  if (lut.size() <= 0) return;
  out.clear();
  for (unsigned int i=0; i<in.size(); ++i)
    out.append_value( lut[i], in[i] );
}

//******************************************************************************
// Generators
//******************************************************************************

//------------------------
// this is not a generator per si
//------------------------
template <typename Tl>
void linear_range_generator( int b, int e, unsigned int out_phys_range, std::vector<Tl> &lut ) {
  
  //if (lut.size() < out_phys_range) lut.resize(out_phys_range);
  // simple linear mapping for actual range
  double range = e - b;
  if (range < 1) range = out_phys_range;
  for (unsigned int x=0; x<lut.size(); ++x)
    lut[x] = dim::trim<Tl, double> ( (((double)x)-b)*out_phys_range/range, 0, out_phys_range-1);
}
template void linear_range_generator<unsigned char>( int b, int e, unsigned int out_phys_range, std::vector<unsigned char> &lut );
template void linear_range_generator<unsigned int>( int b, int e, unsigned int out_phys_range, std::vector<unsigned int> &lut );
template void linear_range_generator<double>( int b, int e, unsigned int out_phys_range, std::vector<double> &lut );
//------------------------


template <typename Tl>
void linear_full_range_generator( const DimHistogram &, std::vector<Tl> &lut, unsigned int out_phys_range, void * ) {
  // simple linear mapping for full range
  for (unsigned int x=0; x<lut.size(); ++x)
    lut[x] = (Tl) ( (((double)x)*(out_phys_range-1.0))/(lut.size()-1.0) );
}
template void linear_full_range_generator<unsigned char>( const DimHistogram &in, std::vector<unsigned char> &lut, unsigned int out_phys_range, void *args );
template void linear_full_range_generator<unsigned int>( const DimHistogram &in, std::vector<unsigned int> &lut, unsigned int out_phys_range, void *args );
template void linear_full_range_generator<double>( const DimHistogram &in, std::vector<double> &lut, unsigned int out_phys_range, void *args );

template <typename Tl>
void linear_data_range_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void * ) {
  // simple linear mapping for actual range
  int b = in.first_pos();
  int e = in.last_pos();
  double range = e - b;
  if (range < 1) range = out_phys_range;
  for (unsigned int x=0; x<lut.size(); ++x)
    lut[x] = dim::trim<Tl, double> ( (((double)x)-b)*out_phys_range/range, 0, out_phys_range-1);
}
template void linear_data_range_generator<unsigned char>( const DimHistogram &in, std::vector<unsigned char> &lut, unsigned int out_phys_range, void *args );
template void linear_data_range_generator<unsigned int>( const DimHistogram &in, std::vector<unsigned int> &lut, unsigned int out_phys_range, void *args );
template void linear_data_range_generator<double>( const DimHistogram &in, std::vector<double> &lut, unsigned int out_phys_range, void *args );

template <typename Tl>
void linear_data_tolerance_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void * ) {
  
  // simple linear mapping cutting elements with small appearence
  // get 1% threshold
  unsigned int th = (unsigned int) ( in[ in.bin_of_max_value() ] * 1.0 / 100.0 );
  int b = 0;
  int e = in.size()-1;
  for (unsigned int x=0; x<in.size(); ++x)
    if ( in[x] > th ) {
      b = x;
      break;
    }
  for (int x=in.size()-1; x>=0; --x)
    if ( in[x] > th ) {
      e = x;
      break;
    }

  double range = e - b;
  if (range < 1) range = out_phys_range;
  for (unsigned int x=0; x<lut.size(); ++x)
    lut[x] = dim::trim<Tl, double> ( (((double)x)-b)*out_phys_range/range, 0, out_phys_range-1 );
}
template void linear_data_tolerance_generator<unsigned char>( const DimHistogram &in, std::vector<unsigned char> &lut, unsigned int out_phys_range, void *args );
template void linear_data_tolerance_generator<unsigned int>( const DimHistogram &in, std::vector<unsigned int> &lut, unsigned int out_phys_range, void *args );
template void linear_data_tolerance_generator<double>( const DimHistogram &in, std::vector<double> &lut, unsigned int out_phys_range, void *args );

template <typename Tl>
void equalized_generator( const DimHistogram &in, std::vector<Tl> &lut, unsigned int out_phys_range, void * ) {
  
  // equalize
  std::vector<double> map(lut.size(), 0);
  map[0] = in[0];
  for (unsigned int x=1; x<lut.size(); ++x)
    map[x] = map[x-1] + in[x];

  double div = map[lut.size()-1]-map[0];
  if (div > 0)
  for (unsigned int x=0; x<lut.size(); ++x)
    lut[x] = dim::trim<Tl, double>( (Tl) (out_phys_range-1.0)*((map[x]-map[0]) / div), 0, out_phys_range-1 );
}
template void equalized_generator<unsigned char>( const DimHistogram &in, std::vector<unsigned char> &lut, unsigned int out_phys_range, void *args );
template void equalized_generator<unsigned int>( const DimHistogram &in, std::vector<unsigned int> &lut, unsigned int out_phys_range, void *args );
template void equalized_generator<double>( const DimHistogram &in, std::vector<double> &lut, unsigned int out_phys_range, void *args );

template <typename Tl>
void linear_custom_range_generator( const DimHistogram &/*in*/, std::vector<Tl> &lut, unsigned int out_phys_range, void *args ) {
  // simple linear mapping for actual range
  int *vals = (int *)args;
  int b = vals[0];
  int e = vals[1];
  double range = e - b;
  if (range < 1) range = out_phys_range;
  for (unsigned int x=0; x<lut.size(); ++x)
    lut[x] = dim::trim<Tl, double> ( (((double)x)-b)*out_phys_range/range, 0, out_phys_range-1);
}
template void linear_custom_range_generator<unsigned char>( const DimHistogram &in, std::vector<unsigned char> &lut, unsigned int out_phys_range, void *args );
template void linear_custom_range_generator<unsigned int>( const DimHistogram &in, std::vector<unsigned int> &lut, unsigned int out_phys_range, void *args );
template void linear_custom_range_generator<double>( const DimHistogram &in, std::vector<double> &lut, unsigned int out_phys_range, void *args );
